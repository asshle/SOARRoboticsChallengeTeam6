# SOARRoboticsChallengeTeam6
This is the central repository page for SUTD SOAR challenge group 6, 2020 


## Functions
### GUI for 3 servo angles
send this command to arduino upon any value change
"Move:{angleOfGripperMotor}:{angleOfMotorOnArm}:{angleOfBaseMotor}"
"Move:110:20:60"

### Button for 1 and 0 for gripper change
"GripperChange:0"


### pause is a emergency stop button on the gui
"Pause:0"

### Save Angle state button and history

### Add button for save mode and follower mode

follower mode
- adjust accroding to GUI change of servo angle 
- save to history

save mode 
- send the command from the history to arduino
- have a handshake before sending the next one