#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QScopedPointer>
#include <QVideoWidget>
#include <iostream>

#include <controller.h>
#include <controllerimageprovider.h>

int main(int argc, char* argv[])
{
    qmlRegisterType<Controller>("TrackingController", 1, 0, "Controller");

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    Controller controller;
    std::cout << "Created controller" << std::endl;
    // engine.rootContext()->setContextObject(controller);
    engine.rootContext()->setContextProperty("controller", &controller);
    engine.addImageProvider(QLatin1String("bumblebee"),
        controller.m_imageProvider);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    return app.exec();
}
