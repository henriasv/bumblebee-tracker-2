#include "controller.h"
#include <QDebug>
void Controller::update()
{
    camA.setThreshold(m_threshold);
    camB.setThreshold(m_threshold);
    qDebug() << "in update";
    qDebug() << "threshold: " << m_threshold;
}

Controller::Controller(QObject *parent) : QObject(parent)
{
    m_frameMax = 1;
    m_frameMin = 0;
    m_imageProvider = new ControllerImageProvider;
    m_imageProvider->setController(this);
    camA.load("/Users/henriksveinsson/projects/BumblebeeTracker/testVideo/GP050029.MP4", false);
    camB.load("/Users/henriksveinsson/projects/BumblebeeTracker/testVideo/GP050050.MP4", true);
    setFrameMax(camB.getMaxFrame());
}

QPixmap Controller::handlePixmapRequest(QString cam, int frameIndex, QString mode)
{
    if (cam == QString("A"))
    {
        cv::Mat frame = camA.getFrame(frameIndex, mode.toStdString());
        return QPixmap::fromImage(QImage((unsigned char*) frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888));
    }
    else if (cam == QString("B"))
    {
        cv::Mat frame = camB.getFrame(frameIndex, mode.toStdString());
        return QPixmap::fromImage(QImage((unsigned char*) frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888));
    }

    else
    {
        qDebug() << "Invalid camera for controller: " << cam;
    }
}
