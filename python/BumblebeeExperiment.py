import json
import numpy as np
import matplotlib.pyplot as plt


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

class BumblebeeExperiment:
    def __init__(self, parameterFileName):
        with open(parameterFileName) as ifile:
            self.tracking = json.load(ifile)
        #with open(self.parameters["TrackingFile"]) as ifile:
        #    self.tracking = json.load(ifile)
        #print(self.tracking)
        self.camATracking = self.tracking[0::2]
        self.camBTracking = self.tracking[1::2]

    def sumOfBeesOnFlower(self):
        flowerDataA = []
        flowerDataB = []
        for stepA, stepB in zip(self.camATracking, self.camBTracking):
            flowerDataA.append(stepA['FlowerBeeStatus'])
            flowerDataB.append(stepB['FlowerBeeStatus'])



        flowerDataA = np.asarray(flowerDataA)
        #flowerDataASummed = np.sum(flowerDataA, axis=1)
        flowerDataB = np.asarray(flowerDataB)
        #flowerDataBSummed = np.sum(flowerDataB, axis=1)
        
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
    experiment = BumblebeeExperiment(sys.argv[1])
    experiment.sumOfBeesOnFlower()
