#include <QApplication>
#include "AppEngine.h"
#include "widgets/MainWindow.h"
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    if (!AppEngine::instance().initialize()) {//获取引擎单例后启动
        return 1;
    }
    MainWindow w;
    qDebug() << "创建窗口！";
    w.show();
    return app.exec();
}