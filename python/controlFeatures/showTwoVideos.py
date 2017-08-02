import sys	
import cv2
import matplotlib.pyplot as plt

def readFrame(cap, frameNum=None):
	if frameNum is not None:
		cap.set(cv2.CAP_PROP_POS_FRAMES, frameNum)
	#else:
	#	pass
	ret, frame = cap.read()
	#frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
	return frame



capA = cv2.VideoCapture(sys.argv[1])
#capB = cv2.VideoCapture(sys.argv[2])

frame1A = readFrame(capA, 1)
#frame1B = readFrame(capB)
frame1A = cv2.flip(frame1A, -1)
hfig = plt.figure()
p1 = plt.subplot(211)
#p2 = plt.subplot(212)
axframe = plt.axes([0.1, 0.05, 0.8, 0.03], facecolor="b")

#cv2.imshow("frame", frame1A)
#cv2.waitKey(0)

frameNum = int(sys.argv[2])
maxFrame = int(sys.argv[3])
while frameNum<maxFrame:
	cv2.imshow("frame2", readFrame(capA, frameNum))
	cv2.waitKey(1)
	frameNum += 1
#p1.imshow(frame1A)
#p2.imshow(frame1B)


#plt.show()

#def update(val):
	#capA.
