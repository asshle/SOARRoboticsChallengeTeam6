//ope
#include <Servo.h>

struct HardwarePinsInterfacing
{
  //Insert all pin definitions here]
  int ultraTriggerPin; // ultra sonic trigger 
  int ultraEchoPin; // ultra sonic echo 
  int zDepthSensorPin ; // Ir sensor pin no
  int baseMotorPin; // base motor pin 
  int motorArm0Pin=3; // joint on base Pin 
  int motorArm1Pin=10; // joint on arm Pin 
  int motorGripperPin=9; // Gripper pin
  int holder = 3; // abituray for debugging
  bool gripperState = false; 
};
struct Processes
{
  bool threadHalt =false; // process will be paused if true
  int readCommandTimeSlice = 1; // interval for readCommand thread
  int readCommandLastTime=0;//last instance (in millis()) for sensor read
  int writeMotorTimeSlice = 100; // interval for readCommand thread
  int writeMotorLastTime=0;//last instance (in millis()) for sensor read
};

struct ArmsVars
{
  double len; // length of each segment of the arm
  double mX=0.0,mY =0.0 ; //starting position of each segment of the arm
  double polR=0.0,polTiter=0.0; // polar coordinate of the arm orientation
  int pinNo;
  bool desiredState = true; 
  Servo ArmServo;
  
};

void readCommand(void);
void ultraSonicReading(void);
void convertToPolarCoord(ArmsVars arm);
void initServoArms(ArmsVars arm);

HardwarePinsInterfacing hardware;
Processes processes;
ArmsVars motor [3];
int targetAngle[] = {90,90,70};  // motor angle target state

void setup() 
{
  Serial.begin(9600);
  initSystem(); 
}

void loop() 
{
  /// Concept below follows "multi threading" by using time slicing methods 
  /// for more info : https://learn.adafruit.com/multi-tasking-the-arduino-part-1/using-millis-for-timing 

  // scan cycle : readSensor -> ComputeAction ->WriteMotorAngle
  unsigned long currentTime = millis(); // updating clock cycle
  readCommand(); // reading input commands 
  if(!processes.threadHalt) //used in program pause
  {
    if( (unsigned long)(currentTime-processes.writeMotorLastTime)>=processes.writeMotorTimeSlice)
    {
     //Write Motor thread
      moveAllMotors(motor,targetAngle);
      //printMotorState();
      processes.writeMotorLastTime = currentTime;
    }
  }
  else
  {
    // events for when program is in halt
  }     

}
//=====================Initialise Codes =============================================
void initSystem() 
{
  initAllMotors(motor);
  Serial.println("System:System Initialised");
}
void initAllMotors(ArmsVars motor[])
{
  /// Pins definations can be found in HardwarePinsInterfacing struct 
  //pins : 9,10,11
  motor[0].pinNo=hardware.motorGripperPin;
  motor[1].pinNo=hardware.motorArm1Pin;
  motor[2].pinNo=hardware.motorArm0Pin;
  motor[3].pinNo=hardware.holder; // buffer space
   
  /*motor[0].ArmServo.attach(motor[0].pinNo);
  motor[1].ArmServo.attach(motor[1].pinNo);
  motor[2].ArmServo.attach(motor[2].pinNo);
  motor[3].ArmServo.attach(motor[3].pinNo); // buffer space*/
 
  for (int i =0 ; i<=sizeof(motor);i++)
  {
    //TODO: call void initServoArms(ArmsVars arm) here
    motor[i].ArmServo.attach(motor[i].pinNo);
    Serial.print("System: motor Attached at : motor : ");Serial.print(i);Serial.print(" Pin No : ");Serial.println(motor[i].pinNo);
  }

  //moveAllMotors(motor,targetAngle);
  printMotorState();   
  Serial.println("System:Motor Initialised");
}
//=====================Communications Codes =============================================

void printMotorState()
{
  Serial.print("System: motor0 :  ");Serial.print(motor[0].ArmServo.read());Serial.print(" ");
  Serial.print("motor1 :  ");Serial.print(motor[1].ArmServo.read());Serial.print(" ");
  Serial.print("motor2 :  ");Serial.print(motor[2].ArmServo.read());Serial.print(" ");
  Serial.print("threadHalt State :  ");Serial.print(processes.threadHalt);Serial.println(" ");
}
void readCommand()
{
  String command = Serial.readString();
  command.trim();
 
  if(command != "")
  {
    String method,paramHolding,directionCommand,speedValue;
    int _speedValue;
    
    method = command.substring(0,command.indexOf(':')); // needed for checking for valid command
    paramHolding = command.substring(command.indexOf(':')+1,command.length()); // needed for checking for valid command
    
    char methodArray[method.length()+1];
    strcpy(methodArray,method.c_str()); 

    Serial.println(methodArray);
    if(strcmp(methodArray ,"Move" )==0)
    {
      for(int i =0 ; i <=sizeof(targetAngle);i++)
      {
        targetAngle[i]= paramHolding.substring(0 ,paramHolding.indexOf(':')).toInt();
        paramHolding.remove(0 ,paramHolding.indexOf(':')+1);
        //Serial.print(targetAngle[i]);
      }
     
    }
    else if(strcmp(methodArray ,"Pause" )==0)
    {
      if (processes.threadHalt == false)
      {
        processes.threadHalt = true; 
      }
      else
      {
        processes.threadHalt = false; 
      }
    }
  }
}


