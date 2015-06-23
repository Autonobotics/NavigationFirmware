__author__ = 'Dan'

# http://www.raspberry-projects.com/pi/pi-operating-systems/raspbian/io-pins-raspbian/uart-pins
# http://pyserial.sourceforge.net/pyserial_api.html

from serial import Serial
import time


serialPort = Serial("/dev/ttyAMA0", 115200)
if (serialPort.isOpen() == False):
    serialPort.open()

outStr = ' ****UART_TwoBoards_ComIT****  ****UART_TwoBoards_ComIT****  ****UART_TwoBoards_ComIT**** '

serialPort.flushInput()
serialPort.flushOutput()

serialPort.write(outStr)
time.sleep(0.05)
inStr = serialPort.read(serialPort.inWaiting())
print "Received: {0}".format(inStr)

serialPort.close()
