__author__ = 'Pravjot'

import math

import numpy as np
import cv2

import cv2.cv as cv
from AB_Camera_Classes import beacon
from AB_Camera_Modules import nav_board_comm as Nav_Board_Comm
from AB_Logging import ab_log as AB_Log


AB_beaconList = beacon.AB_beacons()
collision_distance = 0

cam_logger = AB_Log.get_logger('AB_CAMERA_MODULE')
#send the next beacons location to the flight controller
def send_next_beacon_info():
    global cam_logger
    currentB = AB_beaconList.currentID
    nextB = AB_beaconList.nextID
    if currentB == nextB:
        #No more beacon left to navigate to
        cam_logger.info("no more beacons left to navigate to")
        return False
    else:
        AB_beaconList.next_beacon()

        next_beacon = AB_beaconList.beacon_info(AB_beaconList.currentID)

        Nav_Board_Comm.send_and_wait(next_beacon, Nav_Board_Comm.ABFlags.BEACON_ROTATION)
    return True

# search the image for the beacon, assuming that we use some kind of RED source
def locate_beacon(image):
    img = cv2.medianBlur(image, 5)
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    #cam_logger.debug("writing debug image debug/hsv.jpg")
    #cv2.imwrite("debug/hsv.jpg", hsv)
    # define the list of boundaries
    boundaries = [
        ([100, 100, 50], [180, 255, 255])
    ]

    for (lower, upper) in boundaries:
        # create numpy arrays from the bounadires
        lower = np.array(lower, dtype="uint8")
        upper = np.array(upper, dtype="uint8")

        #filter out all unnecessary colors
        mask = cv2.inRange(hsv, lower, upper)
        #cv2.imwrite("debug/mask.jpg", mask)
        #cam_logger.debug("writing debug image debug/mask.jpg")
        #HoughCircles likes ring like circles to filled ones
        edge = cv2.Canny(mask, 100, 300)
        #smooth the image by applying gaussian blur
        blurr = cv2.GaussianBlur(edge, (9, 9), 2)
        #cv2.imwrite("debug/edge.jpg", edge)
        #cam_logger.debug("writing debug image debug/edge.jpg")

        circles = cv2.HoughCircles(blurr, cv.CV_HOUGH_GRADIENT, 1.2, 200,
                                   param1=20, param2=80, minRadius=10, maxRadius=0)
        if circles is not None:
            circles = sorted(circles[0], key=lambda x:x[2], reverse=True)

        if circles is not None:
            circles = np.uint16(np.around(circles))
            for (x, y, r) in circles:
                # draw the outer circle
                cv2.circle(img, (x, y), r, (0, 255, 0), 2)
                marker = beacon.marker(x, y, r)

                # draw the center of the circle
                cv2.circle(img, (x, y), 2, (0, 255, 0), 3)
                cv2.imshow("debug/img_circled.jpg", img)
                cv2.waitKey(100)
                #cam_logger.debug("writing debug image debug/img_circled.jpg")
                break
        else:
            marker = None

    return marker

# determine the location of the beacon with respect to the drones current location
#calculate the distance vector and send appropriate information to the drones controller
def distance_to_camera(focalLength, resolution,hFOV, marker, image):
    perWidth = 2*marker.r;

    #calculate the distance using triangle similarity distance calculation
    distance = (marker.KNOWN_WIDTH * focalLength) / perWidth

    #get the center of the image

    row,col = image.shape[0:2]
    image_center = tuple(np.array([row,col])/2)


    #compute the number of pixels along the diagonal
   # pixels_diag = math.sqrt(pow(resolution[0], 2)+pow(resolution[1], 2))
    #degrees_per_pixel_diag = hFOV / pixels_diag
    #x_angle = (marker.x * degrees_per_pixel_diag)

    #X = pixel_to_cm_factor*distance*math.cos(x_angle)
    #Y = pixel_to_cm_factor *distance*math.sin(x_angle)
    print image_center
    X = (image_center[0] - marker.x)* marker.KNOWN_WIDTH/perWidth;
    Y = (image_center[1] - marker.y)*marker.KNOWN_WIDTH/perWidth;

    Z = distance

    return beacon.beaconLocation(X,Y,Z)

def detect_frontal_object(image):
    imageSize = 0

    return imageSize

def frontal_collision(image, focalLength):
    #do some processing
    distance = 0

    #find the image size of the object using bounding box
    imageSize = detect_frontal_object(image)

    #size of the object is s = (imgSize/FOCAL_LENGTH )* distance_to_object
    objectSize = (imageSize*focalLength) / collision_distance
    Nav_Board_Comm.send_and_wait(distance, Nav_Board_Comm.ABFlags.AVOID_FRONT)
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