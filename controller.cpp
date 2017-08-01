#include "controller.h"
#include <QDebug>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <utility>
#include <iostream>

void Controller::loadJsonMetadataFile(QUrl filename)
{
    boost::property_tree::ptree root;
    boost::property_tree::read_json(filename.toLocalFile().toStdString(), root);
    std::string pathA(root.get<std::string>("fileData.pathA"));
    std::string pathB(root.get<std::string>("fileData.pathB"));
    std::cout << "Path A read by controller from file: " << pathA << std::endl;
    std::cout << "Path B read by controller from file: " << pathB << std::endl;
    camA->load(pathA, true);
    camB->load(pathB, false);

    int frameOffsetA = root.get<int>("fileData.frameOffsetA");
    int frameOffsetB = root.get<int>("fileData.frameOffsetB");
    camA->setFrameOffset(frameOffsetA);
    camB->setFrameOffset(frameOffsetB);

    std::vector<int> cornersA;
    std::vector<int> cornersB;

    boost::property_tree::ptree arenaData = root.get_child("arenaData");
    for (boost::property_tree::ptree::value_type& row : arenaData.get_child("cornerPixelsA"))
        for (boost::property_tree::ptree::value_type & value : row.second)
            cornersA.push_back(value.second.get_value<int>());
    for (boost::property_tree::ptree::value_type& row : arenaData.get_child("cornerPixelsB"))
        for (boost::property_tree::ptree::value_type & value : row.second)
            cornersB.push_back(value.second.get_value<int>());

    camA->setRoiMaskVector(cornersA);
    camB->setRoiMaskVector(cornersB);

    int FDthreshL0Yellow = root.get<int>("flowerDetection.threshL0Yellow");
    int FDthreshL1Yellow = root.get<int>("flowerDetection.threshL1Yellow");
    int FDthreshL2Yellow = root.get<int>("flowerDetection.threshL2Yellow");
    int FDthreshL0BlueLight = root.get<int>("flowerDetection.threshL0BlueLight");
    int FDthreshL2BlueLight = root.get<int>("flowerDetection.threshInvL2BlueLight");
    int FDthreshL2BlueDark = root.get<int>("flowerDetection.threshInvL2BlueDark");
    std::string FDflowerType(root.get<std::string>("flowerDetection.flowerType"));

    int CFthreshL1Yellow = root.get<int>("colorFiltering.threshL1Yellow");
    int CFthreshL2Yellow = root.get<int>("colorFiltering.threshL2Yellow");
    int CFthreshL0BlueLight = root.get<int>("colorFiltering.threshL0BlueLight");
    int CFthreshL2BlueLight = root.get<int>("colorFiltering.threshInvL2BlueLight");
    int CFthreshL2BlueDark = root.get<int>("colorFiltering.threshInvL2BlueDark");

    camA->setFlowerDetectionColorFilterParameters(FDflowerType, FDthreshL0Yellow, FDthreshL1Yellow, FDthreshL2Yellow, FDthreshL0BlueLight, FDthreshL2BlueLight, FDthreshL2BlueDark);
    camB->setFlowerDetectionColorFilterParameters(FDflowerType, FDthreshL0Yellow, FDthreshL1Yellow, FDthreshL2Yellow, FDthreshL0BlueLight, FDthreshL2BlueLight, FDthreshL2BlueDark);

    camA->setColorFilteringParameters(CFthreshL1Yellow, CFthreshL2Yellow, CFthreshL0BlueLight, CFthreshL2BlueLight, CFthreshL2BlueDark);
    camB->setColorFilteringParameters(CFthreshL1Yellow, CFthreshL2Yellow, CFthreshL0BlueLight, CFthreshL2BlueLight, CFthreshL2BlueDark);


    //int BDthresh = root.get<int>("beeDetection.threshold");
    //int BDlowerGaussian = root.get<int>("beeDetection.lowerGaussian");
    //int BDupperGaussian = root.get<int>("beeDetection.upperGaussian");
    //int BDminArea = root.get<int>("beeDetection.minArea");
    //int BDmaxArea = root.get<int>("beeDetection.maxArea");
}

