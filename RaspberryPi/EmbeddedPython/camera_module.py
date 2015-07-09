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
global cam_logger


def camera_loop():
    cam_logger = AB_Log.get_logger('AB_CAMERA_MODULE')
    # beacon marker
    marker = None
    markerList = []
    prevMarker = None

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

                    if marker is None:
                        markerList.append(beacon.beaconLocation(0, 0, 0))

                    else:
                        beaconDist = beacon_processing.distance_to_camera(ABcamera.PiCam.FOCAL_LENGTH, ABcamera.PiCam.RESOLUTION,ABcamera.PiCam.HORIZONTAL_FOV, marker)
                        markerList.append(beaconDist)

                        #When the drone reaches a minimum distance from the beacon, go to the next beacon
                        if beaconDist.z < marker.MIN_DISTANCE_BEACON:
                            cam_logger.debug('BEACON: Threshold distance reached, loading next rotation')
                            print('Go find next beacon!')
                            #cause when there are no more beacons left
                            if beacon_processing.send_next_beacon_info() is False:
                                cam_logger.debug('BEACON: Rotation information was not sent successfully!!')
                                break
                            else:
                                markerList = []
                                cam_logger.debug('BEACON: Rotation information was not sent successfully!!')

                                #Wait for response from STM board, for the drone to finish rotation
                                #Nav_Board_Comm.send_and_wait(None, Nav_Board_Comm.ABFlags.QUERY_ROTATION)

                    if len(markerList) == 10:
                        prevMarker = beacon_processing.beacon_filter(markerList)
                        if prevMarker is None:
                            #Nav_Board_Comm.send_and_wait(None, Nav_Board_Comm.ABFlags.NO_BEACON_DETECTED)
                            cam_logger.info('BEACON: Not located')
                            print('NO BEACON')
                        else:
                            #Nav_Board_Comm.send_and_wait(prevMarker, Nav_Board_Comm.ABFlags.BEACON_DETECTED)
                            cam_logger.info('BEACON: Location --- ({0}, {1}, {2}) ---'.format(beaconDist.x, beaconDist.y, beaconDist.z))
                            print('BEACON: ({0}, {1}, {2})'.format(beaconDist.x, beaconDist.y, beaconDist.z))

                        del markerList[0]
                        prevMarker = None
                    else:
                        if marker is None:
                            cam_logger.info('BEACON: Not located')
                            #Nav_Board_Comm.send_and_wait(None, Nav_Board_Comm.ABFlags.NO_BEACON_DETECTED)
                            print('NO BEACON - i')
                        elif marker is not None:
                            #Nav_Board_Comm.send_and_wait(beaconDist, Nav_Board_Comm.ABFlags.BEACON_DETECTED)
                            cam_logger.info('BEACON: Location --- ({0}, {1}, {2}) ---'.format(beaconDist.x, beaconDist.y, beaconDist.z))
                            print('BEACON - i: ({0}, {1}, {2})'.format(beaconDist.x, beaconDist.y, beaconDist.z))
                    # Reset the stream for the next capture
                    stream.seek(0)
                    stream.truncate()

    except Exception, e:
        print(e)
        cam_logger.error('ERROR: %s' % str(e))