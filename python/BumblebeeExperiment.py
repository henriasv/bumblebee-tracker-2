import json
import numpy as np
import matplotlib.pyplot as plt
class BumblebeeExperiment:
    def __init__(self, parameterFileName):
        with open(parameterFileName) as ifile:
            self.parameters = json.load(ifile)
        with open(self.parameters["TrackingFile"]) as ifile:
            self.tracking = json.load(ifile)
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
        flowerDataASummed = np.sum(flowerDataA, axis=1)
        flowerDataB = np.asarray(flowerDataB)
        flowerDataBSummed = np.sum(flowerDataB, axis=1)
        plt.plot(flowerDataASummed)
        plt.plot(flowerDataBSummed)
        plt.show()
        print(flowerDataASummed)


if __name__=="__main__":
    experiment = BumblebeeExperiment("testParameterFile.json")
    experiment.sumOfBeesOnFlower()
