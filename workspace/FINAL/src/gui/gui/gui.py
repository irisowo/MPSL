import sys
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5.QtGui import *
import serial
import time
COM_PORT = 'COM11'    # 指定通訊埠名稱
BAUD_RATES = 9600    # 設定傳輸速率
ser = serial.Serial(COM_PORT, BAUD_RATES)   # 初始化序列通訊埠

class mainwin(QWidget):
    def __init__(self):
        super(mainwin,self).__init__()
        self.resize(1000,800)
        self.setWindowTitle("Car Controller")
        
        self.button1=QPushButton("exit",self)
        self.button1.clicked.connect(self.close)
        self.button1.setCheckable(True)
        self.button1.toggle()

        self.button2=QToolButton()
        self.button2.setFixedSize(240, 160)
        self.button2.setIcon(QIcon(QPixmap("icons/left.png")))
        self.button2.setToolButtonStyle(Qt.ToolButtonIconOnly)
        self.button2.setIconSize(QSize(200, 120))
        self.button2.clicked.connect(lambda:self.send_signal("left"))

        self.button3=QToolButton()
        self.button3.setFixedSize(240, 160)
        self.button3.setIcon(QIcon(QPixmap("icons/up.png")))
        self.button3.setToolButtonStyle(Qt.ToolButtonIconOnly)
        self.button3.setIconSize(QSize(200, 120))
        self.button3.clicked.connect(lambda:self.send_signal("up"))

        self.button4=QToolButton()
        self.button4.setFixedSize(240, 160)
        self.button4.setIcon(QIcon(QPixmap("icons/right.png")))
        self.button4.setToolButtonStyle(Qt.ToolButtonIconOnly)
        self.button4.setIconSize(QSize(200, 120))
        self.button4.clicked.connect(lambda:self.send_signal("right"))

        self.button5=QToolButton()
        self.button5.setFixedSize(240, 160)
        self.button5.setIcon(QIcon(QPixmap("icons/down.png")))
        self.button5.setToolButtonStyle(Qt.ToolButtonIconOnly)
        self.button5.setIconSize(QSize(200, 120))
        self.button5.clicked.connect(lambda:self.send_signal("down"))


        self.setlayout_()
        
    
    def send_signal(self, dir):
        if(dir == "left"):
            ser.write(b'left\r')
            #time.sleep(2)
            ser.cancel_write()
        elif(dir == "right"):
            ser.write(b'right\r')
            #time.sleep(2)
            ser.cancel_write()
        elif(dir == "up"):
            ser.write(b'straight\r')
            #time.sleep(2)

            ser.cancel_write()
        else:
            ser.write(b'backward\r')
            #time.sleep(2)
            ser.cancel_write()
        
    
    def setlayout_(self):
        self.arr=QGridLayout(self)
        self.arr.addWidget(self.button3, 0, 1)
        self.arr.addWidget(self.button2, 1, 0)
        self.arr.addWidget(self.button4, 1, 2)
        self.arr.addWidget(self.button5, 2, 1)
        self.arr.setSpacing(15)
        self.setLayout(self.arr)
       
        
if __name__=="__main__":

    app=QApplication(sys.argv)
    MainWindow=mainwin()
    MainWindow.show()
    app.exec_()