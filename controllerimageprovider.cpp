#include "controllerimageprovider.h"
#include "controller.h"
#include <iostream>

ControllerImageProvider::ControllerImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
{
}

QPixmap ControllerImageProvider::requestPixmap(const QString& id, QSize* size, const QSize& requestedSize)
{

    QStringList strings = id.split("/");
    QString cam = strings[0];
    int frameIndex = strings[1].toInt();
    QString mode = strings[2];
    qDebug() << "processing to return frame " << id << " with index " << frameIndex;
    qDebug() << strings;
    QPixmap retFrame = m_controller->handlePixmapRequest(cam, frameIndex, mode);
    return retFrame;
}
