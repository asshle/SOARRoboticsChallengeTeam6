#include "MeMegaPi.h"
#include <SoftwareSerial.h>

enum Direction {Left,Right,Backward};
enum Memory{LeftTurn, RightTurn , BackTurn , Straight ,Start ,TStop };
enum SensorTransitionState
{
  RotateBackwards,
  Continue,
  ImmediateRightCorrectiveOrder,
  ImmediateLeftCorrectiveOrder,
  WIN,
  MinorRightCorrection,
  RightRotation,
  MinorLeftCorrection,
  LeftRotation,
  LeftRightCondition,
};

struct timeScheduling
{
  const unsigned long readSensorTimeSlice=50; // change this variable for more responsive multitasking 
  const unsigned long MotorControlTimeSlice=50;
  unsigned long readSensorLastTime=0;
  unsigned long MotorControlLastTime=0;
  bool threadHalt = true; 
  
};
struct sensorsValue
{
  unsigned int leftSensorState;
  unsigned int rightSensorState;
  unsigned int combinedState;
  bool TLock = false;
  String _reccoAct = "contiue";
  SensorTransitionState reccomendedAction = Continue;
  bool robotRotationState= false;
  
};
struct motorParam
{
  bool MotorLock = false;
  
};
struct rotationMemory 
{
  Memory stateMemory[100]; // allocating for turning 
  String LastDir= "" ; 
  bool backRotationLock =false;
  int count=0;
  int TStopCount = 0 ; // 15
  int lRotationCounter = 0;
  int rRotationCounter = 0;
  int rotationCounter = 0;
  int errorMovementCounter = 0 ;
  int LRrotationCounterValue= 400; // sampled Value for one rotation 
  int BrotationCounterValue= 487; // sampled Value for one rotation 
  int forwardStepCount =20; // use for moving a small amount
};

void initSerialComms();
void Movement(String dir, int speedValue );
void Interpreter(String incoming);
void Handshake();
void readSensor();
void RotateTurns();
void MotorControl(); 
void UpdateMemoryState(Memory state,String _lastDir);

String lastBTMessage;
String directionPriority="LEFT";

MeMegaPiDCMotor RightMotorA(PORT1A); 
MeMegaPiDCMotor RightMotorB(PORT1B); 
MeMegaPiDCMotor LeftMotorA(PORT2A);
MeMegaPiDCMotor LeftMotorB(PORT2B);

MeLineFollower leftSensor(PORT_5); //Left IR sensor
MeLineFollower rightSensor(PORT_6); //Right IR sensor

timeScheduling multiTasking;
sensorsValue sensorValue;
motorParam motorParam;
rotationMemory rotateMem; 



void setup()
{
  initSerialComms();  
  UpdateMemoryState(Start,"Start");
}

void loop()
{
  // To save overhead power , please try to use minimal code here
  // for clean architecture , use interrupt driven program structures
  
  unsigned long currentTime = millis();
  if(!multiTasking.threadHalt)
  {
    //Individual threads goes here
    if( (unsigned long)(currentTime-multiTasking.readSensorLastTime)>=multiTasking.readSensorTimeSlice)
    {
     //readingSensor thread
     readSensor();
    multiTasking.readSensorLastTime = currentTime;
    }
    if( (unsigned long)(currentTime-multiTasking.MotorControlLastTime) >=multiTasking.MotorControlTimeSlice)
    {
     //movement and sequencing here
     MotorControl();
     multiTasking.MotorControlLastTime = currentTime
    }
  }
  else
  {
    if(motorParam.MotorLock)
    {
      Movement("S",0);
    }
    
  }
  
}

//---------------INIT FUNCTIONS -------------------
void initSerialComms()
{
  Serial.begin(115200);
  Serial3.begin(115200);

  Serial.println("Initialised");
}

