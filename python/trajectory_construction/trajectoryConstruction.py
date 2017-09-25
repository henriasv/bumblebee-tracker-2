
import os
import sys
print(os.environ['PYTHONPATH'])
from BumblebeeExperiment import BumblebeeExperiment
import ujson as json
import pickle
import matplotlib.pyplot as plt
import matplotlib.animation as manimation
import numpy as np
import pandas
import trackpy
import trackpy.predict as predict
import imageio
import functools
from matplotlib import colors as mcolors
import palettable

#from multiprocessing import Pool

#ifilename = "/Users/henriksveinsson/humlevideo_production/2017-03-09/traj20170309.json"
# 




# frameKeypoints = np.asarray(frameKeypoints)
# with open('/Users/henriksveinsson/sandbox/testPickle.pickle', 'wb') as ofile:
#      pickle.dump(frameKeypoints, ofile)

with open('/Users/henriksveinsson/sandbox/testPickle.pickle', 'rb') as ifile:
     frameKeypoints = pickle.load(ifile)


#sys.exit(0)
colors = []
for key, value in mcolors.XKCD_COLORS.items():
    colors.append(value)
colors = ['b', 'g', 'r', 'c', 'm', 'k']
colors = list(palettable.tableau.Tableau_10.mpl_colors)

videoFile = "/Users/henriksveinsson/humlevideo_production/2017-03-09/HERO5BlackA1/concatGOPR0029.mp4"
video = imageio.get_reader(videoFile, "ffmpeg")

def dataframeFromKeypoints(keypoints, timestep):
    if len(keypoints>0):
        return pandas.DataFrame(dict(x=keypoints[:,0], y=keypoints[:,1], frame = timestep))
    else:
        return None

def getFrameDataframe(frameKeypoints, start=1, stop=10):
    frameList = []
    for i in range(start,stop):
        frame = dataframeFromKeypoints(frameKeypoints[i], i)
        if not frame is None:
            frameList.append(frame)
    frameList = pandas.concat(frameList)
    return frameList    


def calculateTrajctoryArcLength(particle):
    x = np.asarray(particle['x'])
    y = np.asarray(particle['y'])
    frame = np.asarray(particle['frame'])
    xdiff = np.diff(x)
    ydiff = np.diff(y)
    tdiff = np.diff
    increments = np.sqrt(xdiff*xdiff+ydiff*ydiff)/np.diff(frame)
    arclength = np.sum(increments)
    return arclength, increments

def plotTrajectoryRange(tr, frameMin, frameMax):
    plt.ion()
    trSubset = tr.loc[tr['frame'] < frameMax]
    trSubset = trSubset.loc[trSubset['frame'] > frameMin]
    if not trSubset.empty:
        ax = trackpy.plot_traj(trSubset,colorby='particle', ax=plt.gca(), label=False)


def plotCurrentTrajectores(tr, frame):
    trSubset = tr.loc[tr['frame'] == frame]
    particles = trSubset['particle'].tolist()
    trSubset = tr[tr.particle.isin(particles) & (tr.frame < frame)]
    if not trSubset.empty:
        print(trSubset.groupby('particle'))
        for name, group in trSubset.groupby('particle'):
            particleIndex = int(list(group['particle'])[0])
            print(particleIndex)
            #subset = pandas.DataFrame(subset)
            plt.plot(group['y'], group['x'], color=colors[int(particleIndex%len(colors))], alpha=0.6, linewidth=2)

def trshow(tr, first_style='bo', last_style='gs', style='b.'):
    frames = list(tr.groupby('frame'))
    nframes = len(frames)
    # 3
    trackpy.plot_traj(tr, colorby='particle', ax=plt.gca(), label=True, plot_style={"style" : "--"})
    plt.axis('equal');
    plt.xlabel('x')
    plt.ylabel('y')

