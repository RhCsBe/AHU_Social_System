#ifndef TCPTHREAD_H
#define TCPTHREAD_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>

//因为一开始创建时没有让TcpThread继承QObject,后来手动添加上去的继承，并使用Q_OBJECT宏
//导致debug模式下编译无法通过，但是release模式下可以通过
//预测原因：缺少moc文件，元对象系统出了问题，然后makefile报错，但在pro文件中找不到修改的地方
//报错：找不到TcpThread的虚表，然后makefile文件出错
//解决方法：重新构建，重启解决一切问题
class TcpThread:public QObject
{
    //老问题了，不继承QObject并使用这个宏，定义signal会报错
    Q_OBJECT
public:
    TcpThread();

private:
    QTcpSocket* socket=nullptr;
    QHostAddress address=QHostAddress("127.0.0.1");
    int port=10086;
};

#endif // TCPTHREAD_H
