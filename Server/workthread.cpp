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
    switch(type)
    {
        case LoginAccount:
        {
            QString account=jsonData.value("account").toString();
            QString password=jsonData.value("password").toString();
            qDebug()<<account<<"    "<<password;
            if(sql.searchUser(account,password))
            {
                replyJson(key,InfoType(type),"登录成功");
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
