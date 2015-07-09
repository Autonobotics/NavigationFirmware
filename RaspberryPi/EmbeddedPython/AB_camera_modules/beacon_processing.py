__author__ = 'Pravjot'

import math

import numpy as np
import cv2

import cv2.cv as cv
from AB_Camera_Classes import beacon
from AB_Camera_Modules import nav_board_comm as Nav_Board_Comm


AB_beaconList = beacon.AB_beacons()

def AB_startup():
    #send be intial beacon lcation to STM board
    startup_beacon_info = AB_beaconList.beacon_info(AB_beaconList.currentID)
    print("Initial Startup beacon information: {0}".format(startup_beacon_info))
    return

#send the next beacons location to the flight controller
def send_next_beacon_info():
    global cam_logger
    currentB = AB_beaconList.currentID
    nextB = AB_beaconList.nextID
    if currentB == nextB:
        #No more beacon left to navigate to
        cam_logger.debug('BEACON: No more beacons to navigate to')
        print("no more beacons left to navigate to")
        return False
    else:
        AB_beaconList.next_beacon()

        next_beacon = AB_beaconList.beacon_info(AB_beaconList.currentID)

        #Nav_Board_Comm.send_and_wait(next_beacon, Nav_Board_Comm.ABFlags.BEACON_ROTATION)

        print("Next beacon information: {0}".format(next_beacon))
    return True

# search the image for the beacon, assuming that we use some kind of RED source
def locate_beacon(image):
    img = cv2.medianBlur(image, 5)
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    cv2.imwrite("debug/hsv.jpg", hsv)
    # define the list of boundaries
    boundaries = [
        ([0, 10, 10], [10, 255, 255])
    ]

    for (lower, upper) in boundaries:
        # create numpy arrays from the bounadires
        lower = np.array(lower, dtype="uint8")
        upper = np.array(upper, dtype="uint8")

        #filter out all unnecessary colors
        mask = cv2.inRange(hsv, lower, upper)
        cv2.imwrite("debug/mask.jpg", mask)
        #HoughCircles likes ring like circles to filled ones
        edge = cv2.Canny(mask, 100, 200)
        #smooth the image by applying gaussian blur
        blurr = cv2.GaussianBlur(edge, (9, 9), 2)


        circles = cv2.HoughCircles(blurr, cv.CV_HOUGH_GRADIENT, 1.2, 150,
                                   param1=20, param2=80, minRadius=0, maxRadius=0)
        if circles is not None:
            circles = np.uint16(np.around(circles[0]))
            for (x, y, r) in circles:
                # draw the outer circle
                cv2.circle(img, (x, y), r, (0, 255, 0), 2)
                marker = beacon.marker(x, y, r)
                #print('MARKER LOCATION X: {0}, Y: {1}, R: {2} in pixels'.format(marker.x, marker.y, marker.r))
                # draw the center of the circle
                cv2.circle(img, (x, y), 2, (0, 255, 0), 3)
                cv2.imwrite("debug/img_circled.jpg", img)
                break
        else:
            marker = None

    return marker

# determine the location of the beacon with respect to the drones current location
#calculate the distance vector and send appropriate information to the drones controller
def distance_to_camera(focalLength, resolution,hFOV, marker):
    perWidth = 2*marker.r;

    #calculate the distance using triangle similarity distance calculation
    distance = (marker.KNOWN_WIDTH * focalLength) / perWidth

    #calculate the angle between the camera and the object in both the diagonal and horizontal
    #http://stackoverflow.com/questions/17499409/opencv-calculate-angle-between-camera-and-pixel
    #pi camera stats: sensor resolution 2592x1944 with horizontal FOV 53.50 degree Vertical FOV 41.41 degrees

    #compute the number of pixels along the diagonal
    pixels_diag = math.sqrt(pow(resolution[0], 2)+pow(resolution[1], 2))
    degrees_per_pixel_diag = hFOV / pixels_diag
    x_angle = (marker.x * degrees_per_pixel_diag)
    X = distance*math.cos(x_angle)
    Y = distance*math.sin(x_angle)

    norm_vec = math.sqrt(math.pow(X,2)+math.pow(Y,2))

    X_norm = X / norm_vec
    Y_norm = Y / norm_vec
    Z = distance

    return beacon.beaconLocation(X_norm,Y_norm,Z)

def frontal_collision(distance_to_object):
    #do some processing
    distance = 0
    Nav_Board_Comm.send_and_wait(distance, Nav_Board_Comm.ABFlags.FRONTAL_COLLISION)
    return

def beacon_filter(markerList):
    no_beacon_count = 0
    beacon_count =0
    tmp = []
    for i in markerList:
        if i.x == 0 and i.y == 0 and i.z == 0:
            no_beacon_count  = no_beacon_count + 1
        else:
            beacon_count = beacon_count + 1
            tmp.append(i)
    if beacon_count < no_beacon_count:
        marker = None
    else:
        marker = regAverage(tmp)
        print(beacon_count)
    return marker

def regAverage(x):
    tmpx = 0.0
    tmpy = 0.0
    tmpz = 0.0
    for i in x:
        tmpx = tmpx + i.x
        tmpy = tmpy + i.y
        tmpz = tmpz + i.z

    return beacon.beaconLocation(tmpx/len(x), tmpy/len(x), tmpz/len(x))