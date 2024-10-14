#ifndef TCPTHREAD_H
#define TCPTHREAD_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include "protocol.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include <QMessageBox>
#include <QHash>

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

    //连接服务器处理
    void connectServer();
    void autoConnect();

    //数据发送
    void sendToServer(QByteArray jsonData, QString fileName, int type, int fileNums, QString RecvAccount);
    void sendFile(QString fileName,QString sendToAccount,int type);
    void sendJson(QByteArray jsonData);

    //数据接收
    void getData();

    //合并数据包
    void mergeDataPackage(QByteArray dataArray);

    //解析json数据包
    void parseMessage(QByteArray dataArray);

    //数据转换
    void getJsonData(int type, QString account = "", QString targetAccount = "", QString message = "",QString messageType="");

signals:
    void loginSuccess(bool result);
    void myInformation(QString str);

private:
    //套接字、ip、端口、缓冲区
    QTcpSocket* socket=nullptr;
    QHostAddress address=QHostAddress("127.0.0.1");
    int port=10086;
    QByteArray dataBuffer;

    //文件指针
    QFile file;
    int fileSize;
    int nowFileSize;
    //QHash<QString,QPair<int,int>> fileProgress;

    //自动重连定时器
    QTimer* timer=nullptr;
    QTimer* readTimer=nullptr;
    static int num;
};

#endif // TCPTHREAD_H
