__author__ = 'Pravjot'

from AB_Camera_Modules import beacon_processing
import AB_Camera_Modules.nav_board_comm as Nav_Board_Comm
from AB_Camera_Classes import beacon
from AB_Camera_Classes import ABcamera
from AB_Logging import ab_log as AB_Log
import numpy as np
import io
import cv2
import time
import picamera
import picamera.array
import traceback, sys
global cam_logger
import RPi.GPIO as GPIO

def camera_loop():
    cam_logger = AB_Log.get_logger('AB_CAMERA_MODULE')
    # beacon marker
    marker = None
    markerList = []
    prevMarker = None
    GPIO.setwarnings(False)
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(22, GPIO.OUT)
    GPIO.output(22, False)
    try:
        with picamera.PiCamera() as camera:
            camera.resolution = (640, 480)
            camera.framerate = 30
            time.sleep(2)
            with picamera.array.PiRGBArray(camera) as stream:
            #capture frames and process them
                for frame in camera.capture_continuous(stream, format='bgr'):
                    image = stream.array
                    if image is not None:
                        #try and locate the beacon marker
                        marker = beacon_processing.locate_beacon(image)

                    if Nav_Board_Comm.ABFlags.STATUS == Nav_Board_Comm.ABFlags.COLLISION_DETECTED:
                        distance  = beacon_processing.frontal_collision(image, ABcamera.PiCam.FOCAL_LENGTH)
                        Nav_Board_Comm.send_and_wait(distance,Nav_Board_Comm.ABFlags.AVOID_FRONT)
                        Nav_Board_Comm.ABFlags.STATUS = 0x0


                    if marker is None:
                        markerList.append(beacon.beaconLocation(0, 0, 0))

                    else:
                        beaconDist = beacon_processing.distance_to_camera(ABcamera.PiCam.FOCAL_LENGTH, ABcamera.PiCam.RESOLUTION,ABcamera.PiCam.HORIZONTAL_FOV, marker)
                        markerList.append(beaconDist)

                        #When the drone reaches a minimum distance from the beacon, go to the next beacon
                        if beaconDist.z < marker.MIN_DISTANCE_BEACON:
                            cam_logger.debug('BEACON: Threshold distance reached, loading next rotation')

                            #cause when there are no more beacons left
                            if beacon_processing.send_next_beacon_info() is False:
                                cam_logger.debug('BEACON: Rotation information was not sent successfully!!')
                                break
                            else:
                                markerList = []
                                #Wait for response from STM board, for the drone to finish rotation
                                Nav_Board_Comm.send_and_wait(None, Nav_Board_Comm.ABFlags.QUERY_ROTATION)

                    if len(markerList) == 3:
                        prevMarker = beacon_processing.beacon_filter(markerList)

                        #NO BEACON DETECTED
                        if prevMarker is None:
                            Nav_Board_Comm.send_and_wait(None, Nav_Board_Comm.ABFlags.NO_BEACON_DETECTED)
                            GPIO.output(22, False)
                        #BEACON DETECTED
                        else:
                            Nav_Board_Comm.send_and_wait(prevMarker, Nav_Board_Comm.ABFlags.BEACON_DETECTED)
                            GPIO.output(22, True)

                        del markerList[0]
                        prevMarker = None
                    else:
                        if marker is None:
                            Nav_Board_Comm.send_and_wait(None, Nav_Board_Comm.ABFlags.NO_BEACON_DETECTED)
                            GPIO.output(22, False)

                        elif marker is not None:
                            Nav_Board_Comm.send_and_wait(beaconDist, Nav_Board_Comm.ABFlags.BEACON_DETECTED)
                            GPIO.output(22, True)
                    # Reset the stream for the next capture
                    stream.seek(0)
                    stream.truncate()

    except Exception, e:
        cam_logger.error(e)
        camera.close()
        cam_logger.error('ERROR: %s \r\n' % str(e))
        traceback.print_exc(file=sys.stdout)