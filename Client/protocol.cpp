#include "protocol.h"


bool Protocol::isFirstLogin = false;
bool Protocol::isRemember = false;
bool Protocol::isAutoLogin =false;
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
QString Protocol::sex="";
int Protocol::age=0;
QString Protocol::birthday="";
QString Protocol::location="";
QString Protocol::bloodType="";
QString Protocol::college="";
QString Protocol::profession=0;
qint64 Protocol::lastLoginTime=0;
qint64 Protocol::loginTime=0;
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

void Protocol::initLoginUserInfo(QString account, QString pwd,qint64 lastLoginTime_temp, qint64 loginTime_temp)
{
    loginUserPath = workPath + "/" + account;
    loginUserFileRecvPath = loginUserPath + "/FileRecv";

    userAccount = account;
    userPwd = pwd;
    lastLoginTime=lastLoginTime_temp;
    loginTime=loginTime_temp;
    //userHeadShot = loginUserPath + "/" + account + ".jpg";
}

void Protocol::initUserInfo(QJsonObject json)
{
    //设置登录的用户数据
    if(json.value("headPhoto").toString()=="")
        userHeadShot=DefalutPixmap;
    else
        userHeadShot=allUserPath+"/"+json.value("headPhoto").toString(); //登录账号头像
    userName=json.value("userName").toString(); //昵称
    signature=json.value("signature").toString(); //个性签名
    sex=json.value("sex").toString();//性别
    age=json.value("age").toInt();//年龄
    birthday=json.value("birthday").toString();//生日
    location=json.value("location").toString();//地址
    bloodType=json.value("bloodType").toString();//血型
    college=json.value("college").toString();//学院
    profession=json.value("profession").toString();//专业
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

QString Protocol::getSex()
{
    return sex;
}

qint64 Protocol::getLastLoginTime()
{
    return lastLoginTime;
}

qint64 Protocol::getLoginTime()
{
    return loginTime;
}

int Protocol::getAge()
{
    return age;
}

QString Protocol::getBirthday()
{
    return birthday;
}

QString Protocol::getLocation()
{
    return location;
}

QString Protocol::getBloodType()
{
    return bloodType;
}

QString Protocol::getCollege()
{
    return college;
}

QString Protocol::getProfession()
{
    return profession;
}

QPixmap Protocol::createHeadShot(QString pixPath, int radius)
{
    //因为给QLabel设置圆角无法对设置的图片直接进行裁剪显示，还是会显示原图，达不到圆形头像的效果，所以不能直接等比缩放显示
    //ui->head_photo->setPixmap(QPixmap(str).scaled(ui->head_photo->size().width(),ui->head_photo->size().height(), Qt::KeepAspectRatio,Qt::SmoothTransformation));

    //第一种方式：缩放->裁剪->重绘

    //等比缩放原图
    QPixmap src=QPixmap(pixPath).scaled(radius*2,radius*2, Qt::KeepAspectRatio,Qt::SmoothTransformation);

    //新建一个空白画布
    QPixmap result(radius*2,radius*2);
    result.fill(Qt::transparent);//填充透明背景

    //将空白画布设置为画笔的绘图背景
    QPainter painter(&result);
    painter.setRenderHints(QPainter::Antialiasing);//抗锯齿
    painter.setRenderHints(QPainter::SmoothPixmapTransform);//平滑像素图变换

    //设置绘制路径，并添加到画笔中
    QPainterPath path;
    path.addRoundedRect(0,0,radius*2,radius*2,radius,radius);
    painter.setClipPath(path);
    //绘制成果图
    painter.drawPixmap(0,0,radius*2,radius*2,src);
    painter.setClipping(false);//关闭裁剪
    //返回头像
    return result;


    //第二种方法：创建透明遮罩
    //效果很奇怪，显示安大校徽时正常，但是显示路飞头像时不知道为什么把透明的是中心区域
    //    QPixmap pixmap(str);
    //    QBitmap mask(ui->head_photo->size());
    //    mask.fill(Qt::transparent);
    //    QPainter painter(&mask);
    //    painter.setRenderHint(QPainter::Antialiasing);
    //    painter.setBrush(Qt::white);
    //    int radius=ui->head_photo->rect().x()/2;
    //    painter.drawRoundedRect(ui->head_photo->rect(),20, 20);
    //    painter.end();

    //    // 应用遮罩到图片
    //    QPixmap roundedPixmap = pixmap.scaled(ui->head_photo->size(), Qt::KeepAspectRatio,Qt::SmoothTransformation);
    //    roundedPixmap.setMask(mask);

    //    // 设置遮罩后的图片到 QLabel
    //    ui->head_photo->setPixmap(roundedPixmap);
}


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






