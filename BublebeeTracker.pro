TEMPLATE = app
TARGET = Bumblebee
QT += qml quick multimediawidgets
CONFIG += qt c++14 plugin

unix: LIBS += -L/usr/local/lib -lopencv_cudafeatures2d -lopencv_cudastereo -lopencv_cudafilters -lopencv_cudaimgproc -lopencv_cudaarithm  -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_xfeatures2d -lopencv_objdetect -lopencv_highgui -lopencv_videoio -lopencv_photo -lopencv_imgcodecs -lopencv_video -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core
INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include

SOURCES += main.cpp \
    controllerimageprovider.cpp \
    controller.cpp \
    beetracker2d.cpp \
    stereohandler.cpp \
    functions.cpp

RESOURCES += qml.qrc \
    qmldir.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += controllerimageprovider.h \
    controllerimageprovider.h \
    controller.h \
    beetracker2d.h \
    stereohandler.h \
    functions.h

