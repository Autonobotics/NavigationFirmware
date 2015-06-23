__author__ = 'Pravjot'

import numpy as np
import cv2

# search the image for the beacon, assuming that we use some kind of RED source
def locate_beacon(image):
    #define the list of boundaries in RGB (BGR cause OpenCV represents images as NumPy arrays in reverse)
    #Each entry represents a a list of tuples with two values: Lower limits and upper limits
    #RED, BLUE, YELLOW, GRAY
    boundaries = [
        ([86, 31, 4], [220, 88, 50])
    ]

    for (lower, upper) in boundaries:
        #create numpy arrays from the bounadires
        lower = np.array(lower, dtype="uint8")
        upper = np.array(upper, dtype="uint8")

        #find the colors within the specified boundaries and apply the mask
        mask = cv2.inRange(image, lower, upper)
        output = cv2.bitwise_and(image, image, mask=mask)

        # convert the image to grayscale, blur it
        gray = cv2.cvtColor(output, cv2.COLOR_BGR2GRAY)
        gray = cv2.GaussianBlur(gray, (5, 5), 0)

        # find the contours in the edged image and keep the largest one;
        # we'll assume that this is our piece of paper in the image
        (_,cnts, _) = cv2.findContours(gray.copy(), cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)
        contours = sorted(cnts, key=cv2.contourArea, reverse=True)[:5]

        for c in contours:
            #approximate the contour
            peri = cv2.arcLength(c, True)
            approx = cv2.approxPolyDP(c, 0.02*peri, True)

            #if our approximated contour has four points, then we assume we found our beacon
            if len(approx) == 4:
                beacon = approx
                break
            #if beacon is not continue? 360 spin to locate? shutdown?
            ## go towards the biggest contour which might be the beacon?
            else:
                #The beacon is the largest contour?
                #beacon = contours[0]
                return False
        #(x,y) coordinates and width and height of the box
        rect = cv2.minAreaRect(beacon)

    return rect

#locate the information stored in the beacon (barcode), return the subimage containing the barcode information
def locate_beacon_information(image):
    #load the image and convert it to grayscale
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

#determine the location of the beacon with respect to the drones current location
#calculate the distance vector and send appropriate information to the drones controller
def distance_to_camera(image,knownWidth,focalLength, marker):
    perWidth = marker[1][0]

    #F = (pixel width * distance from object)/ width of object
    #NOTE: this is a very simple calibration! better to use intrinsic parameters of the camera to calibrate
    #and obtain a better estimation of the focal length of the camera
    #NOTE: documentation of the Pi camera says the focal length = 3.60mm -> 13.606299213 pixels
    #focalLength = (perWidth*knownDistance)/knownWidth

    #calculate the distance using triangle similarity distance calculation
    distance_in_inches = (knownWidth*focalLength)/perWidth

    #draw a bounding box around the image and siplay it
    box = np.int0(cv2.boxPoints(marker))
    cv2.drawContours(image, [box], -1, (0,255,0),2)
    cv2.putText(image, "%.2fin" %(distance_in_inches), (image.shape[1]-200, image.shape[0]-20), cv2.FONT_HERSHEY_SIMPLEX,
                2.0,(0,255,0),3)
    cv2.imshow("image", image)
    cv2.waitKey(0)

    #calculate the angle between the camera and the object in both the diagonal and horizontal
    #http://stackoverflow.com/questions/17499409/opencv-calculate-angle-between-camera-and-pixel
    #pi camera stats: sensor resolution 2592x1944 with horizontal FOV 53.50 degree Vertical FOV 41.41 degrees
    rotationX = 0
    rotationY = 0

    return [distance_in_inches, rotationX, rotationY]

#send the next beacons location to the flight controller
def send_next_beacon_info(beaconId):
    return