//---------------EVENT FUNCTIONS -----------------
void serialEvent() // Event when incoming serial comms is triggered
{
  String Incoming;
  Serial.print("------INTERRUPTED-------------\n");
  if(Serial.available()) 
  {
    Incoming = Serial.readString();
  }
  Serial.println(Incoming);
  Interpreter(Incoming);
  
  Serial.print("------INTERRUPT ENDED-------------\n");
}
void serialEvent3()
{

  /**!   RULES FOR COMMUNICATIONS WITH SERIAL3 
      -> KEEP MESSAGE SHORT , (I.E MESSAGE < 1*SERIAL3.WRITE)
      -> INSERT THE KEYWORD "MEGAPI:" INFRONT OF WHATEVER MESSAGE YOU ARE SENDING 
  !**/
  String Incoming;
  Serial.print("------BT INTERRUPTED-------------\n");
  //Serial3.print("------BT INTERRUPTED-------------\n");
  if(Serial3.available())
  {
    Incoming = Serial3.readString();
    Incoming.trim();
    Serial.println(Incoming);
    //Serial3.println(Incoming);
    if(Incoming != lastBTMessage || Incoming == "S:")
    {
      Interpreter(Incoming);
      lastBTMessage = Incoming;
    } 
  }
  Serial.print("------BT INTERRUPT ENDED-------------\n");
  //Serial3.print("------BT INTERRUPTED ENDED-------------\n");
}
//---------------HELPER FUNCTIONS ---------------------
void Interpreter(String incoming)
{
  String Function,paramHolding,directionCommand,speedValue;
  int _speedValue;
  
  Function = incoming.substring(0,incoming.indexOf(':')); // needed for checking for valid command
  paramHolding = incoming.substring(incoming.indexOf(':')+1,incoming.length());
  
  char FunctionArray[Function.length()+1];
  strcpy(FunctionArray,Function.c_str());
  switch(*FunctionArray)
  {
    case *"Movement":
    
      directionCommand= paramHolding.substring(0 ,paramHolding.indexOf(','));
      speedValue = paramHolding.substring(paramHolding.indexOf(',')+1 , paramHolding.length());
      _speedValue = speedValue.toInt();
 
      Movement(directionCommand,_speedValue);
    break ; 
    case *"Handshake":
      Handshake();
    break;
    case *"S":
     //Stops the multithreading loop
     LoopControl();
    break;
    //TODO:
    //CREATE READSENSOR FUNCTION
    //CREATE UPDATE PRIORITY FUNCTION
    default : 
       Serial.println("Invalid Function ");
    break; 
  }
}
//--------------Memory Access Control --------------------------
void UpdateMemoryState(Memory state,String _lastDir)
{ 
  if(rotateMem.LastDir != _lastDir)
  {
    rotateMem.LastDir = _lastDir;
    rotateMem.stateMemory[rotateMem.count] = state;
    rotateMem.count++; 
    //Serial3.print(sensorValue._reccoAct);Serial3.print(" ");Serial3.print(rotateMem.LastDir);Serial3.print(" ");Serial3.println(rotateMem.count); 
  }
  if(rotateMem.count >99 ) // this is to prevent MemoryOutOfRange 
  {
    //Flush out memory here
    Memory _tempMem =rotateMem.stateMemory[100];
    memset(rotateMem.stateMemory, 0, sizeof(rotateMem.stateMemory)); // clearing memory 
    rotateMem.stateMemory[0]=_tempMem; // saving the previous value of the memory
    rotateMem.count = 0 ; // reseting the counter value
  }
  return; 
}

