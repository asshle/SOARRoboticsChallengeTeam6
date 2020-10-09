
#include <Servo.h>

void ultraSonicReading(void);

struct HardwarePinsInterfacing
{
  //Insert all pin definitions here]
  int ultraTriggerPin; // ultra sonic trigger 
  int ultraEchoPin; // ultra sonic echo 
  int zDepthSensorPin ; // Ir sensor pin no
  int baseMotorPin;
  int motorArm0Pin;
  int motorArm1Pin;
  int motorGripperPin;
  
};
struct Processes
{
  bool threadHalt =false; // process will be paused if true
  int readSensorTimeSlice = 50; // interval for readSensor thread
  int readSensorLastTime=0;//last instance (in millis()) for sensor read
};
struct ArmsVars
{
  double len; // length of each segment of the arm
  double mX=0.0,mY =0.0 ; //starting position of each segment of the arm
  double polR=0.0,polTiter=0.0; // polar coordinate of the arm orientation
  int pinNo;//
  Servo ArmServo;
  
};
HardwarePinsInterfacing hardware;
Processes processes;

void setup() 
{
  Serial.begin(9600);
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
    if( (unsigned long)(currentTime-processes.readSensorLastTime)>=processes.readSensorTimeSlice)
    {
     //readingSensor thread
     //readSensor();
     processes.readSensorLastTime = currentTime;
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
  //(arm.mX)^(2.0) +(arm.mY)^(2.0)
  arm.polR = sqrt(pow(arm.mX,2)+pow(arm.mY,2));
  arm.polTiter = 1/tan(arm.mY/arm.mX);
}

//=====================Initialise Codes =============================================
void initServoArms(int pinNo)
{
  
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
