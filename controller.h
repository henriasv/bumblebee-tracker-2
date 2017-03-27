#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QDebug>
#include <controllerimageprovider.h>
#include <beetracker2d.h>

class Controller : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int frameMin READ frameMin WRITE setFrameMin NOTIFY frameMinChanged)
    Q_PROPERTY(int frameMax READ frameMax WRITE setFrameMax NOTIFY frameMaxChanged)
    Q_PROPERTY(int threshold READ threshold WRITE setThreshold NOTIFY thresholdChanged)


    int m_frameMax;
    int m_frameMin;

    int m_threshold;

public:
    Q_INVOKABLE void update();
    explicit Controller(QObject *parent = 0);
    ControllerImageProvider* m_imageProvider;
    BeeTracker2d camA;
    BeeTracker2d camB;

    QPixmap handlePixmapRequest(QString cam, int frameIndex, QString mode);


int frameMax() const
{
    return m_frameMax;
}

int frameMin() const
{
    return m_frameMin;
}

int threshold() const
{
    return m_threshold;
}

signals:

void frameMaxChanged(int frameMax);

void frameMinChanged(int frameMin);

void thresholdChanged(int threshold);

public slots:
void setFrameMax(int frameMax)
{
    if (m_frameMax == frameMax)
        return;

    m_frameMax = frameMax;
    emit frameMaxChanged(frameMax);
}
void setFrameMin(int frameMin)
{
    if (m_frameMin == frameMin)
        return;

    m_frameMin = frameMin;
    emit frameMinChanged(frameMin);
}
void setThreshold(int threshold)
{
    if (m_threshold == threshold)
        return;

    m_threshold = threshold;
    emit thresholdChanged(threshold);
}
};

#endif // CONTROLLER_H
