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
        case LoginAccount:
        {
            QString account=jsonData.value("account").toString();
            QString password=jsonData.value("password").toString();
            qint64 lastLoginTime=jsonData.value("lastLoginTime").toString().toLongLong();
            qint64 loginTime=jsonData.value("loginTime").toString().toLongLong();
            bool remember=jsonData.value("remember").toBool();
            bool autoLogin=jsonData.value("autoLogin").toBool();
            bool firstLogin=jsonData.value("firstLogin").toBool();
            qDebug()<<account<<"    "<<password;
            if(sql.searchUser(account,password))
            {
                QString fileName="";
                QString headPhoto="";
                //判断用户信息是否过时，过时则同步
                if(!sql.judgeUpdateTimeOfUser(account,lastLoginTime))
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
                //判断联系人信息是否过时
                if(!sql.judgeUpdateTimeOfRelation(account,lastLoginTime))
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
                //文件名单不为空去掉末尾的问号,并发送信号
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
                //QMessageBox::information(nullptr,"提示","登录成功");
            }
            else
            {
                replyJson(key,InfoType(type),"账号或密码错误");
                emit myInformation("登录失败");
                //QMessageBox::information(nullptr,"提示","登录失败");
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
        case LoginAccount:
        {
            jsonData.insert("result",message);
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
