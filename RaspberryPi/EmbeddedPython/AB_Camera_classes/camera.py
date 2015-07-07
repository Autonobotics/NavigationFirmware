__author__ = 'Pravjot'

from picamera.array import PiRGBArray
from picamera import PiCamera
import time
import cv2
from AB_camera_modules import image_transformations
import glob
import numpy as np

class PiCam:
    #Run the calibration
    FOCAL_LENGTH = 570.41995122
    RESOLUTION = [640, 480]

    #https://www.raspberrypi.org/documentation/hardware/camera.md
    HORIZONTAL_FOV = 53.50
    VERTICAL_FOX = 41.41

    def PiVideo(self):
    # initialize the camera and grab a reference to the raw camera capture
        camera = PiCamera()
        camera.resolution = (640, 480)
        camera.framerate = 30
        rawCapture = PiRGBArray(camera, size=(640, 480))

        # allow the camera to warmup
        time.sleep(2)

        return [camera, rawCapture]

    def get_PiImage(self):
        #intialize the camera and grab a reference to the raw camera capture
        camera = PiCamera()

        try:
            rawCapture = PiRGBArray(camera)
            #camera warmup time
            time.sleep(0.1)

            #grab an image from the camera
            camera.capture(rawCapture, format='bgr')
            image = rawCapture.array
            cv2.imwrite("output.jpg", image)

            #display the image on screen and wait for a keypress
            #cv2 .imshow("Raspberry Pi Image Capture", image)
            #cv2.waitKey(0)
        except ValueError:
            #log error
            print 'Could not capture image!'
            return False
        return True