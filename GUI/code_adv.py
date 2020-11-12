import Tkinter
from pyfirmata import Arduino, SERVO
import tkSimpleDialog
from Tkinter import IntVar, Button, StringVar, Entry
from time import sleep
import sqlite3 as lite

class App:

    def __init__(self, port, pin1, pin2, pin3, pin4, pin5):
        # Setting up Arduino
        self.arduino = Arduino(port)

        self.servo1 = pin1
        self.servo2 = pin2
        self.servo3 = pin3
        self.servo4 = pin4
        self.servo5 = pin5

        self.arduino.digital[self.servo1].mode = SERVO
        self.arduino.digital[self.servo2].mode = SERVO
        self.arduino.digital[self.servo3].mode = SERVO
        self.arduino.digital[self.servo4].mode = SERVO
        self.arduino.digital[self.servo5].mode = SERVO

        # Setting up Database Connect
        path = 'C:/Users/Mohamad/Desktop/db/servo_2d.db'
        self.Connect = lite.connect(path)

        self.servo1OldVal = 0
        self.servo2OldVal = 0
        self.servo3OldVal = 0
        self.servo4OldVal = 0
        self.servo5OldVal = 0

        self.root = Tkinter.Tk()
        self.root.geometry('600x600')

        # GUI variables
        self.servo1Val = IntVar()
        self.servo2Val = IntVar()
        self.servo3Val = IntVar()
        self.servo4Val = IntVar()
        self.servo5Val = IntVar()

        self.pointName = StringVar()

        # GUI Components
        servo1_slider = Tkinter.Scale(self.root,
                                      label='Servo 1',
                                      length=400,
                                      from_=0, to_=360,
                                      orient=Tkinter.VERTICAL,
                                      variable=self.servo1Val)
        servo1_slider.grid(column=1, row=2)

        servo2_slider = Tkinter.Scale(self.root,
                                      label='Servo 2',
                                      length=400,
                                      from_=0, to_=180,
                                      orient=Tkinter.VERTICAL,
                                      variable=self.servo2Val)
        servo2_slider.grid(column=2, row=2)

        servo3_slider = Tkinter.Scale(self.root,
                                      label='Servo 3',
                                      length=400,
                                      from_=0, to_=180,
                                      orient=Tkinter.VERTICAL,
                                      variable=self.servo3Val)
        servo3_slider.grid(column=3, row=2)

        servo4_slider = Tkinter.Scale(self.root,
                                      label='Servo 4',
                                      length=400,
                                      from_=0, to_=180,
                                      orient=Tkinter.VERTICAL,
                                      variable=self.servo4Val)
        servo4_slider.grid(column=4, row=2)

        servo5_slider = Tkinter.Scale(self.root,
                                      label='Servo 5',
                                      length=400,
                                      from_=0, to_=60,
                                      orient=Tkinter.VERTICAL,
                                      variable=self.servo5Val)
        servo5_slider.grid(column=5, row=2)

        self.btnSave = Button(self.root, text='Save', command=self.onSaveClicked)
        self.btnSave.grid(column=1, row=0)

        self.btnGet = Button(self.root, text='Get', command=self.onGetClicked)
        self.btnGet.grid(column=6, row=0)

        self.pName = Entry(self.root, textvariable=self.pointName)
        self.pName.grid(column=0, row=0)

        self.root.after(100, self.onSliderChange)
        self.root.mainloop()

    #def OnSliderChange(self):
    #    check(val, oldval, pin)
    #def check(self, val, oldVal, pin):
    #    if (val != oldVal):
    #       self.arduino.digital[pin].write(val)
    #        sleep(0.01)
    #        oldVal = val

    def onSliderChange(self):
        if self.servo1Val.get() != self.servo1OldVal:
            self.arduino.digital[self.servo1].write(self.servo1Val.get())
            sleep(0.01)

            self.servo1OldVal = self.servo1Val.get()

        if self.servo2Val.get() != self.servo2OldVal:
            self.arduino.digital[self.servo2].write(self.servo2Val.get())
            sleep(0.01)

            self.servo2OldVal = self.servo2Val.get()

        if self.servo3Val.get() != self.servo3OldVal:
            self.arduino.digital[self.servo3].write(self.servo3Val.get())
            sleep(0.01)

            self.servo3OldVal = self.servo3Val.get()

        if self.servo4Val.get() != self.servo4OldVal:
            self.arduino.digital[self.servo4].write(self.servo4Val.get())
            sleep(0.01)

            self.servo4OldVal = self.servo4Val.get()

        if self.servo5Val.get() != self.servo5OldVal:
            self.arduino.digital[self.servo5].write(self.servo5Val.get())
            sleep(0.01)

            self.servo5OldVal = self.servo5Val.get()

        self.root.after(123, self.onSliderChange)

    def onSaveClicked(self):
        with self.Connect:
            cur = self.Connect.cursor()
            sql = "INSERT INTO test VALUES(?,?,?,?,?,?)"
            cur.execute(sql, (self.servo1Val.get(), self.servo2Val.get(),
                              self.servo3Val.get(), self.servo4Val.get(),
                              self.servo5Val.get(), self.pointName.get()))

    def onGetClicked(self):
        with self.Connect:
            cur = self.Connect.cursor()
            sql = "SELECT * FROM test WHERE point=?"
            cur.execute(sql, [self.pointName.get()])
            points = cur.fetchone()
            #print(points[0])
            print('Servo1: {0} Servo2: {1}Servo1: {2} Servo2: {3}Servo1: {4}'.format(str(points[0]), str(points[1]),
                                                                                     str(points[2]), str(points[3]),
                                                                                     str(points[4])))

            self.servo1Val.set(points[0])
            sleep(0.1)
            self.servo2Val.set(points[1])
            sleep(0.1)
            self.servo3Val.set(points[2])
            sleep(0.1)
            self.servo4Val.set(points[3])
            sleep(0.1)
            self.servo5Val.set(points[4])


if __name__=='__main__':
    port = 'COM7' #tkSimpleDialog(Tkinter.Tk(), 'Port: ')
    ap = App(port, 2, 3, 4, 5, 6)