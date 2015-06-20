__author__ = 'Dan'

import RPi.GPIO as GPIO
import time
GPIO.setmode(GPIO.BCM)

TRIG = 16
ECHO = 20

print "Distance Measurement In Progress"

GPIO.setup(TRIG,GPIO.OUT)
GPIO.setup(ECHO,GPIO.IN)

GPIO.output(TRIG, False)
print "Waiting For Sensor To Settle"
time.sleep(2)

GPIO.output(TRIG, True)
time.sleep(0.00001)
GPIO.output(TRIG, False)

while GPIO.input(ECHO)==0:
  pulse_start = time.time()

while GPIO.input(ECHO)==1:
  pulse_end = time.time()

pulse_durations = pulse_end - pulse_start

distance = pulse_durations * 17150

distance = round(distance, 2)

print "Distance:",distance,"cm"

GPIO.cleanup()