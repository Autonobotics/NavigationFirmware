__author__ = 'Pravjot'
import beacon_processing
#import camera_image_capture
import cv2
if __name__ == "__main__":

    #focal length in pixels
    FOCAL_LENGTH = 13.606299213

    #in inches
    MIN_DISTANCE_BEACON = 2.0

    #beacon marker
    marker = False

    #calibrate camera
    #[ret, mtx, dist, rvecs, tvecs, tvecs] = camera_image_capture.calibrate_camera()

    #wait for command from STM board to tell you that the drone is facing the write way
    #this will be a while statement waiting on a response from UART

    #infinite loop
    #get image from raspberry pi camera and store it
    #imageFlag = camera_image_capture.get_PiImage()
    for i in range(0,5):
        imageFlag = True
        if imageFlag:
            image = cv2.imread('circles_rectangles_noise.jpg')

            #undistort
            #camera_image_capture.undistort_img(image, mtx, dist)

            #beacon_processing.locate_beacon_information('barcode_01.jpg')
            marker = beacon_processing.locate_beacon(image)

        if marker is  None:
            #send response to nav board we have not found it
            print('Cannot locate beacon!')
        else:
            distance = beacon_processing.distance_to_camera(image, marker.KNOWN_WIDTH, FOCAL_LENGTH,marker)
           #send distance to controller

        #When the drone reaches a minimum distance from the beacon, go to the next beacon
        if distance[0] < MIN_DISTANCE_BEACON:
            print('Go find next beacon!')
            beacon_processing.send_next_beacon_info()

        #Wait for response from STM board, for the drone to finish rotation
        #while loop
