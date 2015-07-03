__author__ = 'Pravjot'
import beacon_processing
import camera_image_capture
import cv2
import time
if __name__ == "__main__":

    #beacon marker
    marker = False

    picam = camera_image_capture.PiCam()
    #calibrate camera
    #[ret, mtx, dist, rvecs, tvecs, tvecs] = picam.calibrate_camera()

    #wait for command from STM board to tell you that the drone is facing the write way
    #this will be a while statement waiting on a response from UART

    #infinite loop
    #get image from raspberry pi camera and store it
    camera, rawCapture = picam.PiVideo()

    #capture frames and process them
    for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
        image = frame.array
        if image is not None:
            #image = cv2.imread('output.jpg')

            #undistort
            #camera_image_capture.undistort_img(image, mtx, dist)

            #beacon_processing.locate_beacon_information('barcode_01.jpg')
            marker = beacon_processing.locate_beacon(image)

        if marker is  None:
            #send response to nav board we have not found it
            print('Cannot locate beacon!')
        else:
            distance = beacon_processing.distance_to_camera(image, marker.KNOWN_WIDTH, picam.FOCAL_LENGTH,marker)
           #send distance to controller

            #When the drone reaches a minimum distance from the beacon, go to the next beacon
            if distance[0] < marker.MIN_DISTANCE_BEACON:
                print('Go find next beacon!')
                if beacon_processing.send_next_beacon_info() is False:
                    break



            #Wait for response from STM board, for the drone to finish rotation
            #while loop

          # clear the stream in preparation for the next frame
        rawCapture.truncate(0)