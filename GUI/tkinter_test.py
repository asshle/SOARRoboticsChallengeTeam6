

#Import following packages
import serial
import tkinter
import tk_tools

#Function call to close your program
def close_window():
    window.destroy()
save_history = []

#Function call to move_servo button
def move_servo(var):
    # Getting angle from the srvo
    servo1_angle = servo.get()
    servo2_angle = servo2.get()
    servo3_angle = servo3.get()
    #set value to the speed gauge
    speed_gauge.set_value(servo1_angle) 
    speed_gauge2.set_value(servo2_angle) 
    speed_gauge3.set_value(servo3_angle) 
    print("save_history"+str(save_history))

    print(f"Move:{servo1_angle}:{servo2_angle}:{servo3_angle}") #print value for cross-check
    #send servo angle value to arduino
    #get servo angle ack from arduino to cross-check

# Create a Stack to store the history of positions
def save_state():
    # Getting angle from the srvo
    servo1_angle = servo.get()
    servo2_angle = servo2.get()
    servo3_angle = servo3.get()
    position = f"Move:{servo1_angle}:{servo2_angle}:{servo3_angle}"
    save_history.append(position)
    update_history(position)
    return save_history

def update_history(position):
    #<label name>.config(text="<new text>" + str(<variable name>))
    text_save_history.config(text="History:" + str(save_history),wraplength=1000)
    # for r in range(3):
    #     for c in range(4):
    #         text_save_history.config(text="History:" + str(save_history),
    #             borderwidth=1 ).grid(row=r,column=c)


# MAIN
window = tkinter.Tk() #create tkinter window
window.title("Servo angle control") #give title
window.configure(background="white") #change background color

servo = ["angleOfGripperMotor","angleOfMotorOnArm","angleOfBaseMotor"]

# Servo
# Create a slider for servo position
servo = tkinter.Scale(window, activebackground="blue",
label = " Set the Angle of Gripper Motor",
bg = "white", font=('Verdana',16), from_=0, to=180,
orient=tkinter.HORIZONTAL, length= 400,
command = move_servo)

servo2 = tkinter.Scale(window, activebackground="blue",
label = " Set the Angle of Motor On Arm",
bg = "white", font=('Verdana',16), from_=0, to=180,
orient=tkinter.HORIZONTAL, length= 400,
command = move_servo)

servo3 = tkinter.Scale(window, activebackground="blue",
label = " Set the Angle of Base Motor",
bg = "white", font=('Verdana',16), from_=0, to=180,
orient=tkinter.HORIZONTAL, length= 400,
command = move_servo)

# Speed Gauge
#Create a speed-gauge using tk-tools module
speed_gauge = tk_tools.Gauge(window, max_value=180, bg='white',
label='Servo angle', unit=' deg',
red=90, yellow=10, height = 300, width = 500)

#Create a speed-gauge using tk-tools module
speed_gauge2 = tk_tools.Gauge(window, max_value=180, bg='white',
label='Servo angle', unit=' deg',
red=90, yellow=10, height = 300, width = 500)

speed_gauge3 = tk_tools.Gauge(window, max_value=180, bg='white',
label='Servo angle', unit=' deg',
red=90, yellow=10, height = 300, width = 500)

button_exit = tkinter.Button(window, text="Exit", font=('Verdana',10),
padx=50, pady = 10,
command=close_window)

# button_save_state = tkinter.Button(window, text="Save", bg="red", font=('Verdana',10),
# padx=50, pady = 10,
# command=lambda : (save_history.append(f"Move:{servo.get()}:{servo2.get()}:{servo3.get()}")))

button_save_state = tkinter.Button(window, text="Save", bg="red", font=('Verdana',10),
padx=50, pady = 10,command=save_state)

text_save_history=tkinter.Label(window,text=str(save_history),font=('Verdana',10)
,padx=10,pady=10)
# Create a button to store the servo angle into the stack

#pack your buttons, sliders
servo.grid(row=1, column=0)
servo2.grid(row=1, column=1)
servo3.grid(row=1, column=2)
speed_gauge.grid(row=0, column=0)
speed_gauge2.grid(row=0, column=1)
speed_gauge3.grid(row=0, column=2)

button_exit.grid(row=2,column=0,columnspan=2)
button_save_state.grid(row=2,column=1,columnspan=2)

text_save_history.grid(row=3,column=0,columnspan=3,rowspan=10)


#execute the loop
window.mainloop()