__author__ = 'Dan'

# http://www.raspberry-projects.com/pi/pi-operating-systems/raspbian/io-pins-raspbian/uart-pins
# http://pyserial.sourceforge.net/pyserial_api.html

from serial import Serial
import time


serialPort = Serial("/dev/ttyAMA0", 115200)
if serialPort.isOpen() is False:
    serialPort.open()

syncPacket = [0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]

serialPort.flushInput()
serialPort.flushOutput()

inStr = serialPort.read(16)
print "Received Message: {0}".format(inStr)

output = serialPort.write(str(syncPacket))

print "Sending: {0!s}".format(syncPacket)
if output != 16:
    print "Sending did not send 16 bytes as expected"
time.sleep(0.05)
inStr = serialPort.read(16)
print "Received Message: {0}".format(inStr)

serialPort.close()
