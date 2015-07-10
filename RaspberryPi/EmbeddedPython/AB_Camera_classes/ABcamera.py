__author__ = 'Pravjot'

from picamera.array import PiRGBArray
from picamera import PiCamera
import time

class PiCam:
    #Run the calibration
    FOCAL_LENGTH = 570.41995122

    #https://www.raspberrypi.org/documentation/hardware/camera.md
    HORIZONTAL_FOV = 53.50
    VERTICAL_FOX = 41.41
    RESOLUTION = [640, 480]

    def PiVideo(self):
    # initialize the camera and grab a reference to the raw camera capture
        camera = PiCamera()
        camera.resolution = (640, 480)
        camera.framerate = 30

        # allow the camera to warmup
        time.sleep(2)

        return camera

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

        except ValueError:
            #log error
            print 'Could not capture image!'
            return None
        return image