void reset()
{
  rotateMem.TStopCount=0;
  rotateMem.rotationCounter = 0;
  rotateMem.errorMovementCounter = 0 ;
  rotateMem.lRotationCounter = 0;
  rotateMem.rRotationCounter = 0;
  sensorValue.TLock = false;
};
//--------------Communications Functions -----------------------
void Handshake()
{
  Serial.println("MEGAPI:Recived");
  Serial3.write("MEGAPI:Recieved ");
}
void LoopControl()
{
  if(multiTasking.threadHalt)
  {
    multiTasking.threadHalt = false;
    motorParam.MotorLock = false;
  }
  else
  {
    motorParam.MotorLock = true;
    multiTasking.threadHalt = true;
    
  }
}
void outputMotorCondition()
{
  Serial3.print(sensorValue._reccoAct);Serial3.print(" ");Serial3.print(sensorValue.leftSensorState);Serial3.print(" ");Serial3.print(sensorValue.rightSensorState);
  //Serial3.print(" MemoryCounter : ");Serial3.print(rotateMem.count);
  Serial3.print(" TLock  ");Serial3.print(sensorValue.TLock );
  Serial3.print(" lRotationCounter  ");Serial3.print(rotateMem.lRotationCounter );
  Serial3.print(" rRotationCounter  ");Serial3.println(rotateMem.rRotationCounter );
  //Serial3.print(" TStopCount : ");Serial3.println(rotateMem.TStopCount);
  //Serial3.print(" RotationCounter : ");Serial3.print(rotateMem.rotationCounter);
 // Serial3.print(" errorMovementCounter : ");Serial3.println(rotateMem.errorMovementCounter); 
  //Serial.print(sensorValue._reccoAct);Serial.print(" ");Serial.print(sensorValue.leftSensorState);Serial.print(" ");Serial.println(sensorValue.rightSensorState);
  //Serial3.print("MemoryState : ");Serial3.println(rotateMem.LastDir); Serial3.print(" MemoryAddress : ");Serial3.println(rotateMem.count);
  //Serial.print("MemoryState");Serial.print(": ");Serial.println(rotateMem.LastDir);Serial.print(" MemoryAddress : ");Serial.println(rotateMem.count);
}
//-------------ROBOTIC FUNCTIONS ------------------------------
void MotorControl()
{   
  //TODO:
  SensorTransitionState currentReccoState = sensorValue.reccomendedAction;
  outputMotorCondition();
  // set left or right priority 
  // set routing optimisation here

  int forwardSpeed =100, rotationSpeed=75 , correctionSpeed=100 ; // experiment with the values 
  // defualt  FS =100 , RS = 75 ,CS = 100
  switch(currentReccoState) 
  {
    // TODO: enable speed change through the use of global variable 
    case RotateBackwards:
      //When robot is totally off the line
      sensorValue.robotRotationState = true;
      RotateTurns(Backward,rotationSpeed);
    break;
    case ImmediateRightCorrectiveOrder:
      //When robot is about to leave the line to the right
    break;
    case ImmediateLeftCorrectiveOrder:
       //When robot is about to leave the line to the left
    break;
    case MinorRightCorrection:
       //When robot is of the line by abit to the right
       Movement("D",correctionSpeed);
    break;
    case MinorLeftCorrection:
      //When robot is of the line by abit to the left
      Movement("A",correctionSpeed);
    break;
    case RightRotation:
      //When right turn is possible
      sensorValue.robotRotationState = true;
      RotateTurns(Right,rotationSpeed);
    break;
    case LeftRotation:
      //When left turn is possible
      sensorValue.robotRotationState = true ;
      RotateTurns(Left,rotationSpeed);
    break;
    case Continue:
      Movement("F",forwardSpeed); // 
      UpdateMemoryState(Straight,"Straight");
      reset();
      
    break;
    case LeftRightCondition:
     //UpdateMemoryState(TStop,"TStop");
     //Serial3.print("Tjunction Detected");Serial3.print(" ");Serial3.print(sensorValue.leftSensorState);Serial3.print(" ");Serial3.println(sensorValue.rightSensorState);
     sensorValue.TLock= true; 
     if(directionPriority == "LEFT")
     {
      //Movement("S",0);
      RotateTurns(Left,75);
     }
     else
     {
      //Movement("S",0);
      RotateTurns(Right,75);
     }
    break;
    
  }
}

