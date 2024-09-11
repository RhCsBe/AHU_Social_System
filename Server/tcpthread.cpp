#include "tcpthread.h"

TcpThread::TcpThread()
{
    server=new QTcpServer(this);
    server->listen(address,port);
    connect(server,&QTcpServer::newConnection,this,[&](){
        while(server->hasPendingConnections())
        {
            QTcpSocket* socket=server->nextPendingConnection();
            QString key=socket->peerAddress().toString()+QString::number(socket->peerPort());
            allSocket.insert(key,socket);
        }
    });
}
