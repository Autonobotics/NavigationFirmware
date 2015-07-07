__author__ = 'Pravjot'

from AB_camera_modules import beacon_processing, image_transformations
from ARMPIT import protocol as ARMPit
from AB_Camera_classes import camera
from AB_Logging import ab_log ab as AB_Log
import cv2
import time

if __name__ == "__main__":
    #beacon marker
    marker = None
    picam = camera.PiCam()
    vector_dist = []

    #intialize handshake
    ARMPit.perform_handshake();

    #wait for command from STM board to tell you that the drone is facing the write way
    #this will be a while statement waiting on a response from UART
    cmd = ARMPit.uart_receive_cmd()

    #infinite loop
    #get image from raspberry pi camera and store it
    camera, rawCapture = picam.PiVideo()

    #capture frames and process them
    for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
        image = frame.array
        if image is not None:
            #undistort
            #camera_image_capture.undistort_img(image, mtx, dist)
            #beacon_processing.locate_beacon_information('barcode_01.jpg')
            marker = beacon_processing.locate_beacon(image)

        if marker is  None:
            #send response to nav board we have not found it
            print('Cannot locate beacon!')
        else:
            distance, x, y = beacon_processing.distance_to_camera(image, picam, marker)
            vector_dist.append(distance)

            if len(vector_dist) == 5:
                #avg = beacon_processing.movingAverage(vector_dist, 10)
                avg = beacon_processing.regAverage(vector_dist)
                vector_dist = []
                print('AVERAGE: {0}'.format(avg))

           #send distance to controller


            #When the drone reaches a minimum distance from the beacon, go to the next beacon
            if distance < marker.MIN_DISTANCE_BEACON:
                print('Go find next beacon!')
                if beacon_processing.send_next_beacon_info() is False:
                    break



            #Wait for response from STM board, for the drone to finish rotation
            #while loop
          # clear the stream in preparation for the next frame
        rawCapture.truncate(0)