void Controller::requestFrameUpdate(QString id, int threshold, bool stereo)
{
    camA->setThreshold(threshold);
    camB->setThreshold(threshold);

    QStringList strings = id.split("/");
    int frameIndex = strings[0].toInt();
    QString mode = strings[1];
    camA->getFrame(frameIndex, mode.toStdString());
    camB->getFrame(frameIndex, mode.toStdString());

    if (stereo)
    {
        m_stereo->compute(mode.toStdString());
    }

    emit framesUpdated();
}


void Controller::setParameters(int window1, int window2, int minimumArea, int maximumArea)
{
    camA->setParameters(window1, window2, minimumArea, maximumArea);
    camB->setParameters(window1, window2, minimumArea, maximumArea);
}

void Controller::initializeJsonFile(QUrl filename)
{
    m_jsonFile.open(filename.toLocalFile().toStdString(), std::ofstream::out);
    if (!m_jsonFile.is_open())
    {
        qDebug() << "Could not open json file in Controller";
    }
    m_jsonFile << "[\n";
}

void Controller::finalizeJsonFile()
{
    m_jsonFile << "\n]\n";
    m_jsonFile.close();
}

void Controller::appendKeypointsToFile()
{
    if (hasWrittenFrame)
    {
        m_jsonFile << ",\n";
    }
    else
    {
        hasWrittenFrame = true;
    }
    m_jsonFile << camA->getDumpString().toStdString();
    m_jsonFile << ",\n";
    m_jsonFile << camB->getDumpString().toStdString();
}

Controller::Controller(QObject *parent) : QObject(parent)
{
    m_frameMax = 1;
    m_frameMin = 0;
    m_imageProvider = new ControllerImageProvider;
    m_imageProvider->setController(this);
    camA = std::make_shared<BeeTracker2d>("A");
    camB = std::make_shared<BeeTracker2d>("B");
    //camA->load("/Users/henriksveinsson/projects/BumblebeeTracker/testVideo/GOPR0034.MP4", true);
    //camB->load("/Users/henriksveinsson/projects/BumblebeeTracker/testVideo/GOPR0055.MP4", false);
    //camA->load("/Users/henriksveinsson/Dropbox/humlevideo/GP010017.MP4", true);
    //camB->load("/Users/henriksveinsson/Dropbox/humlevideo/GP010036.MP4", false);
    //camA->load("/home/henriasv/testvideo/GOPR0018.mp4", true);
    //camB->load("/home/henriasv/testvideo/GOPR0039.mp4", false);
    //camA->load("/home/henriasv/testvideo/test_concat/A/concat.mp4", true);
    //camB->load("/home/henriasv/testvideo/test_concat/B/concat.mp4", false);

    m_stereo = std::make_shared<StereoHandler>(camA, camB);

}

QPixmap Controller::handlePixmapRequest(QString cam, int frameIndex, QString mode)
{
    setFrameMax(camB->getMaxFrame());
    if (cam == QString("A"))
    {
        cv::Mat frame = camA->m_cpuFrame;
        return QPixmap::fromImage(QImage((unsigned char*) frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888));
    }
    else if (cam == QString("B"))
    {
        cv::Mat frame = camB->m_cpuFrame;
        return QPixmap::fromImage(QImage((unsigned char*) frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888));
    }

    else if (cam == QString("Stereo"))
    {
        cv::Mat frame = m_stereo->m_frame;
        return QPixmap::fromImage(QImage((unsigned char*) frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888));
    }

    else
    {
        qDebug() << "Invalid camera for controller: " << cam;
    }
}

int Controller::frameMax() const
{
    return m_frameMax;
}

int Controller::frameMin() const
{
    return m_frameMin;
}

int Controller::threshold() const
{
    return m_threshold;
}

void Controller::setFrameMax(int frameMax)
{
    if (m_frameMax == frameMax)
        return;

    m_frameMax = frameMax;
    emit frameMaxChanged(frameMax);
}

void Controller::setFrameMin(int frameMin)
{
    if (m_frameMin == frameMin)
        return;

    m_frameMin = frameMin;
    emit frameMinChanged(frameMin);
}

void Controller::setThreshold(int threshold)
{
    if (m_threshold == threshold)
        return;

    m_threshold = threshold;
    emit thresholdChanged(threshold);
}
