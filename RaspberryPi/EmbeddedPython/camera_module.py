__author__ = 'Pravjot'

from AB_Camera_Modules import beacon_processing
import AB_Camera_Modules.nav_board_comm as NAVcomm
from AB_Camera_Classes import ABcamera
from AB_Logging import ab_log
import numpy as np
import io
import cv2

def camera_loop():
    # beacon marker
    marker = None
    picam = ABcamera.PiCam()
    vector_dist = []

    #get image from raspberry pi camera and store it
    camera, rawCapture = picam.PiVideo()
    stream = io.BytesIO()

    #capture frames and process them
    for frame in camera.capture_continuous(stream, format='jpeg'):
        #image = frame.array
        # clear the stream in preparation for the next frame
        #rawCapture.truncate(0)

        # Construct a numpy array from the stream
        data = np.fromstring(stream.getvalue(), dtype=np.uint8)
        # "Decode" the image from the array, preserving colour
        image = cv2.imdecode(data, 1)

        if image is not None:
            #try and locate the beacon marker
            marker = beacon_processing.locate_beacon(image)

        if marker is None:
            print('Cannot locate beacon!')
            while NAVcomm.send_no_beacon() is not True:
                #wait for nav board to respond
                pass
        else:
            #X,Y,Z
            beaconDist = beacon_processing.distance_to_camera(image, picam, marker)
            vector_dist.append(beaconDist)
            while NAVcomm.send_beacon_detected(beaconDist) is not True:
                    #wait until the nav board has accepted the distance
                    pass

            if len(vector_dist) == 5:
                avg = beacon_processing.regAverage(vector_dist)
                vector_dist = []
                print('------AVERAGE: ({0}, {1}, {2})'.format(avg.x, avg.y, avg.z))

                #send distance to controller
                while NAVcomm.send_beacon_detected(avg) is not True:
                    #wait until the nav board has accepted the distance
                    pass

            #When the drone reaches a minimum distance from the beacon, go to the next beacon
            if beaconDist.z < marker.MIN_DISTANCE_BEACON:
                print('Go find next beacon!')
                if beacon_processing.send_next_beacon_info() is False:
                    break
                #Wait for response from STM board, for the drone to finish rotation
                #while loop
                while NAVcomm.query_drone_rotation() is not True:
                #wait for drone to complete the rotation
                    pass

