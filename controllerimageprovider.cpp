#include "controllerimageprovider.h"
#include <QDebug>
#include <iostream>
#include "controller.h"

ControllerImageProvider::ControllerImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
{

}


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
