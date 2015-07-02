__author__ = 'Pravjot'

import numpy as np
import cv2
from AB_Camera_Module import beaconClass


AB_beaconList = beaconClass.AB_beacons()

def AB_startup():
    #send be intial beacon lcation to STM board
    startup_beacon_info = AB_beaconList.beacon_info(AB_beaconList.currentID)
    print("Initial Startup beacon information: {0}".format(startup_beacon_info))
    return

#send the next beacons location to the flight controller
def send_next_beacon_info():
    currentB = AB_beaconList.currentID
    nextB = AB_beaconList.nextID
    if currentB == nextB:
    #No more beacon left to navigate to
        print("no more beacons left to navigate to")
    else:
        AB_beaconList.next_beacon()
        #UART command to send information to STMF board
        next_beacon = AB_beaconList.beacon_info(AB_beaconList.currentID)
        print("Next beacon information: {0}".format(next_beacon))
    return

# search the image for the beacon, assuming that we use some kind of RED source
def locate_beacon(image):
    img = cv2.medianBlur(image, 5)
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)

    # HSV properties
    lower = [160, 100, 100]
    upper = [179, 255, 255]

    # create numpy arrays from the bounadires
    lower = np.array(lower, dtype="uint8")
    upper = np.array(upper, dtype="uint8")
    mask = cv2.inRange(hsv, lower, upper)

    #smooth the image by applying gaussian blur
    blurr = cv2.GaussianBlur(mask, (9, 9), 2)

    #img = cv2.medianBlur(img,5)
    #[h, s, v] = cv2.split(blurr)
    #cimg = cv2.cvtColor(mask,cv2.)

    circles = cv2.HoughCircles(blurr, cv2.HOUGH_GRADIENT, 1, 20,
                               param1=50, param2=30, minRadius=0, maxRadius=0)

    circles = np.uint16(np.around(circles[0, :]))

    for (x, y, r) in circles:
        # draw the outer circle
        cv2.circle(img, (x, y), r, (0, 255, 0), 2)
        marker = beaconClass.marker(x, y, r)
        print('MARKER LOCATION X: {0}, Y: {1}, R: {2}'.format(marker.x, marker.y, marker.r))
        # draw the center of the circle
        cv2.circle(img, (x, y), 2, (0, 255, 0), 3)

    return marker


# locate the information stored in the beacon (barcode), return the subimage containing the barcode information
def locate_beacon_information(image):
    # load the image and convert it to grayscale
    orig = image.copy()
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

    #compute the Scharr gradient magnitude represntation of the images in both x and y direction
    gradX = cv2.Sobel(gray, ddepth=cv2.CV_32F, dx=1, dy=0, ksize=-1)
    gradY = cv2.Sobel(gray, ddepth=cv2.CV_32F, dx=0, dy=1, ksize=-1)

    #subtract the y-gradient from the x-gradient
    gradient = cv2.subtract(gradX, gradY)
    gradient = cv2.convertScaleAbs(gradient)

    #filter out the noise in the image and focus solely on the barcode region
    # blur and threshold the image
    blurred = cv2.blur(gradient, (9, 9))
    ret, thresh = cv2.threshold(blurred, 225, 255, cv2.THRESH_BINARY)


    #construct a closing kernel and apply it to the thresholded image
    kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (21, 7))
    closed = cv2.morphologyEx(thresh, cv2.MORPH_CLOSE, kernel)

    #perform a series of erosions and dilations
    closed = cv2.erode(closed, None, iterations=4)
    closed = cv2.dilate(closed, None, iterations=4)

    #find the contours in the thresholded image. then sort the contours by their area, keeping only the largest one
    img, cnts, hierarchy = cv2.findContours(closed.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    c = sorted(cnts, key=cv2.contourArea, reverse=True)[0]

    #compute the rotated bounding box of the largest contour
    rect = cv2.minAreaRect(c)
    box = np.int0(cv2.boxPoints(rect))

    #draw a bounding box around the detected barcode and display the image
    cv2.drawContours(image, [box], -1, (0, 255, 0), 3)
    cv2.imshow("Barcode detection", image)
    cv2.waitKey(0)
    return


# determine the location of the beacon with respect to the drones current location
#calculate the distance vector and send appropriate information to the drones controller
def distance_to_camera(image, knownWidth, focalLength, marker):
    perWidth = 2 * marker.r;

    #calculate the distance using triangle similarity distance calculation
    distance_in_inches = (knownWidth * focalLength) / perWidth

    #calculate the angle between the camera and the object in both the diagonal and horizontal
    #http://stackoverflow.com/questions/17499409/opencv-calculate-angle-between-camera-and-pixel
    #pi camera stats: sensor resolution 2592x1944 with horizontal FOV 53.50 degree Vertical FOV 41.41 degrees
    rotationX = 0
    rotationY = 0
    print('Distance in inches {0}'.format(distance_in_inches))
    return [distance_in_inches, rotationX, rotationY]

