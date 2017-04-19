#include "controller.h"
#include <QDebug>
#include <jsoncpp/json/json.h>


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

Controller::Controller(QObject *parent) : QObject(parent)
{
    m_frameMax = 1;
    m_frameMin = 0;
    m_imageProvider = new ControllerImageProvider;
    m_imageProvider->setController(this);
    camA = std::make_shared<BeeTracker2d>();
    camB = std::make_shared<BeeTracker2d>();
    camA->load("/home/henriasv/git_repos/bumblebee-tracker-2/testVideo/GOPR0034.MP4", true);
    camB->load("/home/henriasv/git_repos/bumblebee-tracker-2/testVideo/GOPR0055.MP4", false);
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
