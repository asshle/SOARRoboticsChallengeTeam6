
using System.Windows.Forms;
using System;
using System;
using System.IO.Ports;
using System.Threading;

namespace Robotics_Testing
{
    public partial class lb_SerialReturn : Form
    {
        static SerialPort _serialPort;
        bool IsClosed = false;

        public lb_SerialReturn()
        {
            InitializeComponent();
            _serialPort = new SerialPort();
            _serialPort.PortName = "COM4";//Set your board COM
            _serialPort.BaudRate = 9600;

            try
            {
                _serialPort.Open();
            }
            catch 
            {
                // Settle errror handling here
            };

            Timer timer = new Timer();
            timer.Interval = 1000;
            timer.Tick += new System.EventHandler(timer_Tick);

            timer.Start();

        }
        private void Form1_Load(object sender, EventArgs e)
        {
            //A Thread to listen forever the serial port
            Thread Hilo = new Thread(ListenSerial);
            Hilo.Start();
        }

        int []data = { 90, 90, 70 };
        private void button2_Click(object sender, EventArgs e)
        {
            int stepCount = 1;
            data[0] += stepCount;
            data[1] += stepCount;
            data[2] += stepCount;

            _serialPort.Write("Move:" + data[0] + ":"+data[1] + ":"+data[2] + ":" );

        }

        private void ListenSerial()
        {
            while (!IsClosed)
            {
                try
                {
                    //read to data from arduino
                    string AString = _serialPort.ReadLine();

                    //write the data in something textbox
                    tb_SerialReturn.Invoke(new MethodInvoker(
                        delegate
                        {
                            tb_SerialReturn.Text = AString;
                        }
                        ));

                }
                catch { }
            }
        }
        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            //when the form will be closed this line close the serial port
            IsClosed = true;
            if (_serialPort.IsOpen)
                _serialPort.Close();
        }
    }
}

 

 