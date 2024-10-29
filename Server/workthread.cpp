#include "workthread.h"

WorkThread::WorkThread(QObject *parent)
    : QObject{parent}
{

}

void WorkThread::mergeDataPackage(QString key, QByteArray dataArray)
{
    QDataStream fileHead(&dataArray,QFile::ReadOnly);
    FileType fileType;
    qint16 space;
    int fileSize;
    fileHead>>fileType>>space>>fileSize;
    dataArray.remove(0,10);
    QFile* file=nullptr;

    switch (fileType)
    {
    case JsonDataHead:
        parseMessage(key,dataArray.left(fileSize));
        break;
    case FileInfoHead:
        {
            int type,fileSize;
            QString fileName,sendToAccount;
            fileHead>>type>>fileSize>>fileName>>sendToAccount;
            this->fileSize.insert(key,QPair(0,fileSize));
            switch(type)
            {

            }
        }
        break;
    case FileDataHead:
    {

    }
        break;
    case FileEndDataHead:
    {

    }
        break;
    default:
        break;
    }
}

void WorkThread::parseMessage(QString key, QByteArray dataArray)
{
    QJsonObject jsonData=QJsonDocument::fromJson(dataArray).object();
    int type=jsonData.value("type").toInt();
    //qDebug()<<"type:"<<type;
    switch(type)
    {
        case LoginAccount://用户登录
        {
            //读取登录信息
            QString account=jsonData.value("account").toString();
            QString password=jsonData.value("password").toString();
            qint64 lastLoginTime=jsonData.value("lastLoginTime").toString().toLongLong();
            qint64 loginTime=jsonData.value("loginTime").toString().toLongLong();
            bool remember=jsonData.value("remember").toBool();
            bool autoLogin=jsonData.value("autoLogin").toBool();
            bool firstLogin=jsonData.value("firstLogin").toBool();
            qDebug()<<account<<"    "<<password;

            //验证登录信息
            if(sql.searchUser(account,password))
            {
                QString fileName="";
                QString headPhoto="";
                //判断是否第一次登录，或者用户信息是否过时，过时则同步
                if(firstLogin || !sql.judgeUpdateTimeOfUser(account,lastLoginTime))
                {
                    //判断是否发送login文件
                    qDebug()<<"用户信息过时";
                    QString temp=Protocol::getWorkPath()+"/"+account+"/login.json";
                    //判断是否存在login文件
                    if(temp==Protocol::isFileExist(temp))
                    {
                        //没有则创建
                        updateFile(temp,QJsonDocument(sql.getUserDateFile(account,remember,autoLogin)).toJson());
                    }
                    //添加进发送文件的名单
                    fileName+=temp+"?";

                    //判断是否发送用户头像
                    QString str=sql.getUserHeadPhoto(account);
                    if(str!=""&&Protocol::isFileExist("allUser",str))
                    {
                        headPhoto+=Protocol::getAllUserPath()+"/"+str+"?";
                    }
                }
                //判断是否第一次登录，联系人信息是否过时
                if(firstLogin || !sql.judgeUpdateTimeOfRelation(account,lastLoginTime))
                {
                    //判断是否发送friend文件
                    qDebug()<<"联系人信息过时";
                    QString temp=Protocol::getWorkPath()+"/"+account+"/friend.json";
                    if(temp==Protocol::isFileExist(temp))
                    {
                        updateFile(temp,QJsonDocument(sql.getFriendDataFile(account)).toJson());
                    }
                    fileName+=temp+"?";

                    //判断是否发送好友头像
                    QStringList list=sql.getAllFriendHeadPhoto(account);
                    for(auto i:list)
                    {
                        if(i!=""&&Protocol::isFileExist("allUser",i))
                        {
                            headPhoto+=Protocol::getAllUserPath()+"/"+i+"?";
                        }
                    }
                }
                //头像名单不为空去掉末尾的问号,并发送信号
                if(headPhoto!="")
                {
                    headPhoto.chop(1);
                    emit sendToServer(key,InfoType::AllHeadPhoto,"",account,QByteArray(),"",headPhoto);
                }

                //文件名单不为空去掉末尾的问号
                if(fileName!="")
                {
                    fileName.remove(fileName.length()-1,1);
                }
                qDebug()<<fileName;
                replyJson(key,InfoType(type),"登录成功","","",account,fileName);
                emit myInformation("登录成功");

                //将该用户套接字添加进在线用户套接字映射表
                emit addUserOnline(account,key);
                //添加消息记录映射表
                messageArray.insert(account,QHash<QString,QJsonArray*>());
                sendMessageFile(account,lastLoginTime);
                //设置用户状态
                sql.userUpLine(account,loginTime);

                //1.5秒后发送存储在数据库的消息任务
                QTimer::singleShot(1500,this,[&](){sendMessageTask(account);});
            }
            else
            {
                replyJson(key,InfoType(type),"账号或密码错误");
                emit myInformation("登录失败");
                //QMessageBox::information(nullptr,"提示","登录失败");
            }
            break;
        }
        case SendMessage://发送消息
        {
            //qDebug()<<"接收到发来的信息："<<jsonData.value("message").toString();
            QString sender=jsonData.value("sender").toString();
            QString receiver=jsonData.value("receiver").toString();

            //先将messageTask转成message，去除多余信息
            QJsonObject json=messageTaskToMessage(jsonData);
            //将消息添加进发送者的消息缓存中
            addMessage(sender,receiver,json);
            //qDebug()<<"消息缓存完毕";

            //判断接收者是否在线
            if(sql.isUserOnLine(receiver))
            {
                //在线转发并存储在接收者的消息缓存中
                sendToServer(key,type,sender,receiver,QJsonDocument(jsonData).toJson(),"","");

                addMessage(receiver,sender,json);
            }
            else
            {
                //不在则将消息转成任务存储到mysql任务表中，等待接收者上线后转发
                sql.addMessageTask(jsonData);
            }
            break;
        }
        case Registration:
        {
            //读取json信息
            QString id=jsonData.value("id").toString();
            QString identity=jsonData.value("identity").toString();
            QString message=jsonData.value("message").toString();
            //判断注册信息是否正确
            if(sql.judgeUserRegistrationMessage(id,identity,message))
            {
                //判断用户是否已存在
                if(sql.userExist(id))
                {
                    replyJson(key,InfoType::Registration,QString::number(0));//账号已存在
                }
                else
                {
                    //往数据库写入新用户信息,并创建用户数据文件夹
                    userRegister(id,message.split("\n").at(1));
                    replyJson(key,InfoType::Registration,QString::number(1));//注册成功
                }
            }
            else
            {
                replyJson(key,InfoType::Registration,QString::number(-1));//信息错误，注册失败
            }
            break;
        }
        default:
            break;
    }
}