void RotateTurns(Direction rotationDir,int rotationSpeed)
{
  // calibrate the speed value 
  // Observation is that right motor is weaker that left motor
  switch (rotationDir)
  {
    case Right:
      if(sensorValue.reccomendedAction != Continue)
      {
        //Serial3.print("RightTurn");Serial3.print(" ");Serial3.print(sensorValue.leftSensorState);Serial3.print(" ");Serial3.println(sensorValue.rightSensorState);
        if(rotateMem.rotationCounter <rotateMem.LRrotationCounterValue) // this is overcome sharp turns or places where it is out of physical reach of sensors
        {
          // if no anomaly is detected
          Movement("R",rotationSpeed);
          rotateMem.rotationCounter++;
          sensorValue.robotRotationState = true;
          UpdateMemoryState(RightTurn,"RightTurn");
          
          rotateMem.rRotationCounter++;
          if(rotateMem.rRotationCounter > 20)
          {
            sensorValue.TLock= true;
          }
        }
        else
        {
           // recovery action
          Movement("S",0);
          rotateMem.rotationCounter = 0;
          //Serial3.print("Recovery Action : ");
          sensorValue.robotRotationState = false;
        }
      }
      else
      {
        Movement("S",0);
        sensorValue.robotRotationState = false;
        
      }
    break;
    case Left:
     if(sensorValue.reccomendedAction != Continue)
      {
        //Serial3.print("LeftTurn");Serial3.print(" ");Serial3.print(sensorValue.leftSensorState);Serial3.print(" ");Serial3.println(sensorValue.rightSensorState);
        if(rotateMem.rotationCounter <rotateMem.LRrotationCounterValue) // value here is sampled value please tweak this value
        {
          // value sampled abouve is for one rev 
          Movement("L",rotationSpeed);
          rotateMem.rotationCounter++;
          sensorValue.robotRotationState = true;
          UpdateMemoryState(LeftTurn,"LeftTurn");

          rotateMem.lRotationCounter++;
          if(rotateMem.lRotationCounter > 20)
          {
            sensorValue.TLock= true;
          }
        }
        else
        {
          Movement("S",0);
          rotateMem.rotationCounter = 0;
          sensorValue.robotRotationState = false;
        }
      }
      else
      {
        //Serial3.println("Rotating : Stop");
        Movement("S",0);
        sensorValue.robotRotationState = false;
      }
    break;
    case Backward:
     if(sensorValue.reccomendedAction != Continue )
      {
        if(rotateMem.rotationCounter < rotateMem.BrotationCounterValue ) // value here sampled value please tweak this value
        {
          // value sampled abouve is
          // allocating for turning  for one rev 

          Movement("l",rotationSpeed);
          UpdateMemoryState(BackTurn,"LeftBackTurn"); 
          rotateMem.rotationCounter++;
        }
        else
        {
          Movement("S",0);
          if(rotateMem.errorMovementCounter <rotateMem.forwardStepCount  )
          {
            Movement("F",100);
            rotateMem.errorMovementCounter++;
          }
          else 
          {
            Movement("S",0);
            rotateMem.rotationCounter=0;
            rotateMem.errorMovementCounter =0;
            sensorValue.robotRotationState = false;
          }
        }
      }
      else
      {
        //Serial3.println("Rotating : Stop");
        Movement("S",0);
        sensorValue.robotRotationState = false;
        UpdateMemoryState(Straight,"Straight");
      }
    break;
  }
}

