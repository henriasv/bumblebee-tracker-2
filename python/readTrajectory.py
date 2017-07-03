import json
import numpy as np
import matplotlib.pyplot as plt

import imageio

def readTrajectory(filename):
    allpointsx = []
    allpointsy = []
    with open(filename) as ifile:
        #print ifile.read()
        data = json.load(ifile)
        for entry in data:
            points = np.asarray(entry["Keypoints"])
            allpointsx.extend(points[:,0])
            allpointsy.extend(points[:,1])
        #    plt.scatter(points[:,0], points[:,1])
        #plt.show()
    return allpointsx, allpointsy

if __name__=="__main__":
    import numpy.ma as ma
    """
    numBees = []
    for entry in json.load(open("/private/tmp/test")):
        numBees.append(len(entry["Keypoints"]))
    plt.plot(numBees)
    plt.show()
    sys.exit(0)
    plt.ion()
    #readTrajectory("testFile.json")
    videoFile = "/Users/henriksveinsson/Dropbox/humlevideo/GP010017.MP4"
    vid = imageio.get_reader(videoFile, "ffmpeg")
    plt.figure()

    for entry in json.load(open("/private/tmp/test")):
        frame = entry["Frame"]
        plt.imshow(np.asarray(vid.get_data(frame+1)[::-1, ::-1]))
        points = np.asarray(entry["Keypoints"])
        plt.scatter(points[:,1], points[:,0], alpha=0.5)
        plt.pause(0.01)
        plt.gcf().clear()
        #plt.show()
    """
    """
    videoFile = "/Users/henriksveinsson/Dropbox/humlevideo/GP010017.MP4"
    vid = imageio.get_reader(videoFile, "ffmpeg")
    plt.ion()
    myobj = plt.imshow(np.asarray(vid.get_data(1)[::-1, ::-1]))
    plt.show()


    for entry in json.load(open("/private/tmp/test")):
        frame = entry["Frame"]
        if frame > 19975 and frame <20020:
            myobj.set_data(np.asarray(vid.get_data(frame+1)[::-1, ::-1]))
            points = np.asarray(entry["Keypoints"])
            plt.scatter(points[:,1], points[:,0], color=[0,0,1], alpha=0.05)
            plt.pause(0.001)
            plt.draw()
    plt.pause(0)
    """
    #for i in range(2, 100):
    #    data = np.asarray(vid.get_data(i)[::-1, ::-1])
    #    myobj.set_data(data)
    #    plt.draw()
    #    plt.pause(0.001)

    videoFile = "/Users/henriksveinsson/Dropbox/humlevideo/GP010017.MP4"
    vid = imageio.get_reader(videoFile, "ffmpeg")
    plt.imshow(np.asarray(vid.get_data(10000)[::-1, ::-1]))

    xdata, ydata = readTrajectory("/private/tmp/test")
    xdata = np.asarray(xdata)[100000:150000]
    ydata = np.asarray(ydata)[100000:150000]
    histogram = np.histogram2d(np.asarray(xdata), np.asarray(ydata), 500)
    data = histogram[0]
    print(data)
    print (data == 0)
    #data[data<=1] = np.nan
    data = np.log(data)
    datam = ma.masked_where(np.isinf(data),data)
    plt.pcolormesh(histogram[2], histogram[1], datam[:, :], alpha=0.5, cmap="ocean")
    #plt.colorbar()
    plt.xticks([])
    plt.yticks([])
    plt.show()

    #plt.scatter(xdata, ydata)

    #plt.show()
