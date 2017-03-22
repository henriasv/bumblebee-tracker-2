#include "controllerimageprovider.h"
#include <QDebug>
#include <iostream>
#include "controller.h"

ControllerImageProvider::ControllerImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
{

}

/*
QPixmap ControllerImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    qDebug() << "processing to return frame " << id;
    int frameIndex = id.toInt();
    cv::Mat img;
    m_videoCapture.set(CV_CAP_PROP_POS_FRAMES, frameIndex);
    int read = m_videoCapture.read(img);
    QPixmap ret = QPixmap::fromImage(QImage((unsigned char*) img.data, img.cols, img.rows, img.step, QImage::Format_RGB888));
    qDebug() << "frameMax: " <<  m_controller->frameMax();
    return ret;
}
*/

QPixmap ControllerImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    qDebug() << "processing to return frame" << id;
    QStringList strings = id.split("/");
    QString cam = strings[0];
    int frameIndex = strings[1].toInt();
    QString mode = strings[2];

    qDebug() << strings;
    QPixmap retFrame =  m_controller->handlePixmapRequest(cam, frameIndex, mode);
    return retFrame;
}
