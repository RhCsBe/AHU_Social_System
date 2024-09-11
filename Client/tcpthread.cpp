#include "tcpthread.h"

TcpThread::TcpThread()
{
    socket=new QTcpSocket(this);
    connect(socket,&QTcpSocket::connected,this,[&](){qDebug()<<"连接成功";});
    socket->connectToHost(address,port);
    qDebug()<<"等待连接";
}
