#ifndef TCPTHREAD_H
#define TCPTHREAD_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QHash>
#include "protocol.h"
#include <QByteArray>
#include <QString>
#include <QMessageBox>

//信号receiveFinished报错
//报错：构建项目时无法找到receiveFinished的实现，以及在元对象文件中找不到该信号
//推测原因：项目已经构建完毕，元对象文件已生成，系统未检测到moc对象发生修改，所以哪怕重新构建也无法解决（疑似一开始未添加Q_OBJECT或未继承QObject类就已经构建生成元对象）
//解决方法：在尝试重新构建无法解决后，尝试删除生成项目文件夹后，重新构建，删除后系统会根据当前代码情况重新生成moc对象，问题解决
//警告：下次一定要在创建文件时，把继承和宏搞定，让系统自己完成链接以及生成元对象等一系列工作，省的又出来奇奇怪怪的报错
class TcpThread:public QObject
{
    Q_OBJECT
public:
    explicit TcpThread(QObject *parent = nullptr);

    //初始化服务器
    void initServer();
    //获取客户端发来的数据
    void getData();
    //回复客户端或转发数据
    void sendToClient(QString key, int type, QString account, QString targetAccount, QByteArray jsonData, QString messageType, QString fileName);
    //发送普通文件
    void SendFile(QTcpSocket* socket, QString fileName, QString senderAccount, int type);
    //发送json文件
    void sendJson(QTcpSocket* socket,QByteArray jsonData);

signals:
    void receiveFinished(QString key, QByteArray dataBuffer);
    void myInformation(QString str);

private:
    QTcpServer* server=nullptr;
    QHostAddress address=QHostAddress::AnyIPv4;
    int port=10086;
    QHash<QString,QTcpSocket*> allSocket;
    QHash<QString,QByteArray*> allDateBuffer;
};

#endif // TCPTHREAD_H
