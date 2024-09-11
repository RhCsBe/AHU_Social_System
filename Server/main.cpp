#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //设置工作目录，若不存在，则创建对应文件夹
    Protocol::setWorkPath(a.applicationDirPath()+"/userData");
    Protocol::createWorkPath();

    Widget w;
    w.show();
    return a.exec();
}
