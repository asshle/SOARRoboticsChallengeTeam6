import tkinter
def add():
    x = int(in1.get()) #get value from entry box
    y = int(in2.get()) #get value from entry box
    z = x + y
    label.configure(text= z, font= ('Verdana',16))
window = tkinter.Tk() #create tkinter window
window.title("Addition") #give title
window.configure(background="white") #change background color
button_on = tkinter.Button(window, text="Add",
font= ('Verdana',16), padx=60, pady =5,
bg="green",fg="white",
command = add)
button_on.grid(row=1,column=0, columnspan=2)
in1 = tkinter.Entry(window, width = 5, borderwidth=1, font= ('Verdana',16))
in1.grid(row = 0, column =0, padx=20, pady =10)
in2 = tkinter.Entry(window, width = 5, borderwidth=1, font= ('Verdana',16))
in2.grid(row = 0, column =1, padx=20, pady =10)
label = tkinter.Label(window, text="0", font= ('Verdana',16), bg="lightgreen")
label.grid(row=0, column=2, rowspan=2)
window.mainloop()