#ifndef CONTROLLERIMAGEPROVIDER_H
#define CONTROLLERIMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv/highgui.h>

class Controller;
class ControllerImageProvider : public QQuickImageProvider
{

public:
    ControllerImageProvider();
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;
    void setVideoSource(QString source);
    void setController(Controller* controller) {m_controller = controller;}

private:
    cv::VideoCapture m_videoCapture;
    Controller * m_controller;

};

#endif // CONTROLLERIMAGEPROVIDER_H
