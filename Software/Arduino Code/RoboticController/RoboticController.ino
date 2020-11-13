//ope
#include <Servo.h>

struct HardwarePinsInterfacing
{
  //Insert all pin definitions here]
  int ultraTriggerPin; // ultra sonic trigger 
  int ultraEchoPin; // ultra sonic echo 
  int zDepthSensorPin ; // Ir sensor pin no
  int baseMotorPin; // base motor pin 
  int motorArm0Pin=11; // joint on base Pin 
  int motorArm1Pin=10; // joint on arm Pin 
  int motorGripperPin=9; // Gripper pin
  bool gripperState = false; 
};
struct Processes
{
  bool threadHalt =false; // process will be paused if true
  int readCommandTimeSlice = 1; // interval for readCommand thread
  int readCommandLastTime=0;//last instance (in millis()) for sensor read
  int writeMotorTimeSlice = 1; // interval for readCommand thread
  int writeMotorLastTime=0;//last instance (in millis()) for sensor read
};

struct ArmsVars
{
  double len; // length of each segment of the arm
  double mX=0.0,mY =0.0 ; //starting position of each segment of the arm
  double polR=0.0,polTiter=0.0; // polar coordinate of the arm orientation
  int pinNo;//
  Servo ArmServo;
  
};

void readCommand(void);
void ultraSonicReading(void);
void convertToPolarCoord(ArmsVars arm);
void initServoArms(ArmsVars arm);

HardwarePinsInterfacing hardware;
Processes processes;
ArmsVars motor [2];

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
    if( (unsigned long)(currentTime-processes.readCommandLastTime)>=processes.readCommandTimeSlice)
    {
     //readingSensor thread
     readCommand(); 
     processes.readCommandLastTime = currentTime;
    }
    if( (unsigned long)(currentTime-processes.writeMotorLastTime)>=processes.writeMotorTimeSlice)
    {
     //Write Motor thread
     
      moveMotor(motor[2],100);
      moveMotor(motor[1],90);
      moveMotor(motor[0],110);
      //gripperChange();
      processes.writeMotorLastTime = currentTime;
    }
    
  }
  else
  {
    // events for when program is in halt
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
  
  //motor[2].ArmServo.attach(motor[2].pinNo);
  for (int i =0 ; i<=sizeof(motor);i++)
  {
    //TODO: call void initServoArms(ArmsVars arm) here
    motor[i].ArmServo.attach(motor[i].pinNo);
  }
  gripperChange();
  Serial.println("System:Motor Initialised");
}
//=====================Communications Codes =============================================
void readCommand(void)
{
  String Command = Serial.readString(); // reading incoming serial 
 
  Command.trim();
  //Serial.println("Test");
  //String param[] ;
  if(Command!= "" )
  {
    //Serial.println("ReadBack:"+Command);
    //Serial.println(Command);
    char** commands = returnCommandParam(':' , Command);
    //char** commands = myFunction();
    Serial.print("Main: ");Serial.println(commands[1]);
  }
}
/*char** myFunction() // test function 
{  
    char ** sub_str = malloc(10 * sizeof(char*));
    Serial.println("entered");
    char* test ="hello";
    for (int i =0 ; i < 10; ++i)
    {
      sub_str[i] = malloc(20 * sizeof(char));
      strcpy(sub_str[i],test);
      //Serial.println( sub_str[0]);
    } 
    Serial.println(sub_str[0]);
    return sub_str;
}

char** returnCommandParam(char delimiter , String Command)
{
  /// Returns  linked list of the command parameters
  
  char CommandChar[Command.length()+1];  
  int count= 0; 
  Command.toCharArray(CommandChar,Command.length()+1);//Converting to char Array
  int returnArrayLen =countDelimiter(delimiter,CommandChar,Command.length()); // getting no of occurance of delimiters
  char ** sub_str = malloc(returnArrayLen+1* sizeof(char*));
  
  for (int i =0 ; i <= returnArrayLen; ++i)
  {
    ///iterating through all the instance of "String" Array length
    
    char* temp=malloc(20* sizeof(char*));
    int tempSize = 0; 
    while(CommandChar[count]!=delimiter && count < sizeof(CommandChar))
    {
      
      tempSize++;
      count++;
    }
    count ++; // to skip the delimiter 
    //char tempTransfer[temp.length] = temp.c_str();
    
    //Setting String length in each instance
    sub_str[i] = malloc(tempSize * sizeof(char)); //Dynamically set length of the String length 
    strcpy(sub_str[i],temp);
    Serial.println(temp);
  }
  Serial.println(sub_str[0]);
  
  return sub_str;
}*/

char** returnCommandParam(char delimiter , String Command)
{
  /// Returns  linked list of the command parameters
  
  char CommandChar[Command.length()+1];
  int count= 0;
  Command.toCharArray(CommandChar,Command.length()+1);//Converting to char Array
  int returnArrayLen =countDelimiter(delimiter,CommandChar,Command.length()); // getting no of occurance of delimiters
  char ** sub_str = malloc(returnArrayLen+1* sizeof(char*));
  
  for (int i =0 ; i <= returnArrayLen; ++i)
  {
    ///iterating through all the instance of "String" Array length
    
    char* temp=malloc(20* sizeof(char*));
    int tempSize = 0;
    while(CommandChar[count]!=delimiter && count < sizeof(CommandChar))
    {
      temp[count]=CommandChar[count];
      tempSize++;
      count++;
    }
    count ++;// to skip the delimiter
    //char tempTransfer[temp.length] = temp.c_str();
    
    //Setting String length in each instance
    sub_str[i] = malloc(tempSize * sizeof(char)); //Dynamically set length of the String length
    strcpy(sub_str[i],temp);
    //Serial.println(temp);
  }
  //Serial.println(sub_str[0]);
  
  return sub_str;
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

void moveMotor(ArmsVars motor , int deg)
{
  if (motor.ArmServo.read() < deg)
  {
    while(motor.ArmServo.read() != deg)
    {
      motor.ArmServo.write(motor.ArmServo.read()+1);
      //Serial.println(motor.ArmServo.read());
    } 
  }
  else
  {
    while(motor.ArmServo.read() != deg)
    {
      motor.ArmServo.write(motor.ArmServo.read()-1);
      //Serial.println(motor.ArmServo.read());
    } 
  }
}

void gripperChange()
{

  Serial.println(motor[0].ArmServo.read());
  Serial.println(motor[0].ArmServo.attached());
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
