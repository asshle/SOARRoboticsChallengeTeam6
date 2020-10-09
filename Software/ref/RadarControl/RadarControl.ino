/*
 * code written by : LIM PIN 
 * code vetted by : BERNARD CHUA KIM SWEE
 * Last editted : 120220 2321
 * 
 * the idea behind this project is similar to a radar system however our team are trying to render the data
 * created in the physical into the digital recreation for higher level processing our team hopes that one day
 * this idea can be used in automation systems and unmanned systems for better immersion 
 * 
 * the code below work on a multithreaded concept and uses timeslicing to thread individual functions 
 * the code below is still in developmental phase and still require a lot more testing before using it for
 * production.
 * 
 * please feel free to reproduce any part of this code 
 * 
 * if there is any bugs spotted in the code please feel free to update this idoit at email 
 * limpinpin123@gmail.com 
*/

//checking board type for cross board operations
#if ARDUINO_AVR_UNO
  #include <Servo.h>
#elif ESP32
  #include <ESP32Servo.h>
  #include "BluetoothSerial.h"
#endif


void commandEvent();
String SerialBTRead();
void radarRotation();
void ultraSonicReading();
void help();

struct timeScheduling
{
  const unsigned long servoTimeSlice=50; 
  const unsigned long ultraSonicTimeSlice=50;
  unsigned long servoLastTime=0;
  unsigned long ultraSonicLastTime=0;
  bool threadHalt = false; 
  //TODO:Implement priority que system to allow for immediate soft interrupt
};
enum ConnType{serial,serialBT};

//Objects
BluetoothSerial SerialBT;
Servo radarMotor;
timeScheduling multiTasking;
//Pins used



//Arduino Uno pins config
/*const int MotorControlPin = 3; //Digital Pin 3  
const int ultraTriggerPin = 9;
const int ultraEchoPin = 10;*/

//ESP 32 pins config
const int MotorControlPin = 13; //Digital Pin 3  
const int ultraTriggerPin = 14;
const int ultraEchoPin = 12;


//Global variables 
const int motorResolution = 1;
unsigned int motorDeg=0;
unsigned long duration;
unsigned long distance;
String Message;
bool motorIncremental=true;
bool formatted =true;
  
void setup() 
{
  pinMode(ultraTriggerPin,OUTPUT);
  pinMode(ultraEchoPin,INPUT);
  Serial.begin(115200);
  SerialBT.begin("ESP32test");
  radarMotor.attach(MotorControlPin);

}

void loop() 
{
  unsigned long currentTime = millis();
  if(!multiTasking.threadHalt)
  {
    //Individual threads goes here
    if( (unsigned long)(currentTime-multiTasking.servoLastTime)>=multiTasking.servoTimeSlice)
    {
     //motor Rotating thread
     radarRotation();
    }
    if( (unsigned long)(currentTime-multiTasking.ultraSonicLastTime) >=multiTasking.ultraSonicTimeSlice)
    {
     //Sensor input thread
     ultraSonicReading();
    }
  }
  if(Serial.available()>0)
  {
    //read incoming commands from either bluetooth or serial command
    commandEvent(serial);
  }
  if(SerialBT.available()>0) 
  { 
   char incoming = SerialBT.read();//More efficient then readString
   if(incoming != '\n')
   {
     Message += String(incoming);
   }
   else 
   {
     Message.trim(); //Cleaning out all buffer space
     commandEvent(serialBT);
   }
  }
}

