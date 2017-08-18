import json
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd


class RunningMean:
    def __init__(self,N):
        self.N = N

    def __call__(self, npArray):
        N = self.N
        retArray = np.zeros(npArray.size)*np.nan
        padL = int(N/2)
        padR = N-padL-1
        retArray[padL:-padR] = np.convolve(npArray, np.ones((N,))/N, mode='valid')
        return retArray

def identifyLandings(array, threshold):
    onFlower = array>threshold
    changes = np.append(np.where(onFlower[:-1] != onFlower[1:])[0], len(onFlower))
    durations = changes[1:]-changes[:-1]
    times = changes[:-1]
    print(onFlower)
    return times[0::2], durations[0::2]

class BumblebeeExperiment:
    def __init__(self, parameterFileName):
        with open(parameterFileName) as ifile:
            self.tracking = json.load(ifile)
        self.camATracking = self.tracking[0::2]
        self.camBTracking = self.tracking[1::2]

    def sumOfBeesOnIndividualFlower(self):
        landingData = {"tid" : [], "varighet" : [], "farge": [], "blomst":[]}
        flowerDataA = []
        flowerDataB = []
        flowerColors = self.camATracking[0]["flowerColors"]
        for stepA, stepB in zip(self.camATracking, self.camBTracking):
            flowerDataA.append(stepA['FlowerBeeStatus'])
            flowerDataB.append(stepB['FlowerBeeStatus'])


        flowerDataA = np.asarray(flowerDataA)
        flowerDataB = np.asarray(flowerDataB)
        
        flowerDataAll = np.logical_and(flowerDataA, flowerDataB);
        numFlowers = flowerDataAll.shape[1]
        print(flowerDataAll.shape)
        f, axarr = plt.subplots(int(np.ceil(numFlowers/2)), 2)
        myfig = plt.figure()
        myScatter = plt.axes()
        myfig2 = plt.figure()
        myBar  = plt.axes()
        myRunningMean = RunningMean(120)
        for i in range(numFlowers):
            plotColor = "b" if flowerColors[i] == "blue" else "y"
            smoothedData = myRunningMean(flowerDataAll[:,i])
            #axarr[int(np.floor(i/2)), i%2].plot(identifyLandings(smoothedData, 0.5), c="k")
            axarr[int(np.floor(i/2)), i%2].plot(smoothedData, c=plotColor)
            axarr[int(np.floor(i/2)), i%2].set_ylim([-0.1, 1.1])
            times, durations = identifyLandings(smoothedData, 0.5) 
            myScatter.plot(times, durations, '--o', c=plotColor)
            if len(times)>0 and len(durations) > 0:
                print("times: ", len(times), " durations", len(durations))
                for time, duration in zip(times, durations):
                    landingData["tid"].append(time); landingData["varighet"].append(duration)
                    landingData["farge"].append(flowerColors[i]); landingData["blomst"].append(i)
                    myBar.barh(i, duration, left=time, height=1, align='center', color=plotColor, alpha = 1.0)
                    if duration < 1000:
                        myBar.barh(i, 1000, left=time, height=1, align='center', color=plotColor, alpha = 0.5)
            myBar.set_xlim([0, len(smoothedData)])
        dataFrame = pd.DataFrame.from_dict(landingData)
        dataFrame = dataFrame.sort_values("tid")
        dataFrame = dataFrame.reset_index()
        del dataFrame["index"]
        self.landingData = dataFrame
        
        print(dataFrame)

    def sumOfBeesOnFlower(self):
        flowerDataA = []
        flowerDataB = []
        for stepA, stepB in zip(self.camATracking, self.camBTracking):
            flowerDataA.append(stepA['FlowerBeeStatus'])
            flowerDataB.append(stepB['FlowerBeeStatus'])

        flowerDataA = np.asarray(flowerDataA)
        flowerDataB = np.asarray(flowerDataB)
        
        flowerDataAll = np.logical_and(flowerDataA, flowerDataB);
        flowerDataSummed = np.sum(flowerDataAll, axis=1)
        myRunningMean = RunningMean(120)
        plt.plot(myRunningMean(flowerDataSummed))
        #plt.plot(flowerDataASummed)
        #plt.plot(flowerDataBSummed)
        
        plt.show()
        print(np.dtype(flowerDataSummed))


if __name__=="__main__":
    import sys
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("infile", type=str)
    parser.add_argument("--csv", type=str)
    parser.add_argument("--plot", action="store_true")
    args = parser.parse_args()

    experiment = BumblebeeExperiment(args.infile)
    experiment.sumOfBeesOnIndividualFlower()
    if (args.csv):
        experiment.landingData.to_csv(args.csv, sep=";", index_label="landing")
    if (args.plot):
        plt.show()