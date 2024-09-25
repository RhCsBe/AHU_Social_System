#include "tcpthread.h"

TcpThread::TcpThread()
{
    //为了保证线程安全，要先将TcpThread移入子线程后再new对象
    //connectServer();
}

void TcpThread::connectServer()
{
    if(!socket)
    {
        socket=new QTcpSocket(this);
        connect(socket,&QTcpSocket::connected,this,[&](){
            Protocol::isConnecting=true;
            qDebug()<<"连接成功";
        });
        connect(socket,&QTcpSocket::disconnected,this,[&](){
            Protocol::isConnecting=false;
            //socket->close();
            qDebug()<<"连接失败";
        });
        connect(socket,&QTcpSocket::readyRead,this,&TcpThread::getData,Qt::QueuedConnection);
        qDebug()<<"等待连接";
    }
    socket->connectToHost(address,port);
}

void TcpThread::autoConnect()
{
    if(!timer)
    {
        timer=new QTimer(this);
        connect(timer,&QTimer::timeout,this,[&](){
            connectServer();
            socket->waitForConnected(1000);
            qDebug()<<"hahah";
            if(Protocol::isConnecting)
            {
                timer->stop();
            }
        });
    }
    timer->start(5000);
}

void TcpThread::sendToServer(QByteArray jsonData, QString fileName, int type, int fileNums, QString recvAccount)
{
//    //若文件名不为空则为发送文件
//    if(fileName != "")
//    {
//        if(fileNums > 1)
//        {
//            //用?号分割文件名,因为?号不能作为文件名
//            QStringList fileNames = fileName.split("?");

//            for(auto fileName : fileNames)
//            {
//                sendFile(fileName, type, recvAccount);
//            }
//        }
//        else
//        {
//            sendFile(fileName, type, recvAccount);
//        }
//    }

    if(jsonData.size() != 0)
    {
        sendJson(jsonData);
    }
}

void TcpThread::sendFile(QString fileName, QString sendToAccount, int type)
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
    int dataSize=sizeof(type)+sizeof(fileSize)+(fName.size()*2+4)+(sendToAccount.size()*2+4);
    //写入文件头数据包包头
    fileHead<<FileType::FileInfoHead<<qint16(0)<<dataSize;
    //写入数据
    fileHead<<type<<fileSize<<fName<<sendToAccount;
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
}

void TcpThread::sendJson(QByteArray jsonData)
{
    QByteArray dataArray;
    QDataStream out(&dataArray,QFile::WriteOnly);
    out<<JsonDataHead;
    out<<qint16(0);
    out<<jsonData.size();
    dataArray.append(jsonData);
    dataArray.resize(BufferSize);
    socket->write(dataArray);
    socket->waitForBytesWritten();
    //QMessageBox::information(nullptr,"提示","客户端发送数据成功");
    emit myInformation("客户端发送数据成功");
}

void TcpThread::getData()
{
    //如果有可读取的数据则读取
    if(socket->bytesAvailable())
    {
        QByteArray receiveData=socket->readAll();
        int dataSize=receiveData.size();
        //如果接受到的数据块大小加上缓冲区数据大小等于数据包大小，则直接发送接收完成信号
        if(dataSize+dataBuffer.size()==BufferSize)
        {
            dataBuffer.append(receiveData);
            emit myInformation("客户端收到一个数据包");
            //qDebug()<<"客户端收到一个数据包";
            //QMessageBox::information(nullptr,"提示","客户端收到一个数据包");
            mergeDataPackage(dataBuffer);
            dataBuffer.clear();
        }
        //如果接受到的数据块大小加上缓冲区数据大小大于数据包大小，则将数据拼接成一个数据包大小，发送接收完成信号，并将剩余数据装入缓冲区
        else if(dataSize+dataBuffer.size()>BufferSize)
        {
            int otherSize=BufferSize-dataBuffer.size();
            dataBuffer.append(receiveData.left(otherSize));
            receiveData.remove(0,otherSize);
            mergeDataPackage(dataBuffer);
            dataBuffer.clear();
            dataBuffer.append(receiveData);
        }
        //如果接受到的数据块大小加上缓冲区数据大小小于数据包大小，则直接进行数据拼接
        else if(dataSize+dataBuffer.size()<BufferSize)
        {
            dataBuffer.append(receiveData);
        }
    }
    else
    {
        return;
    }

}

void TcpThread::mergeDataPackage(QByteArray dataArray)
{
    QDataStream fileHead(&dataArray,QFile::ReadOnly);
    FileType fileType;
    qint16 space;
    int packetSize;
    fileHead>>fileType>>space>>fileSize;
    dataArray.remove(0,10);
    switch (fileType)
    {
    case JsonDataHead:
        //json数据可以直接解析
        parseMessage(dataArray.left(packetSize));
        break;
    case FileInfoHead:
        {
            int type,fileSize;
            QString fileName,sendToAccount;
            fileHead>>type>>fileSize>>fileName>>sendToAccount;
            this->fileSize=fileSize;
            this->nowFileSize=0;
            switch(type)
            {
                case LoginAccount://接收登录数据
                {
                    //第一次登录时文件夹不存在，需要创建文件夹
                    QDir dir;
                    if(dir.exists(Protocol::getUserPath()))
                    {
                        if(!dir.mkdir(Protocol::getUserPath()))
                        {
                            qDebug()<<"文件夹"+Protocol::getUserAccount()<<"创建失败";
                            return;
                        }
                        if(!dir.mkdir(Protocol::getUserPath()+"/FileRecv"))
                        {
                            qDebug()<<"文件夹"+Protocol::getUserAccount()+"/FileRecv"<<"创建失败";
                            return;
                        }
                    }
                    file.setFileName(Protocol::getUserPath()+"/"+fileName);
                    if(!file.open(QFile::WriteOnly|QFile::Truncate))
                    {
                    qDebug()<<"文件"<<fileName<<"打开失败";
                    return;
                    }
                    //fileProgress.insert(fileName,QPair<0,fileSize>);
                    break;
                }
                case AllUserData://接收好友头像
                {
                    file.setFileName(Protocol::getAllUserPath()+"/"+fileName);
                    if(!file.open(QFile::WriteOnly|QFile::Truncate))
                    {
                    qDebug()<<"文件"<<fileName<<"打开失败";
                    return;
                    }
                    break;
                }
            }
        }
        break;
    case FileDataHead:
        {
            file.write(dataArray);
            nowFileSize+=NoHeadBufferSize;
        }
        break;
    case FileEndDataHead:
        {
            file.write(dataArray.left(fileSize-nowFileSize));
            file.close();
        }
        break;
    default:
        break;
    }
}

void TcpThread::parseMessage(QByteArray dataArray)
{
    QJsonObject jsonData=QJsonDocument::fromJson(dataArray).object();
    int type=jsonData.value("type").toInt();
    switch(type)
    {
        case LoginAccount:
        {
            QString result=jsonData.value("result").toString();
            qDebug()<<"result:"<<result;
            if(result=="登录成功")
            {
                emit loginSuccess(true);
            }
            else
            {
                emit loginSuccess(false);
            }
            break;
        }
        default:
            break;
    }
}

void TcpThread::getJsonData(int type, QString account, QString targetAccount, QString message,QString messageType)
{
    QString fileName = "";
    int fileNums = -1;
    QString recvaccount = "";
    QJsonObject data;
    data.insert("type", type);
    switch(type)
    {
        case LoginAccount:
            data.insert("account",account);
            data.insert("password",message);
            break;

        default:
            break;
    }
    QJsonDocument document(data);
    sendToServer(document.toJson(),fileName,type,fileNums,recvaccount);
}
