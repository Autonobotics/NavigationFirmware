__author__ = 'Pravjot'
import time
import picamera
from fractions import Fraction

with picamera.PiCamera() as camera:
    # Set a framerate of 1/6fps, then set shutter
    # speed to 6s and ISO to 800
    camera.framerate = Fraction(1, 6)
    camera.shutter_speed = 6000000
    camera.exposure_mode = 'off'
    camera.iso = 800
    # Give the camera a good long time to measure AWB
    # (you may wish to use fixed AWB instead)
    time.sleep(10)
    for i, filename in enumerate(camera.capture_continuous('capture{counter:02d}.jpg')):
        print(filename)
        time.sleep(0.5)
        if i == 60:
            break