void runTimeCatch()
{
   if(sensorValue.leftSensorState == 0 && sensorValue.rightSensorState == 0 )
    {
      //filtering right turn
      rotateMem.TStopCount++;
      if( rotateMem.TStopCount >15)
      {
        sensorValue._reccoAct = "LeftRightCondition";
        sensorValue.reccomendedAction =LeftRightCondition;
      }
    }
}
void Movement(String dir, int speedValue )
{
  String heading;
  String errorFlag="";
  int speedDifference= 0 ; 
  if(speedValue<-255 || speedValue > 255 )
  {
    errorFlag = String("Movement : INVALID SPEEDVALUE PLEASE INPUT FROM RANGE 255 TO -255\n");
  }
  char dirArray[dir.length()+1];
  strcpy(dirArray,dir.c_str());
  switch(*dirArray)
  {
    //TODO : find a way to rotate based on deg placement
    
    case *"F": //FORWARD CODE
       RightMotorA.run(-speedValue);
       RightMotorB.run(-speedValue);
       LeftMotorA.run(speedValue);
       LeftMotorB.run(speedValue);
       heading = "FORWARD"; 
    break;

    case *"B": // BACKWARD CODE
       RightMotorA.run(speedValue);
       RightMotorB.run(speedValue);
       LeftMotorA.run(-speedValue);
       LeftMotorB.run(-speedValue);  
       heading = "BACKWARD";
    break; 
    case *"R": // ROTATE RIGHT CODE 
       speedDifference = 70;
       RightMotorA.run(speedValue-speedDifference);
       RightMotorB.run(speedValue-speedDifference);
       LeftMotorA.run(speedValue);
       LeftMotorB.run(speedValue);  
       heading = "RIGHT ROTATION";
    break; 
    case *"L": // ROTATE LEFT CODE 
       speedDifference = 70;
       RightMotorA.run(-speedValue);
       RightMotorB.run(-speedValue);
       LeftMotorA.run(-speedValue+speedDifference);
       LeftMotorB.run(-speedValue+speedDifference);  
       heading = "LEFT ROTATION";
    break; 
    case *"l": // ROTATE left and back code
       RightMotorA.run(-speedValue);
       RightMotorB.run(-speedValue);
       LeftMotorA.run(-speedValue);
       LeftMotorB.run(-speedValue);  
       heading = "BACKWARDS ROTATION";
    break;

    case *"r":// ROTATE right and back code
       RightMotorA.run(speedValue);
       RightMotorB.run(speedValue);
       LeftMotorA.run(speedValue);
       LeftMotorB.run(speedValue);  
       heading = "BACKWARDS ROTATION";
    break;
    
    case *"A": //minor left Correction code
       speedDifference = 30 ; 
       RightMotorA.run(-speedValue-speedDifference);
       RightMotorB.run(-speedValue-speedDifference);
       LeftMotorA.run(speedValue-speedDifference);
       LeftMotorB.run(speedValue-speedDifference);  
       heading = "LEFT Correction";
    break;
    case *"D": //minor Right Correction code
       speedDifference = 30 ; 
       RightMotorA.run(-speedValue+speedDifference);
       RightMotorB.run(-speedValue+speedDifference);
       LeftMotorA.run(speedValue+speedDifference);
       LeftMotorB.run(speedValue+speedDifference);  
       heading = "LEFT ROTATION";
    break;

    case *"S": //STOP CODE
       RightMotorA.stop();
       RightMotorB.stop();
       LeftMotorA.stop();
       LeftMotorB.stop();  
       speedValue = 0;
       heading = "STOP";
    break;
    default : // error catching
      errorFlag+=String("Movement : INVALID direction please input a valid direction");
    break;
  }
  if(errorFlag!="")
  {
    Serial.println(errorFlag);
    return 0 ;
  }
  //Serial.print("Movement : ");Serial.print(heading);Serial.print(" at SpeedValue: " ); Serial.println(speedValue);
  if(multiTasking.threadHalt == false)
  {
    //Serial3.println(sensorValue._reccoAct);
  }
  //Serial3.print("Movement : ");Serial3.print(heading);Serial3.print(" at SpeedValue: " ); Serial3.println(speedValue);
}

