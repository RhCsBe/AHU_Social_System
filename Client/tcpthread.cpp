#include "tcpthread.h"

int TcpThread::num=0;

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
        //连接成功处理
        connect(socket,&QTcpSocket::connected,this,[&](){
            Protocol::isConnecting=true;
            //判断是否登陆后重连
            if(Protocol::isLogin)
            {
                //修改登录时间
                Protocol::setLastLoginTime(Protocol::getLoginTime());
                Protocol::setLoginTime(QDateTime::currentMSecsSinceEpoch());
                //发射信号通知主界面存储信息到文件中，后续按登录处理
                emit reconnection();
                getJsonData(InfoType::Reconnection,Protocol::getUserAccount(),"",Protocol::getUserPwd());
            }

            qDebug()<<"连接成功";
        });
        //连接断开处理
        connect(socket,&QTcpSocket::disconnected,this,[&](){
            Protocol::isConnecting=false;
            socket->close();//清除写缓冲区
            startAutoConnet();//开启自动重连
            qDebug()<<"连接断开";
        });
        //数据到达处理
        connect(socket,&QTcpSocket::readyRead,this,&TcpThread::getData,Qt::QueuedConnection);
        qDebug()<<"等待连接";
    }
    socket->connectToHost(address,port);
    //如果连接超时开启自动重连
    if(!socket->waitForConnected(3000))
    {
        qDebug()<<"连接超时，开启自动重连";
        startAutoConnet();
    }
}

void TcpThread::autoConnect()
{
    socket->connectToHost(address,port);
    if(socket->waitForConnected(1000))
    {
        timer->stop();
    }
}

void TcpThread::startAutoConnet()
{
    if(!timer)
    {
        timer=new QTimer(this);
        connect(timer,&QTimer::timeout,this,&TcpThread::autoConnect);
    }
    timer->start(5000);
}

void TcpThread::sendToServer(QByteArray jsonData, QString fileName, int type, int fileNums, QString recvAccount)
{
    qDebug()<<"fileName:"<<fileName;
    //若文件名不为空则为发送文件
    if(fileName != "")
    {
        QStringList list=fileName.split("?");
        for(auto i:list)
        {
            qDebug()<<i;
            SendFile(socket,i,"",type);
        }
    }

    //json数据包不为空则发送json数据
    if(jsonData.size() != 0)
    {
        sendJson(jsonData);
    }
}

