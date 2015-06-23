__author__ = 'Pravjot'
import beacon_processing
#import camera_image_capture
import cv2
if __name__ == "__main__":

    #focal length in pixels
    FOCAL_LENGTH = 13.606299213
    #intialize the known object width in inches
    KNOWN_WIDTH = 11.0

    #calibrate camera
    #[ret, mtx, dist, rvecs, tvecs, tvecs] = camera_image_capture.calibrate_camera()

    #get image from raspberry pi camera
    #image = camera_image_capture.get_PiImage()

    #just for testing
    image = cv2.imread('pokemon_games.png')

    #undistort
    #camera_image_capture.undistort_img(image, mtx, dist)

    #beacon_processing.locate_beacon_information('barcode_01.jpg')
    marker = beacon_processing.locate_beacon(image)
    if marker != False:
        distance = beacon_processing.distance_to_camera(image,KNOWN_WIDTH,FOCAL_LENGTH,marker)
    else:
        #send response to nav board we have not found it
        print('Cannot locate beacon!')