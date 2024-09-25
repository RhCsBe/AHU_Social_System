#include "maininterface.h"
#include <QApplication>
#include "login.h"
#include <ElaApplication.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //初始化Ela参数，否则会出现一些奇怪的渲染问题
    ElaApplication::getInstance()->init();
    //设置应用图标
    a.setWindowIcon(QIcon("://photo/AHU1.png"));
    //设置工作文件路径和创建用户数据目录
    Protocol::setWorkPath(a.applicationDirPath()+"/userData");
    Protocol::createWorkPath();
    //启动所有界面
    MainInterface w;
    w.show();
    //w.hide();
    //QApplication::setQuitOnLastWindowClosed(false);
    return a.exec();
}
