#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QVideoWidget>
#include <QDebug>
#include <QQmlContext>
#include <QScopedPointer>

#include <controllerimageprovider.h>
#include <controller.h>

int main(int argc, char *argv[])
{
    qmlRegisterType<Controller>("TrackingController", 1, 0, "Controller");

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    Controller* controller = new Controller();
    //engine.rootContext()->setContextObject(controller);
    engine.rootContext()->setContextProperty("controller", controller);
    engine.addImageProvider(QLatin1String("bumblebee"), controller->m_imageProvider);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    return app.exec();
}