void WorkThread::replyJson(QString key, InfoType type, QString message, QString messageType, QString account, QString targetAccount, QString fileName)
{
    QJsonObject jsonData;
    jsonData.insert("type",type);
    switch(type)
    {
        case Registration:
        case LoginAccount:
        {
            jsonData.insert("result",message);
            break;
        }
        default:
            break;
    }
    QJsonDocument jsonDocument(jsonData);
    QByteArray dataArray=jsonDocument.toJson();
    emit sendToServer(key,type,account,targetAccount,dataArray,messageType,fileName);
}

void WorkThread::updateFile(QString fileName, QByteArray data)
{
    QFile file(fileName);
    if(file.open(QFile::WriteOnly))
    {
        file.write(data);
        file.close();
    }
}

void WorkThread::openMessageFile(QString sender, QString receiver)
{
    QJsonArray* json=new QJsonArray();
    messageArray[sender].insert(receiver,json);
}

void WorkThread::addMessage(QString sender,QString receiver,QJsonObject json)
{
    //不存在对应的消息记录映射表则创建
    if(!messageArray.value(sender).contains(receiver))
        openMessageFile(sender,receiver);
    //添加消息
    messageArray.value(sender).value(receiver)->append(json);
}

QJsonObject WorkThread::messageTaskToMessage(QJsonObject json)
{
    QJsonObject message;
    message.insert("account",json.value("sender").toString());
    message.insert("message",json.value("message").toString());
    message.insert("time",json.value("time").toString());
    return message;
}

