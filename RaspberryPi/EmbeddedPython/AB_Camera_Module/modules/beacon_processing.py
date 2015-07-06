__author__ = 'Pravjot'

import numpy as np
import cv2
import cv2.cv as cv
from classes import beacon
import math


AB_beaconList = beacon.AB_beacons()

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
        return False
    else:
        AB_beaconList.next_beacon()
        #UART command to send information to STMF board
        next_beacon = AB_beaconList.beacon_info(AB_beaconList.currentID)
        print("Next beacon information: {0}".format(next_beacon))
    return True

# search the image for the beacon, assuming that we use some kind of RED source
def locate_beacon(image):
    img = cv2.medianBlur(image, 5)
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    #cv2.imwrite("hsv.jpg", hsv)
    # HSV properties
    lower = [150, 90, 90]
    upper = [190, 255, 255]

    # create numpy arrays from the bounadires
    lower = np.array(lower, dtype="uint8")
    upper = np.array(upper, dtype="uint8")

    #filter out all unnecessary colors
    mask = cv2.inRange(hsv, lower, upper)
    #HoughCircles likes ring like circles to filled ones
    edge = cv2.Canny(mask, 100, 200)
    #smooth the image by applying gaussian blur
    blurr = cv2.GaussianBlur(edge, (9, 9), 2)

    #cv2.imwrite("edge.jpg", edge)
    #cv2.imwrite("mask.jpg", mask)
    #cv2.imwrite("blurr.jpg", blurr)

    circles = cv2.HoughCircles(blurr, cv.CV_HOUGH_GRADIENT, 1.2, 150,
                               param1=20, param2=80, minRadius=0, maxRadius=0)
    if circles is not None:
        circles = np.uint16(np.around(circles[0, :]))
        for (x, y, r) in circles:
            # draw the outer circle
            cv2.circle(img, (x, y), r, (0, 255, 0), 2)
            marker = beacon.marker(x, y, r)
            print('MARKER LOCATION X: {0}, Y: {1}, R: {2} in pixels'.format(marker.x, marker.y, marker.r))
            # draw the center of the circle
            cv2.circle(img, (x, y), 2, (0, 255, 0), 3)
            #cv2.imwrite("img_circled.jpg", img)
    else:
        marker = None

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
    return


# determine the location of the beacon with respect to the drones current location
#calculate the distance vector and send appropriate information to the drones controller
def distance_to_camera(image, cam, marker):
    perWidth = 2*marker.r;

    #calculate the distance using triangle similarity distance calculation
    distance = (marker.KNOWN_WIDTH * cam.FOCAL_LENGTH) / perWidth

    #calculate the angle between the camera and the object in both the diagonal and horizontal
    #http://stackoverflow.com/questions/17499409/opencv-calculate-angle-between-camera-and-pixel
    #pi camera stats: sensor resolution 2592x1944 with horizontal FOV 53.50 degree Vertical FOV 41.41 degrees

    #compute the number of pixels along the diagonal
    pixels_diag = math.sqrt(pow(cam.RESOLUTION[0], 2)+pow(cam.RESOLUTION[1], 2))
    degrees_per_pixel_diag = cam.HORIZONTAL_FOV / pixels_diag
    x_angle = (marker.x * degrees_per_pixel_diag)
    X = distance*math.cos(x_angle)
    Y = distance*math.sin(x_angle)

    print('Distance in cm: {0}, Angle: {1} ({2},{3})'.format(distance, x_angle, X, Y))

    norm_vec = math.sqrt(math.pow(X,2)+math.pow(Y,2))
    print norm_vec
    X_norm = X / norm_vec
    Y_norm = Y / norm_vec
    print('NORMALIZED: ({0},{1})'.format(X_norm, Y_norm))
    return [distance, X, Y]

def movingAverage(x, N):
    return np.convolve(x,np.ones((N, ))/N, mode='valid')[(N-1): ]

def regAverage(x):
    return reduce(lambda i, j: i+j, x)/len(x)