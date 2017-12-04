import numpy as np
import matplotlib.pyplot as plt 
import os
import pandas
from multiprocessing import Pool

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



oversikt = pandas.DataFrame.from_csv('/Users/henriksveinsson/humlevideo_production/oversikt.csv', sep=";", index_col=None)
print(oversikt)

colors = {"0":"green", "1":"orange", "10":"red", "100":"black"}

def loadAndCalculate(filename):
	print(filename)
	
	filenameB = filename.replace("trajectories.csv", "trajectoriesB.csv")
	oversiktEntry = oversikt[oversikt["filnavn"]==os.path.basename(filename)]
	print(oversiktEntry)
	dose = int(oversiktEntry["dose"])
	dager_etter_levering = int(oversiktEntry["dager_etter_levering"])
	bol = int(oversiktEntry["bol"])


	#bol 	= 		oversikt[oversikt["filnavn"]==os.path.basename(oversiktFilename)]["bol"]
	fileSize = os.stat(filename).st_size
	if fileSize<1e6:
		return
	tr = pandas.read_csv(filename, engine='c')
	print("read csv file")

	incrementsAll = []
	for particle in tr.groupby('particle'):
		arclength, increments = calculateTrajctoryArcLength(particle[1])
		incrementsAll.extend(increments)
	
	trB = pandas.read_csv(filenameB, engine="c")
	for particle in trB.groupby('particle'):
		arclength, increments = calculateTrajctoryArcLength(particle[1])
		incrementsAll.extend(increments)

	incrementsAll = np.sort(incrementsAll)
	numVelocities = len(incrementsAll)
	numParticlesAll = []
	# for frame in tr.groupby('frame'):
	# 	numParticlesAll.append(len(frame[1]))
	# numParticlesAll = np.asarray(numParticlesAll)
	
	return {"dose":dose, "velocities":incrementsAll, "numParticles" : numParticlesAll, "bol":bol, "dager_etter_levering":dager_etter_levering, "numVelocities":numVelocities}

fig1 = plt.figure(1)
fig2 = plt.figure(2)

p = Pool(8)

fileList = []
basePath = os.path.join(os.environ['HOME'], 'humlevideo_production')
for folder in os.listdir(basePath):
    if "2017" in folder:
        for file in os.listdir(os.path.join(basePath, folder)):
        	if "trajectories.csv" in file:
        		fileList.append(os.path.join(basePath, folder, file))
print(fileList)

results = p.map(loadAndCalculate, fileList)


proportionDict = {"dose":[], "activityProportion":[], "bol":[], "dager_etter_levering":[], "numVelocities":[]}



for element in results:
	if not element is None:
		plt.figure(1)
		plt.subplot(1, 2, 2)
		increments = element["velocities"]
		dose = element["dose"]
		bol = element["bol"]
		dager_etter_levering = element["dager_etter_levering"]
		plt.plot(increments[::100], (np.arange(len(increments))/len(increments))[::100], color=colors[str(dose)])
		plt.xlabel(r"$\mathrm{Speed} \ \left(\frac{\mathrm{px}}{\frac{1}{60}\mathrm{sec}}\right)$", fontsize=14)
		plt.ylabel("Cumulative proportion", fontsize=14)
		plt.axvline(3, color='k', linestyle='--')
		plt.subplot(1, 2, 1)
		plt.hist(increments, bins=10000, histtype="step", color=colors[str(dose)])
		plt.xscale("log")
		plt.yscale("log")
		plt.xlabel(r"$\mathrm{Speed} \ \left(\frac{\mathrm{px}}{\frac{1}{60}\mathrm{sec}}\right)$", fontsize=14)
		plt.ylabel("Count", fontsize=14)
		plt.axvline(3, color='k', linestyle='--')

		plt.figure(2)

		index = np.where(increments > 3)[0][0]
		print(index)
		proportion = index/len(increments)
		proportionDict["numVelocities"].append(element["numVelocities"])
		proportionDict["dose"].append(dose)
		proportionDict["activityProportion"].append(1-proportion)
		proportionDict["bol"].append(bol)
		proportionDict["dager_etter_levering"].append(dager_etter_levering)


dataFrame = pandas.DataFrame(proportionDict)
print(dataFrame)
dataFrame.boxplot(by="dose")

dataFrame.to_csv("~/activity.csv")


		#myRunningMean = RunningMean(1000)
		#plt.plot(myRunningMean(element["numParticles"]), color=colors[element["dose"]])
            # if "trajectories.csv" in file:
            # 	print(file)
            # 	dose = str(int(oversikt[oversikt["filnavn"]==file]["dose"]))
            	
            # 	trajectoriesFilename = os.path.join(basePath, folder, file)
            # 	fileSize = os.stat(trajectoriesFilename).st_size
            # 	if fileSize<1e6:
	           #  	continue
            # 	tr = pandas.DataFrame.from_csv(trajectoriesFilename)

            # 	#print(tr)
            # 	incrementsAll = []
            # 	for particle in tr.groupby('particle'):
            # 		arclength, increments = calculateTrajctoryArcLength(particle[1])
            # 		incrementsAll.extend(increments)
            # 	plt.figure(1)
            # 	plt.plot(np.sort(incrementsAll), np.arange(len(incrementsAll))/len(incrementsAll), color=colors[dose])

            # 	numParticlesAll = []
            # 	for frame in tr.groupby('frame'):
            # 		numParticlesAll.append(len(frame[1]))
            # 		#print(len(frame[1]))
            	
            # 	numParticlesAll = np.asarray(numParticlesAll)
            # 	myRunningMean = RunningMean(60)
            # 	plt.figure(2)
            # 	plt.plot(range(len(numParticlesAll)), myRunningMean(numParticlesAll), color=colors[dose])
plt.show()



