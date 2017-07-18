import sys
import cv2
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("videoFile", type=str)
parser.add_argument("outFile", type=str)
parser.add_argument("--flip", action="store_true")
args = parser.parse_args()

print args.videoFile
cap = cv2.VideoCapture(args.videoFile)
ret, frame = cap.read()
frame = cv2.transpose(frame)
if args.flip:
	frame = cv2.flip(frame, -1)
cv2.imwrite(args.outFile, frame)