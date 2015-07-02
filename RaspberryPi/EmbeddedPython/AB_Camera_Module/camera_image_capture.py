__author__ = 'Pravjot'

from picamera.array import PiRGBArray
from picamera import PiCamera
import time
import cv2
import pyimagesearch
import glob
import numpy as np


def get_PiImage():
    #intialize the camera and grab a reference to the raw camera capture
    camera = PiCamera()

    try:
        rawCapture = PiRGBArray(camera)
        #camera warmup time
        time.sleep(0.1)

        #grab an image from the camera
        camera.capture(rawCapture, format='bgr')
        image = rawCapture.array
        cv2.imwrite("output.jpg", image)

        #display the image on screen and wait for a keypress
        #cv2 .imshow("Raspberry Pi Image Capture", image)
        #cv2.waitKey(0)
    except ValueError:
        #log error
        print 'Could not capture image!'
        return False
    return True

#calibrate the camera using intrinsic parameters of the camera
#is it necessary for raspberry pi camera?
# refer to: http://opencv-python-tutroals.readthedocs.org/en/latest/py_tutorials/py_calib3d/py_calibration/py_calibration.html
def calibrate_camera():
    #termination criteria
    criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)

    #prepare object points ei (0,0,0), (1,0,0).....(6,5,0)
    objp = np.zeros((6*7,3), np.float32)
    objp[:,:2] = np.mgrid[0:7,0:6].T.reshape(-1,2)

    #Arrays to store objects points and image points from all the images
    objpoints = [] #3d point in real world space
    imgpoints = [] #2d points in image plane

    images = glob.glob('*jpg')

    for fname in images:
        img = cv2.imread(fname)
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

        #find the chess board corners
        ret, corners = cv2.findChessboardCorners(img, cv2.COLOR_BGR2GRAY)

        #if found, add object points, image points (after refining them)
        if ret == True:
            objpoints.append(objp)

            corners2 = cv2.cornerSubPix(gray, corners,(11,11), (-1,-1), criteria)

            #Draw and display the corners
            img = cv2.drawChessboardCorners(img, (7,6), corners2, ret)
            cv2.imshow('img', img)
            cv2.waitKey(500)
    cv2.destroyAllWindows()
    ret, mtx, dist, rvecs, tvecs = cv2.calibrateCamera(objpoints, imgpoints, gray.shape[::-1], None, None)

    return [ret, mtx, dist, rvecs, tvecs, tvecs]

def undistort_img(image, mtx, dist):
    h,w = image.shape[:2]
    newcameramtx, roi = cv2.getOptimalNewCameraMatrix(mtx, dist, (w,h), 1,(w,h))

    #undistort
    dst = cv2.undistort(image, mtx, dist, None, newcameramtx)

    #crop the image
    x,y,w,h = roi
    undistorted_img = dst[y:y+h, x:x+w]
    cv2.imshow('Undistorted image', undistorted_img)
    cv2.waitKey(0)

    return undistorted_img