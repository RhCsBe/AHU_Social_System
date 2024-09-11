#include "protocol.h"


bool Protocol::isFirstLogin = false;
bool Protocol::isRemember = false;
bool Protocol::isConnecting = false;
QString Protocol::workPath = "";
QString Protocol::allUserPath = "";

QString Protocol::loginUserPath = "";
QString Protocol::loginUserFileRecvPath = "";

QString Protocol::userAccount = "";
QString Protocol::userPwd = "";
QString Protocol::userHeadShot = "";
QString Protocol::userName = "";
QString Protocol::signature = "";
QString Protocol::scrollbarStyle = "QScrollBar{width:8px;background:white;margin:0px,0px,0px,0px;}"
                                   "QScrollBar::handle{width:8px;background:rgb(235,235,235);border-radius:4px;height:20px;}"
                                   "QScrollBar::handle:hover{width:8px;background:rgb(139,139,139);border-radius:4px;height:20px;}";


Protocol::Protocol()
{

}

void Protocol::setWorkPath(QString path)
{
    workPath=path;
    allUserPath=path+"/allUser";
}

QString Protocol::getWorkPath()
{
    return workPath;
}

QString Protocol::getAllUserPath()
{
    return allUserPath;
}

bool Protocol::createWorkPath()
{
    QDir dir;

    if(!dir.exists(workPath))
    {
        qDebug() << "未创建用户数据文件夹";

        if(!dir.mkdir(workPath))
        {
            qDebug() << "创建用户数据文件夹失败";
            return false;
        }
    }

    if(!dir.exists(allUserPath))
    {
        qDebug() << "allusers数据文件夹未创建";

        if(!dir.mkdir(allUserPath))
        {
            qDebug() << "allusers数据文件夹创建失败!";
            return false;
        }
    }

    return true;
}

void Protocol::initLoginUserInfo(QString account, QString pwd)
{
    loginUserPath = workPath + "/" + account;
    loginUserFileRecvPath = loginUserPath + "/FileRecv";

    userAccount = account;
    userPwd = pwd;
    userHeadShot = loginUserPath + "/" + account + ".jpg";
}

void Protocol::initUserNameAndSig(QString name, QString sig)
{
    userName=name;
    signature=sig;
}

QString Protocol::getUserPath()
{
    return loginUserPath;
}

QString Protocol::getUserLoginFile()
{
    return loginUserPath + "/login.json";
}

QString Protocol::getUserFileRecvPath()
{
    return loginUserFileRecvPath;
}

QString Protocol::getUserHeadShot()
{
    return userHeadShot;
}

QString Protocol::getUserAccount()
{
    return userAccount;
}

QString Protocol::getUserPwd()
{
    return userPwd;
}

QString Protocol::getUserName()
{
    return userName;
}

QString Protocol::getSignature()
{
    return signature;
}

//QPixmap Protocol::createHeadShot(QString pixPath)
//{

//}

QString Protocol::isFileExist(QString filePath)
{
    QFileInfo fileInfo;
    if(fileInfo.exists(filePath))
    {
        int num=1;
        QString firstName = filePath.split(".").first();
        QString lastName = filePath.split(".").last();

        while(true)
        {
            QString newFileName = firstName + "_" + QString::number(num) + lastName;
            if(!fileInfo.exists(newFileName))
            {
                return newFileName;
            }
            num++;
        }
    }
    else
    {
        return filePath;
    }
}