//=====================Hardware Interfacing / conversion Codes=======================
void ultraSonicReading(void)
{
  ///ultraSonic Reading scan cycle thread
  
  digitalWrite(hardware.ultraTriggerPin, LOW);
  delayMicroseconds(2); //! TODO replace this with another form of non blocking code
  
  /// Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(hardware.ultraTriggerPin, HIGH);
  delayMicroseconds(10);//! TODO replace this with another form of non blocking code
  digitalWrite(hardware.ultraTriggerPin, LOW);
  
  /// Reads the echoPin, returns the sound wave travel time in microseconds
  int duration = pulseIn(hardware.ultraEchoPin, HIGH);
  
  /// Calculating the distance return is in cm
  // TODO: determine how to pass this to next thread/function 
  int distance= duration*0.01745;

  //IGNORE BOTTOM (USED DURING DIAGNOSTICS )
  /*if(formatted)
  {
    //Serial.print("Motor angle : ");Serial.print(radarMotor.read());Serial.print(" distance to object : ");Serial.println(distance); 
    //SerialBT.print("Motor angle : ");SerialBT.print(radarMotor.read());SerialBT.print(" distance to object : ");SerialBT.println(distance);
  }
  else
  {
    //Serial.print(radarMotor.read());Serial.print(" ");Serial.println(distance);
    //SerialBT.print(radarMotor.read());SerialBT.print(" ");SerialBT.println(distance);
  }*/
}
/*void moveAllMotors(ArmsVars motors[],int targetAngle[])
{
  for(int i = 0 ; i <=sizeof(motors);i++)
  {
    motors[i].ArmServo.write(targetAngle[i]); 
    Serial.println(motors[i].ArmServo.read());
    delay(2);
  }
}*/
void moveAllMotors(ArmsVars motors[],int targetAngle[])
{
  int stepCount = 1; 
  int numCompleted= 0;
  setAllParamFalse(motors);
  if (sizeof(motors) == sizeof(targetAngle))
  {
    // when angle and motor count are equal 
    while(numCompleted <= sizeof(motors))
    {
     //readCommand();
     Serial.print("Num completed: ");Serial.print(numCompleted);Serial.print(" Size of motor : ");Serial.println(sizeof(motors));
     for(int i = 0 ; i<=sizeof(motors); i++ )
      {
        if(motor[i].ArmServo.read() < targetAngle[i])
        {
          motor[i].ArmServo.write(motor[i].ArmServo.read()+stepCount);
        }
        else if(motor[i].ArmServo.read() > targetAngle[i])
        {
          motor[i].ArmServo.write(motor[i].ArmServo.read()-stepCount);
        }
        else if (motor[i].desiredState == false &&  motor[i].ArmServo.read() == targetAngle[i]  )
        {
          motor[i].desiredState = true; 
          numCompleted ++;
          Serial.print("Num Completed : ");Serial.println(numCompleted);
        }
        //Serial.print("motor ");Serial.print(i); Serial.print(" Current angle: ");Serial.print(motor[i].ArmServo.read());Serial.print(" target angle:");Serial.println(targetAngle[i]);
        //printMotorState();
        Serial.print("motor ");Serial.print(i); Serial.print("angle: ");Serial.println(motor[i].ArmServo.read());
      }
      
    }
    
  }
  
}
void setAllParamFalse(ArmsVars motor[] )
{
  for(int i =0 ; i <= sizeof(motor); i++)
  {
    motor[i].desiredState = false; 
  }
}
void moveMotor(ArmsVars motor , int deg)
{
  int stepCount = 1;
  if (motor.ArmServo.read() < deg)
  {
    while(motor.ArmServo.read() != deg)
    {
      motor.ArmServo.write(motor.ArmServo.read()+stepCount);
      //Serial.println(motor.ArmServo.read());
    } 
  }
  else
  {
    while(motor.ArmServo.read() != deg)
    {
      motor.ArmServo.write(motor.ArmServo.read()-stepCount);
      //Serial.println(motor.ArmServo.read());
    } 
  }
}

void gripperChange()
{
  if(motor[0].ArmServo.read() != 110)
  {
   motor[0].ArmServo.write(110);
   hardware.gripperState = true;
  }
  else
  {
  motor[0].ArmServo.write(90);
  hardware.gripperState = false;
  }
}
