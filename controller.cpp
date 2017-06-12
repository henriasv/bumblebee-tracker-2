#include "controller.h"
#include <QDebug>


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
        m_stereo->compute();
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
}

Controller::Controller(QObject *parent) : QObject(parent)
{
    m_frameMax = 1;
    m_frameMin = 0;
    m_imageProvider = new ControllerImageProvider;
    m_imageProvider->setController(this);
    camA = std::make_shared<BeeTracker2d>();
    camB = std::make_shared<BeeTracker2d>();
    //camA->load("/Users/henriksveinsson/projects/BumblebeeTracker/testVideo/GOPR0034.MP4", true);
    //camB->load("/Users/henriksveinsson/projects/BumblebeeTracker/testVideo/GOPR0055.MP4", false);
    camA->load("/Users/henriksveinsson/Dropbox/humlevideo/GP010017.MP4", true);
    camB->load("/Users/henriksveinsson/Dropbox/humlevideo/GP010036.MP4", false);
    m_stereo = std::make_shared<StereoHandler>(camA, camB);
    setFrameMax(camB->getMaxFrame());
}

QPixmap Controller::handlePixmapRequest(QString cam, int frameIndex, QString mode)
{

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
