# Python script to adjust the servo with GUI

## Controlling the Servo
Standard Servo
```
import time
import board
import pulseio
from adafruit_motor import servo
# create a PWMOut object on Pin A2.
pwm = pulseio.PWMOut(board.A2, duty_cycle=2 ** 15, frequency=50)
# Create a servo object, my_servo.
my_servo = servo.Servo(pwm)
while True:
for angle in range(0, 180, 5): # 0 - 180 degrees, 5 degrees at a time.
my_servo.angle = angle
time.sleep(0.05)
for angle in range(180, 0, -5): # 180 - 0 degrees, 5 degrees at a time.
my_servo.angle = angle
time.sleep(0.05)
```

Continuous Servo
```
# Continuous Servo Test Program for CircuitPython
import time
import board
import pulseio
from adafruit_motor import servo
# create a PWMOut object on Pin A2.
pwm = pulseio.PWMOut(board.A2, frequency=50)
# Create a servo object, my_servo.
my_servo = servo.ContinuousServo(pwm)
while True:
print("forward")
my_servo.throttle = 1.0
time.sleep(2.0)
print("stop")
my_servo.throttle = 0.0
time.sleep(2.0)
print("reverse")
my_servo.throttle = -1.0
time.sleep(2.0)
print("stop")
my_servo.throttle = 0.0
time.sleep(4.0)
```