def plotTrajectoryOnVideo(tr, video, startFrame, stopFrame, writer=None, savefig=False):
    #if writer is None:
    #    plt.ion()
    for frame in range(startFrame, stopFrame):
        print("Plotting frame: ", frame)
        plt.cla()
        frameArray = np.asarray(video.get_data(frame+631))[::-1, ::-1]
        frameArray = np.transpose(frameArray, (0, 1, 2))
        plt.imshow(frameArray)
        #plotTrajectoryRange(tr, startFrame, frame)
        plotCurrentTrajectores(tr, frame)
        #if writer is None:
        #    plt.pause(0.05)
        #else:
        #    writer.grab_frame()
        
        #plt.show()
        if savefig == True:
            plt.xlabel("x (pixels)")
            plt.ylabel("y (pixels)")
            plt.savefig("trajectories.pdf", dpi=300)

def constructTrajectory(filename):
    print(filename)
    ofilename = filename.replace(".json", "_trajectoriesB.csv")
    if (os.path.isfile(ofilename)): 
        fileSize = os.stat(ofilename).st_size
        if fileSize>1e6:
            print(ofilename, 'already analysed')
            return
    else:
        print("output csv does not yet exist")

    myExperiment = BumblebeeExperiment(filename);
    frameKeypoints = []
    for element in myExperiment.camBTracking:
        frameKeypoints.append(np.asarray(element['Keypoints']))

    try: 
        frameList = getFrameDataframe(frameKeypoints)
        pred = trackpy.predict.NearestVelocityPredict(span=2)
        tr = pred.link_df(frameList, 40, memory=10)
        tr.to_csv(ofilename)
    except ValueError as e:
        print(e)


# p = Pool(4)


# fileList = []
# basePath = os.path.join(os.environ['HOME'], 'humlevideo_production')
# for folder in os.listdir(basePath):
#     if "2017" in folder:
#         for file in os.listdir(os.path.join(basePath, folder)):
#             if "json" in file:
#                 print(file)
#                 ifilename = os.path.join(basePath, folder, file);
#                 fileList.append(ifilename)
#                 # myExperiment = BumblebeeExperiment(ifilename);
#                 # frameKeypoints = []
#                 # for element in myExperiment.camATracking:
#                 #     frameKeypoints.append(np.asarray(element['Keypoints']))

#                 # try: 
#                 #     frameList = getFrameDataframe(frameKeypoints)
#                 #     pred = trackpy.predict.NearestVelocityPredict(span=2)
#                 #     tr = pred.link_df(frameList, 40, memory=10)
#                 #     tr.to_csv(os.path.join(basePath, folder, file.replace(".json", "_trajectories.csv")))
#                 # except ValueError as e:
#                 #     print(e)

# p.map(constructTrajectory, fileList)

print(frameKeypoints[0:19])
start = 10000
stop  = 11000
frameList = getFrameDataframe(frameKeypoints, start=start, stop=stop)
print("got frames")
pred = trackpy.predict.NearestVelocityPredict(span=2)
tr = pred.link_df(frameList, 40, memory=10)

#FFMpegWriter = manimation.writers['ffmpeg']
#metadata = dict(title='Movie Test', artist='Matplotlib',
#                comment='Movie support!')
#writer = FFMpegWriter(fps=15, metadata=metadata)

#writer = None
#fig = plt.figure()
#with writer.saving(fig, "test.mp4", 200): 
plotTrajectoryOnVideo(tr, video, 10999, 11000, writer=None, savefig=True)


# def fitFunc(x, A, B, a, b):
#     return A*x*x/(a*a*a)*np.exp(-x*x/(2*a*a)) + B*x*x/(b*b*b)*np.exp(-x*x/(2*b*b))

# incrementsAll = []

# for particle in tr.groupby('particle'):
#     arclength, increments = calculateTrajctoryArcLength(particle[1])
#     incrementsAll.extend(increments)
# print(incrementsAll)

# plt.plot(np.sort(incrementsAll), np.arange(len(incrementsAll))/len(incrementsAll))
# plt.show()

# plt.hist(incrementsAll, bins=100)
# plt.show()
# plt.figure()
# plt.hist(incrementsAll, bins=1000)
# plt.show()

#plotTrajectoryRange(tr, 10500, 11000)
#trshow(tr)
#plt.show()

#emsd = trackpy.emsd(tr, mpp=100, fps=60, max_lagtime=1000)
#plt.figure()
#plt.plot(emsd)
#plt.show()
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

