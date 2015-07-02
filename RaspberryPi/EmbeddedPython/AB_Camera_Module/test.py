import cv2
import numpy as np

img = cv2.imread('circles_rectangles_noise.jpg')
cv2.imshow("original", img)
cv2.waitKey(0)
img = cv2.medianBlur(img,5)
hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)

cv2.imshow('hsv', hsv)
cv2.waitKey(0)

#HSV properties
lower = [160,100,100]
upper = [179,255,255]

#create numpy arrays from the bounadires
lower = np.array(lower, dtype="uint8")
upper = np.array(upper, dtype="uint8")
mask = cv2.inRange(hsv, lower, upper)

cv2.imshow("color detect", mask)
cv2.waitKey(0)
#smooth the image by applying gaussian blur
blurr = cv2.GaussianBlur(mask, (9,9),2)
cv2.imshow("blurr", blurr)
cv2.waitKey(0)

#img = cv2.medianBlur(img,5)
#[h, s, v] = cv2.split(blurr)
#cimg = cv2.cvtColor(mask,cv2.)

circles = cv2.HoughCircles(blurr,cv2.HOUGH_GRADIENT,1,20,
                            param1=50,param2=30,minRadius=0,maxRadius=0)

circles = np.uint16(np.around(circles[0, :]))

for (x, y, r) in circles:
    # draw the outer circle
    cv2.circle(img,(x,y),r,(0,255,0),2)
    print(x)
    print(y)
    print(r)
    # draw the center of the circle
    cv2.circle(img,(x,y),2,(0,255,0),3)
cv2 .imshow('output.jpg', img)
cv2.waitKey(0)