# PIxel values

pixelsA = [[ 980., 1176.], [704, 630], [791, 2152], [716, 2060], [72, 2623], [74, 112], [178, 129], [1475, 550], [1471, 2123], [628, 158], [662, 295], [1121, 448], [94, 2204], [1135, 1981]]
pixelsB = [[1092., 1176.], [802, 664], [725, 2169], [819, 2081], [70, 2164], [63, 621], [121, 588], [1481, 112], [1458, 2621], [640, 287], [394, 500], [848, 351], [377, 1980], [1347, 2226]]

if __name__ == "__main__":
	for element in pixelsA:
		print "pointsA.push_back(cv::Point2f(%f, %f));" % (element[0], element[1])
	for element in pixelsB:
		print "pointsB.push_back(cv::Point2f(%f, %f));" % (element[0], element[1])