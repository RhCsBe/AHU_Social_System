#include "tcpthread.h"

TcpThread::TcpThread(QObject *parent): QObject(parent)
{

}

void TcpThread::initServer()
{
    server=new QTcpServer();
    server->listen(address,port);
    connect(server,&QTcpServer::newConnection,this,[&](){
        while(server->hasPendingConnections())
        {
            QTcpSocket* socket=server->nextPendingConnection();
            QByteArray* byteArray=new QByteArray();
            connect(socket,&QTcpSocket::readyRead,this,&TcpThread::getData,Qt::QueuedConnection);
            QString key=socket->peerAddress().toString()+QString::number(socket->peerPort());
            qDebug()<<key;
            allSocket.insert(key,socket);
            allDateBuffer.insert(key,byteArray);
        }
    });
}

void TcpThread::getData()
{
    //获取触发readyRead信号的套接字指针
    QTcpSocket* socket=(QTcpSocket*)sender();
    //获取socket对应的缓冲区
    QString key=socket->peerAddress().toString()+QString::number(socket->peerPort());
    QByteArray* dataBuffer=allDateBuffer.value(key);
    //如果有可读取的数据则读取
    if(socket->bytesAvailable())
    {
        QByteArray receiveData=socket->readAll();
        int dataSize=receiveData.size();
        //如果接受到的数据块大小加上缓冲区数据大小等于数据包大小，则直接发送接收完成信号
        if(dataSize+dataBuffer->size()==BufferSize)
        {
            QByteArray dataArray;
            dataArray.append(*dataBuffer);
            dataArray.append(receiveData);
            emit receiveFinished(key,dataArray);
            emit myInformation("收到一个数据包");
            //QMessageBox::information(nullptr,"提示","收到一个数据包");
            dataBuffer->clear();
        }
        //如果接受到的数据块大小加上缓冲区数据大小大于数据包大小，则将数据拼接成一个数据包大小，发送接收完成信号，并将剩余数据装入缓冲区
        else if(dataSize+dataBuffer->size()>BufferSize)
        {
            dataBuffer->append(receiveData);
            while(dataBuffer->size()>=BufferSize)
            {
                receiveFinished(key,dataBuffer->left(BufferSize));
                dataBuffer->remove(0,BufferSize);
            }
        }
        //如果接受到的数据块大小加上缓冲区数据大小小于数据包大小，则直接进行数据拼接
        else if(dataSize+dataBuffer->size()<BufferSize)
        {
            dataBuffer->append(receiveData);
        }
    }
    else
    {
        return;
    }

}

void TcpThread::sendToClient(QString key, int type, QString account, QString targetAccount, QByteArray jsonData, QString messageType, QString fileName)
{
    QTcpSocket* socket;
    socket=allSocket[key];
    switch(type)
    {
        case LoginAccount:
        case AllHeadPhoto:
        {
            if(fileName!="")
            {
                QStringList list=fileName.split("?");
                for(auto i:list)
                {
                    qDebug()<<i;
                    SendFile(socket,i,"",type);
                }
            }
            break;
        }

    }
    if(jsonData.size()>0)
    {
        qDebug()<<"发送json数据";
        sendJson(socket,jsonData);
    }
}

void TcpThread::SendFile(QTcpSocket *socket, QString fileName, QString senderAccount, int type)
{
    QFile file(fileName);
    //文件打不开直接退出
    if(!file.open(QFile::ReadOnly))
    {
        qDebug()<<"文件"<<fileName<<"打开失败";
        return;
    }
    //获取文件的大小和数据，并为输出缓冲区绑定数据流（只写模式）
    int fileSize=file.size();
    //获取真正的文件名
    QString fName=fileName.split("/").last();
    //QByteArray fileData=file.readAll();
    QByteArray writeData;

    //发送文件头数据包
    QDataStream fileHead(&writeData,QFile::WriteOnly);
    //QString用QDataStream序列化其大小会*2后+4
    int dataSize=sizeof(type)+sizeof(fileSize)+(fName.size()*2+4)+(senderAccount.size()*2+4);
    //写入文件头数据包包头
    fileHead<<FileType::FileInfoHead<<qint16(0)<<dataSize;
    //写入数据
    fileHead<<type<<fileSize<<fName<<senderAccount;
    writeData.resize(BufferSize);
    socket->write(writeData);
    socket->waitForBytesWritten();
    //发送完要清空缓冲区，后面还得用
    writeData.clear();

    //发送文件数据数据包
    int packetSum=fileSize/NoHeadBufferSize;
    int addOne=fileSize%NoHeadBufferSize;
    //如果存在剩余数据不满一个数据包数据块大小，则发送的数据包数量+1
    if(addOne)
        packetSum++;
    for(int i=0;i<packetSum-1;i++)
    {
        /*
    将QDataStream对象所关联的QByteArray对象清空后，用QDataStream对象去继续写入数据到QByteArray对象中，
    结果并不如预期那样从位置0开始写入，而是从之前的位置开始写入，前面的数据呈现未定义状态（QBuffer有一样的问题）
    */
        QByteArray packetData=file.read(NoHeadBufferSize);
        QDataStream out(&writeData,QFile::WriteOnly);
        out<<FileType::FileDataHead<<qint16(0)<<NoHeadBufferSize;
        writeData.append(packetData);
        socket->write(writeData);
        socket->waitForBytesWritten();
        writeData.clear();
    }

    //发送文件尾数据包
    QByteArray packetData=file.readAll();//读取剩余数据
    QDataStream out(&writeData,QFile::WriteOnly);
    out<<FileType::FileEndDataHead<<qint16(0)<<packetData.size();
    writeData.append(packetData);
    //如果大小不够，填充成完整数据包大小
    if(writeData.size()<BufferSize)
        writeData.resize(BufferSize);
    socket->write(writeData);
    socket->waitForBytesWritten();
    writeData.clear();

    //关闭文件
    file.close();
    qDebug()<<fileName<<"发送完毕";
}

void TcpThread::sendJson(QTcpSocket *socket, QByteArray jsonData)
{
    //用数据流打开数据包
    QByteArray dataArray;
    QDataStream out(&dataArray,QFile::WriteOnly);
    //写入数据包包头
    out<<FileType::JsonDataHead<<qint16(0)<<jsonData.size();
    //添加数据
    dataArray.append(jsonData);
    //将数据包大小设置为协议大小
    dataArray.resize(BufferSize);
    //发送数据并阻塞等待
    socket->write(dataArray);
    socket->waitForBytesWritten();
    emit myInformation("服务器发送了一个json数据包");
}
