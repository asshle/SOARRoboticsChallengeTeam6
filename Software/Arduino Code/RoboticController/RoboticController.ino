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
  if(!processes.threadHalt) //used in program pause
  {
    //Individual threads goes here
    /*if( (unsigned long)(currentTime-processes.readCommandLastTime)>=processes.readCommandTimeSlice)
    {
     //readingSensor thread
     readCommand(); 
     processes.readCommandLastTime = currentTime;
    }*/
    if( (unsigned long)(currentTime-processes.writeMotorLastTime)>=processes.writeMotorTimeSlice)
    {
     //Write Motor thread
     
      int targetAngle[] = {110,45,60};
      moveAllMotors(motor,targetAngle);

      //gripperChange();
      
      printMotorState();
      processes.writeMotorLastTime = currentTime;
    }
    
  }
  else
  {
    // events for when program is in halt
  }

  if(Serial.available())
  {
     readCommand();
  }
}
// =====================Computations Codes ==========================================

void convertToPolarCoord(ArmsVars arm)
{
  arm.polR = sqrt(pow(arm.mX,2)+pow(arm.mY,2));
  arm.polTiter = 1/tan(arm.mY/arm.mX);
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

  printMotorState();   
  Serial.println("System:Motor Initialised");
}
//=====================Communications Codes =============================================

void printMotorState()
{
  Serial.print("System: motor0 :  ");Serial.print(motor[0].ArmServo.read());Serial.print(" ");
  Serial.print("motor1 :  ");Serial.print(motor[1].ArmServo.read());Serial.print(" ");
  Serial.print("motor2 :  ");Serial.println(motor[2].ArmServo.read());Serial.print(" ");
}

void readCommand(void)
{
  processes.threadHalt= true;
  String Command = Serial.readString(); // reading incoming serial 
  Serial.print("System: DataDump :");Serial.print(Command);
  Command.trim();
  if(Command!= "" )
  {
    //char** commands = returnCommandParam(':' , Command);
    char CommandChar[Command.length()+1];  
    Command.toCharArray(CommandChar,Command.length());//Converting to char Array
    int returnArrayLen =countDelimiter(':',CommandChar,Command.length()); // getting no of occurance of delimiter
    char* subStr[returnArrayLen]; 
    
    returnCommandParam(':' , Command ,subStr,returnArrayLen);
    Serial.print("Main: ");Serial.println(subStr[1]);
  }
  processes.threadHalt= false;
}

void returnCommandParam(char delimiter ,String command,char* subStr[], int sizeOfArray) // test function 
{  
  int count= 0; 
  int j =0; 
  for(int i =0 ; i<=sizeOfArray;i++)
  {
    String holder=""; 
    while(command[j] != delimiter || j == sizeof(command) )
    {
      holder += command[j];
      j++;
    }
    j++; // to skip the delimiter
    subStr[i]=malloc(j* sizeof(char*));
    strcpy(subStr[i],holder.c_str());

    Serial.print("returnCommandParam : ");Serial.println(subStr[i]);
  }
  return subStr;
}



int countDelimiter (char delimiter,char CommandChar[],int charLen)
{
  /// Return count of delimiter in a given string 
  
  int count=0; 
  //Testing Points 
  //Serial.print("charLen : ");Serial.println(charLen);
  for(int j=0 ; j<=charLen ; j++)
  {
     //Testing Points 
    //Serial.println(CommandChar[j]);
    if(CommandChar[j]==delimiter)
    {
      count++;
    }
  }
  //Testing Points 
  //Serial.print("DelimiterCount : "); Serial.println(count);
  return count;
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
void moveAllMotors(ArmsVars motors[],int targetAngle[])
{
  int stepCount = 1; 
  int numCompleted= 0;
  setAllParamFalse(motors);
  if (sizeof(motors) == sizeof(targetAngle))
  {
    // when angle and motor count are equal 
    while(numCompleted < sizeof(motors)+1)
    {
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
        }
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
