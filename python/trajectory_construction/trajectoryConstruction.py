
from BumblebeeExperiment import BumblebeeExperiment
import ujson as json
import pickle
import matplotlib.pyplot as plt
import numpy as np

# ifilename = "/Users/henriksveinsson/humlevideo_production/2017-03-09/traj20170309.json"
# myExperiment = BumblebeeExperiment(ifilename);


# frameKeypoints = []
# for element in myExperiment.camATracking:
#      frameKeypoints.append(np.asarray(element['Keypoints']))

# frameKeypoints = np.asarray(frameKeypoints)
# with open('/Users/henriksveinsson/sandbox/testPickle.pickle', 'wb') as ofile:
#      pickle.dump(frameKeypoints, ofile)

with open('/Users/henriksveinsson/sandbox/testPickle.pickle', 'rb') as ifile:
    frameKeypoints = pickle.load(ifile)


import numpy as np
import pandas
import trackpy
import trackpy.predict as predict
import imageio



videoFile = "/Users/henriksveinsson/humlevideo_production/2017-03-09/HERO5BlackA1/concatGOPR0029.mp4"
video = imageio.get_reader(videoFile, "ffmpeg")

def dataframeFromKeypoints(keypoints, timestep):
    if len(keypoints>0):
        return pandas.DataFrame(dict(x=keypoints[:,0], y=keypoints[:,1], frame = timestep))
    else:
        return None

frameList = []
for i in range(10000, 11000):
    frame = dataframeFromKeypoints(frameKeypoints[i], i)
    if not frame is None:
        frameList.append(frame)

frameList = pandas.concat(frameList)

pred = trackpy.predict.NearestVelocityPredict(span=1)
tr = pred.link_df(frameList, 40, memory=5)
#tr = trackpy.link_df(frameList, 100, memory=10, predictor=predict.predictor(predict.NearestVelocityPredict()))
for particle in tr.groupby('particle'):
    print(particle)


def plotTrajectoryRange(tr, frameMin, frameMax):
    plt.ion()
    trSubset = tr.loc[tr['frame'] < frameMax]
    trSubset = trSubset.loc[trSubset['frame'] > frameMin]
    if not trSubset.empty:
        trackpy.plot_traj(trSubset,colorby='particle', ax=plt.gca(), label=True)

def trshow(tr, first_style='bo', last_style='gs', style='b.'):
    frames = list(tr.groupby('frame'))
    nframes = len(frames)
    # 3
    trackpy.plot_traj(tr, colorby='particle', ax=plt.gca(), label=True, plot_style={"style" : "--"})
    plt.axis('equal');
    plt.xlabel('x')
    plt.ylabel('y')

def plotTrajectoryOnVideo(tr, video, startFrame, stopFrame):
    plt.ion()
    for frame in range(startFrame, stopFrame):
        plt.cla()
        frameArray = np.asarray(video.get_data(frame+631))[::-1, ::-1]
        frameArray = np.transpose(frameArray, (1, 0, 2))
        plt.imshow(frameArray)
        plotTrajectoryRange(tr, startFrame, frame)
        plt.pause(0.05)

plotTrajectoryOnVideo(tr, video, 10000, 11000)
#plotUpToFrame(tr, 500, 1000)
#trshow(tr)
plt.show()

emsd = trackpy.emsd(tr, mpp=100, fps=60, max_lagtime=1000)
plt.figure()
plt.plot(emsd)
plt.show()
# print(frameKeypoints)
# print(frameKeypoints[10000])

# plt.ion()
# plt.figure(1)
# for i in range(10000, len(frameKeypoints)):    
#     plt.cla()
#     plt.plot(frameKeypoints[i][:,0], frameKeypoints[i][:,1], 'o')
#     plt.xlim((0, 1500))
#     plt.ylim((0, 2560))
#     plt.pause(0.05)
# plt.show()

