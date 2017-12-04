#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QDebug>
#include <memory>
#include <controllerimageprovider.h>
#include <beetracker2d.h>
#include <stereohandler.h>
#include <fstream>
#include <QUrl>

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
    Q_INVOKABLE void loadJsonMetadataFile(QUrl filename);
    Q_INVOKABLE void requestFrameUpdate(QString contents, int threshold, bool stereo);
    Q_INVOKABLE void setParameters(int window1, int window2, int minimumArea, int maximumArea);
    Q_INVOKABLE void initializeJsonFile(QUrl filename);
    Q_INVOKABLE void finalizeJsonFile();
    Q_INVOKABLE void appendKeypointsToFile();

    explicit Controller(QObject *parent = 0);
    ControllerImageProvider* m_imageProvider;

    std::shared_ptr<BeeTracker2d> camA;
    std::shared_ptr<BeeTracker2d> camB;

    std::shared_ptr<StereoHandler> m_stereo;

    QPixmap handlePixmapRequest(QString cam, int frameIndex, QString mode);

    int m_framesInProcess = 0;

    int frameMax() const;

    int frameMin() const;

    int threshold() const;
private:
    std::ofstream m_jsonFile;
    bool hasWrittenFrame = false;
    QPixmap* m_pixmapA;
    QPixmap* m_pixmapB;
    QPixmap* m_pixmapStereo;

signals:

    void frameMaxChanged(int frameMax);

    void frameMinChanged(int frameMin);

    void thresholdChanged(int threshold);

    void framesUpdated();

public slots:
    void setFrameMax(int frameMax);
    void setFrameMin(int frameMin);
    void setThreshold(int threshold);
};

#endif // CONTROLLER_H
