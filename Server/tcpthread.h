#ifndef TCPTHREAD_H
#define TCPTHREAD_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QHash>

class TcpThread:public QObject
{
    Q_OBJECT
public:
    TcpThread();

private:
    QTcpServer* server=nullptr;
    QHostAddress address=QHostAddress::AnyIPv4;
    int port=10086;
    QHash<QString,QTcpSocket*> allSocket;
};

#endif // TCPTHREAD_H
