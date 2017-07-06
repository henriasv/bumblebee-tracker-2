import cv2 
import numpy as np

def drawlines(img1,img2,lines,pts1,pts2):
    ''' img1 - image on which we draw the epilines for the points in img2
        lines - corresponding epilines '''
    dummy,c,dim = img1.shape
    #img1 = cv2.cvtColor(img1,cv2.COLOR_GRAY2BGR)
    #img2 = cv2.cvtColor(img2,cv2.COLOR_GRAY2BGR)
    for r,pt1,pt2 in zip(lines,pts1,pts2):
        color = tuple(np.random.randint(0,255,3).tolist())
        x0,y0 = map(int, [0, -r[2]/r[1] ])
        x1,y1 = map(int, [c, -(r[2]+r[0]*c)/r[1] ])
        img1 = cv2.line(img1, (x0,y0), (x1,y1), color,1)
        img1 = cv2.circle(img1,tuple(pt1),5,color,-1)
        img2 = cv2.circle(img2,tuple(pt2),5,color,-1)
    return img1,img2
"""
vidA = cv2.VideoCapture("/home/henriasv/testvideo/GOPR0018.mp4")
vidB = cv2.VideoCapture("/home/henriasv/testvideo/GOPR0039.mp4")

ret1, frameA = vidA.read()
ret2, frameB = vidB.read()

frameA = cv2.transpose(frameA)
frameB = cv2.transpose(frameB)
frameA = cv2.flip(frameA, -1)
#frameB = cv2.flip(frameB, 0)
"""
frameA = cv2.imread("frameA.jpg")
frameB = cv2.imread("frameB.jpg")

# Pixel values
pixelsA = [[ 980., 1176.], [704, 630], [791, 2152], [716, 2060], [72, 2623], [74, 112], [178, 129], [1475, 550], [1471, 2123], [628, 158], [662, 295], [1121, 448], [94, 2204], [1135, 1981]]
pixelsB = [[1092., 1176.], [802, 664], [725, 2169], [819, 2081], [70, 2164], [63, 621], [121, 588], [1481, 112], [1458, 2621], [640, 287], [394, 500], [848, 351], [377, 1980], [1347, 2226]]
pixelsA = np.asarray(pixelsA)
pixelsB = np.asarray(pixelsB)
pixelsA = np.int32(pixelsA)
pixelsB = np.int32(pixelsB)
print pixelsA
print pixelsB

for pixelA, pixelB in zip(pixelsA, pixelsB):
	cv2.circle(frameA, (int(pixelA[0]), int(pixelA[1])), 5, (255, 0, 0), 3)
	cv2.circle(frameB, (int(pixelB[0]), int(pixelB[1])), 5, (255, 0, 0), 3)


fundamentalMat = cv2.findFundamentalMat(pixelsA, pixelsB, cv2.FM_8POINT)[0]
print fundamentalMat

epilines = cv2.computeCorrespondEpilines(pixelsB, 2, fundamentalMat)
epilines = epilines.reshape(-1, 3)
img1, dummy  = drawlines(frameA, frameB, epilines, pixelsA, pixelsB)

epilines = cv2.computeCorrespondEpilines(pixelsA, 1, fundamentalMat)
epilines = epilines.reshape(-1, 3)
img2, dummy  = drawlines(frameB, frameA, epilines, pixelsB, pixelsA)


cv2.imwrite("frameAWithIndications.jpg", frameA)
cv2.imwrite("frameBWithIndications.jpg", frameB)
cv2.imwrite("epilinesA.jpg", img1)
cv2.imwrite("epilinesB.jpg", img2)

#cv2.imshow("frameA", frame1)
#cv2.imshow("frameB", frame2)
#cv2.waitKey(0)
