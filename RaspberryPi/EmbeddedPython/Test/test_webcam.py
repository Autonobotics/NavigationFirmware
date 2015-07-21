__author__ = 'Pravjot'
#import camera_module

import cv2
import numpy as np

def locate_beacon(image):
    img = cv2.medianBlur(image, 5)
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
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
        cv2.imshow('masked', mask)
        cv2.waitKey(0)
        #HoughCircles likes ring like circles to filled ones
        edge = cv2.Canny(mask, 100, 300)

        cv2.imshow('edge', edge)
        cv2.waitKey(0)
        #smooth the image by applying gaussian blur
        blurr = cv2.GaussianBlur(edge, (9, 9), 2)


        circles = cv2.HoughCircles(blurr, cv2.HOUGH_GRADIENT, 1.2, 200,
                                   param1=20, param2=80, minRadius=10, maxRadius=0)
        if circles is not None:
            circles = sorted(circles[0], key=lambda x:x[2], reverse=True)

        if circles is not None:
            circles = np.uint16(np.around(circles))
            for (x, y, r) in circles:
                # draw the outer circle
                cv2.circle(img, (x, y), r, (0, 255, 0), 2)
                # draw the center of the circle
                cv2.circle(img, (x, y), 2, (0, 255, 0), 3)
                cv2.imshow("img_circled.jpg", img)
                cv2.waitKey(0)
                break

    return

def frontal(image):
    img = cv2.medianBlur(image, 5)
    #smooth the image by applying gaussian blur
    blurr = cv2.GaussianBlur(img, (9, 9), 2)
    edge = cv2.Canny(blurr, 100, 300)

    cv2.imshow('edge', edge)
    cv2.waitKey(0)



    return



if __name__ == "__main__":
    # Camera 0 is the integrated web cam on my netbook
    camera_port = 0

    #Number of frames to throw away while the camera adjusts to light levels
    ramp_frames = 30

    # Now we can initialize the camera capture object with the cv2.VideoCapture class.
    # All it needs is the index to a camera port.
    camera = cv2.VideoCapture(camera_port)

    # Captures a single image from the camera and returns it in PIL format
    def get_image():
     # read is the easiest way to get a full image out of a VideoCapture object.
     retval, im = camera.read()
     return im

    # Ramp the camera - these frames will be discarded and are only used to allow v4l2
    # to adjust light levels, if necessary
    for i in xrange(ramp_frames):
     temp = get_image()
    print("Taking image...")
    # Take the actual image we want to keep
    camera_capture = get_image()
    file = "test_image.png"
    # A nice feature of the imwrite method is that it will automatically choose the
    # correct format based on the file extension you provide. Convenient!
    cv2.imshow(file, camera_capture)
    cv2.waitKey(0)
    frontal(camera_capture)

    # You'll want to release the camera, otherwise you won't be able to create a new
    # capture object until your script exits
    del(camera)