void WorkThread::sendMessageFile(QString account, qint64 lastLoginTime)
{
    if(!sql.judgeLastLoginTimeOfUser(account,lastLoginTime))
    {
        QDir dir;
        dir.setPath(Protocol::getWorkPath()+"/"+account+"/message");
        QStringList message=dir.entryList(QDir::Dirs);
        message.removeOne(".");
        message.removeOne("..");
        QVector<qint64> timeSort;
        for(QString i:message)
        {
            timeSort.push_back(i.toLongLong());
        }
        std::sort(timeSort.begin(),timeSort.end());
        for(qint64 num:timeSort)
        {
            if(num<=lastLoginTime)
                continue;
            else
            {
                QDir dir;
                dir.setPath(Protocol::getWorkPath()+"/"+account+"/message/"+QString::number(num));
                QStringList allFile=dir.entryList(QDir::Files);
                allFile.removeOne(".");
                allFile.removeOne("..");
                QString files="";
                for(QString file:allFile)
                {
                    files+=dir.path()+"/"+file;
                    files+="?";
                }
                files.chop(1);
                emit sendToServer("",InfoType::HistoryMessage,"",account,QByteArray(),QString::number(num),files);
            }
        }
        //发送完所有历史消息文件后发送一个json数据包通知客户端读取历史消息数据
        QJsonObject json;
        json.insert("type",InfoType::HistoryMessage);
        emit sendToServer("",InfoType::HistoryMessage,"",account,QJsonDocument(json).toJson(),"","");
    }
}

void WorkThread::sendMessageTask(QString account)
{
    QVector<QJsonObject> array=sql.sendMessageTask(account);
    //遍历发送消息
    for(QJsonObject json: array)
    {
        QString sender=json.value("sender").toString();
        QString receiver=json.value("receiver").toString();
        int type=json.value("type").toInt();
        emit sendToServer("",type,sender,receiver,QJsonDocument(json).toJson(),"","");
        //存储到消息缓存中,记得先将messageTask转成message再存储
        addMessage(receiver,sender,messageTaskToMessage(json));
    }
}



void WorkThread::userDownLine(QString account)
{
    //先将用户的状态设置为下线
    sql.userDownLine(account);

    //然后根据用户登录时间查看是否有消息需要存储
    QString loginTime;
    qint64 temp=sql.getUserLoginTime(account);
    //temp小于0代表sql出现错误，直接返回
    if(temp<0)
        return;
    else
        loginTime=QString::number(temp);
    //查看是否有消息通信
    if(messageArray.value(account).size()>0)
    {
        //以登录时间戳作为文件夹名创建
        QDir dir;
        QString path=Protocol::getWorkPath()+"/"+account+"/message/"+loginTime;
        dir.mkdir(path);
        qDebug()<<path;
        //遍历写入所有与好友的通信消息，文件名：好友账号.json
        for(QString friendAccount:messageArray[account].keys())
        {
            QFile file(path+"/"+friendAccount+".json");
            file.open(QFile::WriteOnly|QFile::Truncate);
            file.write(QJsonDocument(*(messageArray[account].value(friendAccount))).toJson());
            file.close();
        }
    }
}

void WorkThread::userRegister(QString account, QString password)
{
    //往数据库写入新用户信息
    sql.userRegister(account,password);

    //创建用户文件夹
    QDir dir;
    dir.setPath(Protocol::getWorkPath());
    if(!dir.exists(dir.path()+"/"+account))
    {
        if(!dir.mkdir(dir.path()+"/"+account))
        {
            qDebug()<<"创建用户"<<account<<"文件夹失败";
        }
        else
        {
            if(!dir.mkdir(dir.path()+"/"+account+"/message"))
                qDebug()<<"创建用户"<<account<<"文件夹下的message文件夹失败";
            if(!dir.mkdir(dir.path()+"/"+account+"/FileRecv"))
                qDebug()<<"创建用户"<<account<<"文件夹下的FileRecv文件夹失败";
        }
    }
}