void readSensor()
{
  //0 = 00  0=00  win condition    
  //1 = 01  1=01
  //2 = 10  2=10
  //3 = 11  3=11 out of boundary 
  //For ideal situation value shld be 2 1 
  
  // all possible outcome
  // 10 01 best condition (2+1)
  
  // 11 00 strafing left (3+0)
  // 10 00 possible right turn (2+0)
  
  // 00 11 strafing right (0+3)
  // 00 01 possible left turn (0+1)
  
  // 11 10 extreme right Straffing (3+2)
  // 01 11 extreme left Straffing (1+3)
    
  // 11 11 out of line (3+3)
  // 00 00 win condition or left and right turn possible (0+0)
  
  sensorValue.leftSensorState = leftSensor.readSensors();
  sensorValue.rightSensorState = rightSensor.readSensors();
  sensorValue.combinedState = sensorValue.leftSensorState + sensorValue.rightSensorState;
  
  if (sensorValue.leftSensorState!=0 && sensorValue.rightSensorState !=0)
  {
    //oncourse maintain bearing
    switch(sensorValue.combinedState)
    {
      case 6 :
        if(!sensorValue.robotRotationState)
        {
         sensorValue.reccomendedAction = RotateBackwards;
         sensorValue._reccoAct = "Backwards";
         
        }
       
      break;
      case 3:
        // Insert Perfect Condition Code here
        sensorValue._reccoAct = "Continue";
        sensorValue.reccomendedAction = Continue;
        sensorValue.robotRotationState = false; 
       
      break;
      default:
        if (sensorValue.leftSensorState == 3)
        {
          //Extreme leftStraffing 
          //INSERT IMMEDIATE CORRECTIVE ACTION HERE
           if(!sensorValue.robotRotationState)
            {
               sensorValue._reccoAct = "ImmediateRightCorrectiveOrder";
               //sensorValue.reccomendedAction = ImmediateRightCorrectiveOrder; 
            }   
          
        }
        else if (sensorValue.rightSensorState == 3)
        {
          //Extreme rightStraffing 
          //INSERT IMMEDIATE CORRECTIVE ACTION HERE 
          if(!sensorValue.robotRotationState)
          {
            sensorValue._reccoAct = "ImmediateLeftCorrectiveOrder";
            //sensorValue.reccomendedAction = ImmediateLeftCorrectiveOrder; 
          }   
          
        }
      break; 
    }
  }
  else if(sensorValue.leftSensorState ==0  && sensorValue.rightSensorState ==0 &&  !sensorValue.TLock )
  { 
    sensorValue._reccoAct = "LeftRightCondition";
    sensorValue.reccomendedAction = LeftRightCondition; 
  }
  else if (sensorValue.leftSensorState == 0) //Left priority checking 
  {
    //check for right straffing or left turn 
    
    switch(sensorValue.combinedState)
    {
       case 3 :
        // !TODO:
        // INSERT CORRECTIVE ACTION HERE 
        // High priority work
        // Add into memory straffing direction
        if(!sensorValue.robotRotationState )
        {
         sensorValue._reccoAct = "MinorLeftCorrection";
         sensorValue.reccomendedAction = MinorLeftCorrection; 
        
        }   
        
      break;
      case 1:
       // Insert Left rotation Code here
       if(!sensorValue.robotRotationState)
       {
        sensorValue._reccoAct = "LeftRotation";
        sensorValue.reccomendedAction = LeftRotation;
       }       
      break;
      default:
        if(sensorValue.rightSensorState != 0 )
        {
         
        }
      break; 
    }
  } 

  else if (sensorValue.rightSensorState == 0) //Right Checking here 
  {
    //check for left straffing or left turn 

    switch(sensorValue.combinedState)
    {
       case 3 :
        // !TODO:
        // INSERT CORRECTIVE ACTION HERE 
        // High priority work
        // Add into memory straffing direction
        if(!sensorValue.robotRotationState)
        {
          sensorValue._reccoAct = "MinorRightCorrection";
          sensorValue.reccomendedAction = MinorRightCorrection;
        }   
        
      break;
      case 2:
       // Insert right rotation Code here
       if(!sensorValue.robotRotationState)
       {
         sensorValue._reccoAct = "RightRotation";
         sensorValue.reccomendedAction =RightRotation;
         
       }
      break;
      default:
     
      break; 
    }
  }
}  