void commandEvent(ConnType CT)
{
 String commands;
 
 switch(CT)
 {
  case serialBT:
    commands =String(Message); 
    Message = "";  
  break;
  case serial:
    commands = Serial.readString();
  break;
  default:
  break;
 
 }
 
 int Value = commands.toInt();
 Serial.println(commands);
 if(commands == "start" )
 {
  radarMotor.attach(MotorControlPin); // this is to reattach the motor pin on to the servo object
  multiTasking.threadHalt =false;
 }
 else if(commands == "stop")
 {
 // radarMotor.detach(); // this is to prevent the flow of power flowing into the servo 
  multiTasking.threadHalt = true ;
 }
 else if(commands == "readUltra" )
 {
  ultraSonicReading();
 }
 else if(commands == "readServo" )
 {
  getRadarRotDeg();
 }
 else if (commands == "toggleFormat")
 {
  if(formatted)
   formatted = false;
  else
    formatted =true;
 }
 else if (commands =="help")
 {
  multiTasking.threadHalt = true ;
  help();
  //multiTasking.threadHalt = false ;
 }
 else if(Value>=0 && Value<=120)
 {
  radarMotor.attach(MotorControlPin);
  multiTasking.threadHalt = true; 
  radarMotor.write(Value);
  //radarMotor.detach();
 }
 else
 {
  Serial.println("invalid command");
 }
  
}

void radarRotation()
{
  //rotation of servo motor thread
  
  radarMotor.write(motorDeg);
  if(motorIncremental == true && motorDeg<=120 )
  {
     motorDeg+=motorResolution; //updating the next write positing
  }
  else
  {
    motorIncremental =false;
    motorDeg-=motorResolution;
    if(motorDeg<=0) motorIncremental =true;
  }
 
  multiTasking.servoLastTime = millis();
}

//TODO: make a varible to turn on/ off bluetooth response
void getRadarRotDeg()
{
  //used for diagnostics
  //called in serial event
  Serial.println(radarMotor.read());
  SerialBT.println(radarMotor.read());
  //since this function is not in main cycle, millis update is not required here
}

void ultraSonicReading()
{
  //ultraSonic Reading scan cycle thread
  
  digitalWrite(ultraTriggerPin, LOW);
  delayMicroseconds(2); //! TODO replace this with another form of non blocking code
  
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(ultraTriggerPin, HIGH);
  delayMicroseconds(10);//! TODO replace this with another form of non blocking code
  digitalWrite(ultraTriggerPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(ultraEchoPin, HIGH);
  
  // Calculating the distance return is in cm
  distance= duration*0.034/2;
   
  if(formatted)
  {
    Serial.print("Motor angle : ");Serial.print(radarMotor.read());Serial.print(" distance to object : ");Serial.println(distance); 
    SerialBT.print("Motor angle : ");SerialBT.print(radarMotor.read());SerialBT.print(" distance to object : ");SerialBT.println(distance);
  }
  else
  {
    Serial.print(radarMotor.read());Serial.print(" ");Serial.println(distance);
    SerialBT.print(radarMotor.read());SerialBT.print(" ");SerialBT.println(distance);
  }
  multiTasking.ultraSonicLastTime = millis();
}
void help()
{
  //TODO : find a way to make this for efficenct and robust
  String help = "---------------HELP---------------------------\n\nstart\n-Starts the rotation process and the scan cycle of the ultraSonic sensor and servo motor \n-works by attaching a boolean variable to track the state of the process loop \n\nstop\n-Stops the rotation process and the scan cycle of the ultraSonic sensor and servo motor\n-works by changing the boolean variable to control the process loop \n-see start function\n\n\n-reads the current ultra sonic range value \n-furthest guranteed range is 170cm (<- subjected to testings ) \n-please take note original data type for the return value is in long \n\nreadServo\n-return the current servo motor angle angle can only be from 0 to 120\n-take note that original data type is in int  \n\ntoggleFormat\n-toggle between formatted return string or non formated \n-take note that original state is formated  \n-formatted = Motor angle <angle of servo motor> distance to object : <distance of the object>  \n-Non formatted = <angle of servo motor> <distance of the object> \n\n<0-120>\n-set the angle of the servo motor to the specified angle\n-please do not exceed the range of 0-120 \n\n---------------HELP---------------------------";
  Serial.println(help); 
  SerialBT.println(help); 
  // I do sincerly apologise for the monstrosity above but i need to place the help list into a varible for different printing format
  // for easier reference for the help statement please do access file location : "radarProject\shared\CommandHelp.txt"
  // sorry for giving cancer :(
}