void TcpThread::SendFile(QTcpSocket *socket, QString fileName, QString senderAccount, int type)
{
    qDebug()<<"发送文件"<<fileName;
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

void TcpThread::sendJson(QByteArray jsonData)
{
    //创建json数据包并绑定输入流
    QByteArray dataArray;
    QDataStream out(&dataArray,QFile::WriteOnly);

    //写入json文件标志位，置空符，数据包大小
    out<<JsonDataHead;
    out<<qint16(0);
    out<<jsonData.size();

    //写入json数据，并把数据包大小补全
    dataArray.append(jsonData);
    dataArray.resize(BufferSize);

    //发送数据，并阻塞等待
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
            //emit myInformation("客户端收到一个数据包");
            mergeDataPackage(dataBuffer);
            dataBuffer.clear();
        }
        //如果接受到的数据块大小加上缓冲区数据大小大于数据包大小，则将数据拼接成一个数据包大小，发送接收完成信号，并将剩余数据装入缓冲区
        else if(dataSize+dataBuffer.size()>BufferSize)
        {
            dataBuffer.append(receiveData);
            while(dataBuffer.size()>=BufferSize)
            {
                mergeDataPackage(dataBuffer.left(BufferSize));
                dataBuffer.remove(0,BufferSize);
                //QByteArray的remove函数返回值是修改过后的字节序列，不是裁剪的片段，所以会出现数据缺失和错位的现象
                //qDebug()<<"BufferSize:"<<dataBuffer.size();
                //mergeDataPackage(dataBuffer.remove(0,BufferSize));
                //qDebug()<<"BufferSize:"<<dataBuffer.size();
            }
//            int otherSize=BufferSize-dataBuffer.size();
//            dataBuffer.append(receiveData.left(otherSize));
//            receiveData.remove(0,otherSize);
//            mergeDataPackage(dataBuffer);
//            dataBuffer.clear();
//            dataBuffer.append(receiveData);
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
    num++;
    //qDebug()<<"num:"<<num;
    QDataStream fileHead(&dataArray,QFile::ReadOnly);
    FileType fileType;
    qint16 space;
    int packetSize;
    fileHead>>fileType>>space>>packetSize;
    dataArray.remove(0,10);
    //qDebug()<<"fileType:"<<FileType(fileType);
    switch (fileType)
    {
    case JsonDataHead:
        //json数据可以直接解析
        //qDebug()<<"收到json数据";
        parseMessage(dataArray.left(packetSize));
        break;
    case FileInfoHead:
        {
            //上面对数据进行了处理，原来数据流的定位已经不准了，要重新开一个
            QDataStream readFile(&dataArray,QFile::ReadOnly);
            int type,fileSize;
            QString fileName,sendToAccount;
            //fileHead>>type>>fileSize>>fileName>>sendToAccount;
            readFile>>type>>fileSize>>fileName>>sendToAccount;
            this->fileSize=fileSize;
            this->nowFileSize=0;
            //qDebug()<<"type:"<<type;
            switch(type)
            {
                case LoginAccount://接收登录数据
                {
                    //第一次登录时文件夹不存在，需要创建文件夹
                    QDir dir;
                    if(!dir.exists(Protocol::getUserPath()))
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
                        if(!dir.mkdir(Protocol::getUserPath()+"/message"))
                        {
                            qDebug()<<"文件夹"+Protocol::getUserAccount()+"/message"<<"创建失败";
                            return;
                        }
                    }
                    file.setFileName(Protocol::getUserPath()+"/"+fileName);
                    if(!file.open(QFile::WriteOnly))
                    {
                        qDebug()<<"文件"<<fileName<<"打开失败";
                        return;
                    }
                    break;
                }
                case AllHeadPhoto://接收头像
                {
                    file.setFileName(Protocol::getAllUserPath()+"/"+fileName);
                    if(!file.open(QFile::WriteOnly))
                    {
                        qDebug()<<"文件"<<fileName<<"打开失败";
                        return;
                    }
                    break;
                }
                case HistoryMessage://接收历史消息数据
                {

                    QDir dir;
                    //不存在historyMessage文件夹则创建
                    if(!dir.exists(Protocol::getUserPath()+"/historyMessage"))
                        dir.mkdir(Protocol::getUserPath()+"/historyMessage");
                    //不存在对应时间戳文件夹则创建
                    if(!dir.exists(Protocol::getUserPath()+"/historyMessage/"+sendToAccount))
                        dir.mkdir(Protocol::getUserPath()+"/historyMessage/"+sendToAccount);
                    //设置文件路径
                    file.setFileName(Protocol::getUserPath()+"/historyMessage/"+sendToAccount+"/"+fileName);
                    if(!file.open(QFile::WriteOnly))
                    {
                        qDebug()<<"文件"<<fileName<<"打开失败";
                        return;
                    }
                    break;
                }
                case HistoryDynamic:
                {
                    QDir dir;
                    //不存在historyDynamic文件夹则创建
                    if(!dir.exists(Protocol::getUserPath()+"/historyDynamic"))
                        dir.mkdir(Protocol::getUserPath()+"/historyDynamic");
                    //设置文件路径
                    file.setFileName(Protocol::getUserPath()+"/historyDynamic/"+fileName);
                    if(!file.open(QFile::WriteOnly))
                    {
                        qDebug()<<"文件"<<fileName<<"打开失败";
                        return;
                    }
                    break;
                }
                case SearchGroup:
                {
                    file.setFileName(Protocol::getUserPath()+"/searchGroup.json");
                    if(!file.open(QFile::WriteOnly))
                    {
                        qDebug()<<"文件"<<fileName<<"打开失败";
                        return;
                    }
                    break;
                }
                case SearchUser:
                {
                    file.setFileName(Protocol::getUserPath()+"/searchUser.json");
                    if(!file.open(QFile::WriteOnly))
                    {
                        qDebug()<<"文件"<<fileName<<"打开失败";
                        return;
                    }
                    break;
                }
                case AskForGroupData:
                {
                    file.setFileName(Protocol::getUserPath()+"/askForGroupData.json");
                    if(!file.open(QFile::WriteOnly))
                    {
                        qDebug()<<"文件"<<fileName<<"打开失败";
                        return;
                    }
                    break;
                }
                case AskForUserData:
                {
                    file.setFileName(Protocol::getUserPath()+"/askForUserData.json");
                    if(!file.open(QFile::WriteOnly))
                    {
                        qDebug()<<"文件"<<fileName<<"打开失败";
                        return;
                    }
                    break;
                }
                case UpdateFriend:
                case UpdateGroup:
                {
                    file.setFileName(Protocol::getUserPath()+"/"+fileName);
                    if(!file.open(QFile::WriteOnly))
                    {
                        qDebug()<<"文件"<<fileName<<"打开失败";
                        return;
                    }
                    break;
                }
                default:
                    break;
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
        case Reconnection:
        case LoginAccount:
        {
            //-1——登录失败，0——重复登录，1——登录成功
            int result=jsonData.value("result").toString().toInt();
            qDebug()<<"result:"<<result;
            emit loginSuccess(result);
            break;
        }
        case SendMessage:
        {
            QString account=jsonData.value("sender").toString();
            //这一步交给主界面来完成
            //将收到的消息转成json格式
            //QJsonObject message;
            //message.insert("account",account);
            //message.insert("message",jsonData.value("message").toString());
            //message.insert("time",jsonData.value("time").toString());
            //qDebug()<<"tcp收到信息："<<message.value("message").toString();
            //将数据发送给主界面
            emit getMessage(account,jsonData);
            break;
        }
        case HistoryMessage:
        {
            emit getHistoryMessage();
            break;
        }
        case HistoryDynamic:
        {
            emit getHistoryDynamic();
            break;
        }
        case Registration:
        {
            emit registerResult(jsonData.value("result").toString().toInt());
            break;
        }
        case SearchUser:
        case SearchGroup:
        case AskForUserData:
        case AskForGroupData:
        {
            emit getSearchOrAskData(type);
            break;
        }
        case AddFriend:
        {
            emit addFriend(jsonData);
            break;
        }
        case JoinGroup:
        {
            emit joinGroup(jsonData);
            break;
        }
        case SendDynamic:
        {
            emit getDynamic(jsonData);
            break;
        }
        case UpdateFriend:
        {
            emit updateFriend();
            break;
        }
        case UpdateGroup:
        {
            emit updateGroup();
            break;
        }
        case UpdateUserData:
        {
            emit updateUserData();
            break;
        }
        default:
            break;
    }
}

void TcpThread::getJsonData(int type, QString account, QString targetAccount, QString message,QString messageType,QString file)
{
    QString fileName = "";
    int fileNums = -1;
    QString recvaccount = "";
    QJsonObject data;
    data.insert("type", type);
    switch(type)
    {
        case Reconnection:
        case LoginAccount:
        {
            data.insert("account",account);
            data.insert("password",message);
            data.insert("lastLoginTime",QString::number(Protocol::getLastLoginTime()));
            data.insert("loginTime",QString::number(Protocol::getLoginTime()));
            data.insert("remember",Protocol::isRemember);
            data.insert("autoLogin",Protocol::isAutoLogin);
            data.insert("isFirstLogin",Protocol::isFirstLogin);
            break;
        }
        case SendMessage:
        {
            data.insert("time",QString::number(QDateTime::currentMSecsSinceEpoch()));
            data.insert("sender",account);
            data.insert("receiver",targetAccount);
            data.insert("message",message);
            data.insert("messageType",messageType);
            data.insert("file","");
            //data.insert("type",type);
            //qDebug()<<"tcp线程发送："<<message;

            //数据的裁剪交给主界面来进行了
            //因为时间戳在这里生成，所以在这里对数据进行截取存储
            //QJsonObject json;
            //json.insert("time",data.value("time").toString());
            //json.insert("account",account);
            //json.insert("message",message);
            emit getMessage(targetAccount,data);
            //qDebug()<<"tcp已收到数据";
            break;
        }
        case Registration:
        {
            data.insert("id",account);//学号/工号
            data.insert("identity",messageType);//身份
            data.insert("message",message);//除了学号/工号和身份外的所有信息
            break;
        }
        case SearchUser:
        case SearchGroup:
        case AskForUserData:
        case AskForGroupData:
        {
            data.insert("account",account);
            data.insert("message",message);
            break;
        }
        case AddFriend:
        case JoinGroup:
        {
            data.insert("sender",account);
            data.insert("receiver",targetAccount);
            data.insert("message",message);//内容：apply或者agree
            data.insert("messageType",messageType);//群号(如果有的话)
            data.insert("time",QString::number(QDateTime::currentMSecsSinceEpoch()));
            data.insert("file","");
            break;
        }
        case SendDynamic:
        {
            data.insert("sender",account);
            data.insert("receiver","");
            data.insert("message",message);//动态内容
            data.insert("messageType","");
            data.insert("time",QString::number(QDateTime::currentMSecsSinceEpoch()));
            data.insert("file","");

            //截取数据，发回给主界面
            QJsonObject json;
            json.insert("type",type);
            json.insert("account",account);
            json.insert("userName",Protocol::getUserName());
            json.insert("headPhoto",Protocol::getUserHeadShot());
            json.insert("message",message);
            json.insert("time",data.value("time").toString());
            getDynamic(json);
            break;
        }
        case CreateGroup:
        {
            data.insert("sender",account);
            data.insert("receiver",targetAccount);
            data.insert("message",message);//动态内容
            data.insert("messageType","");
            data.insert("time",QString::number(QDateTime::currentMSecsSinceEpoch()));
            data.insert("file","");
            break;
        }
        case ModifyUserData:
        {
            //数据顺序：用户名-》个性签名-》性别-》年龄-》生日-》血型-》位置-》学院-》专业(只有学生用户有专业信息)
            QStringList list=message.split(";");
            data.insert("account",account);
            data.insert("userName",list[0]);
            data.insert("signature",list[1]);
            data.insert("sex",list[2]);
            data.insert("age",list[3].toInt());
            data.insert("birthday",list[4]);
            data.insert("bloodType",list[5]);
            data.insert("location",list[6]);
            data.insert("college",list[7]);
            if(account[0]=='E')
                data.insert("profession",list[8]);
            data.insert("remember",Protocol::isRemember);
            data.insert("autoLogin",Protocol::isAutoLogin);
            data.insert("updateTime",QString::number(QDateTime::currentMSecsSinceEpoch()));
            data.insert("headPhoto",file.split("/").last());
            fileName=file;
            break;
        }
        case ModifyGroupData:
        {
            QStringList list=message.split(";");
            data.insert("account",account);
            data.insert("userName",list[0]);
            data.insert("signature",list[1]);
            data.insert("updateTime",QString::number(QDateTime::currentMSecsSinceEpoch()));
            data.insert("headPhoto",file.split("/").last());
            fileName=file;
            break;
        }
        default:
            break;
    }
    QJsonDocument document(data);
    sendToServer(document.toJson(),fileName,type,fileNums,recvaccount);
}
