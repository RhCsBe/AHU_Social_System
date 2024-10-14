#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QString>
#include <QPainterPath>
#include <QPixmap>
#include <QDir>
#include <QDebug>
#include <QPainter>
#include <QJsonObject>

#define BufferSize 1024//数据包大小
#define HeadSize 10//包头大小
#define NoHeadBufferSize 1014//数据包数据大小
#define DefalutPixmap ":/photo/AHU1.png"

typedef struct{
    QString headPhoto;
    QString userName;
    QString account;
    QString message;
    qint64 time;
    int sum;
} MessageItem;
Q_DECLARE_METATYPE(MessageItem)//注册自定义结构体

//包头类型
enum FileType
{
    JsonDataHead = 50,
    FileInfoHead,
    FileDataHead,
    FileEndDataHead
};

//Json数据中信息类型
enum InfoType
{
    Registration = 1125,
    FindPassword,
    LoginAccount,
    SearchFriend,
    AddFriend,
    ChangeOnlineStatus,
    SendMessage,
    AskForData,
    UserChangeData,
    UpdateHeadPhoto,
    SendFileToFriend,
    AllHeadPhoto
};

//保存用户资料编号
enum UserMsg
{
    UserName = 0,
    Signature,
    Sex,
    Age,
    Birthday,
    Location,
    BloodType,
    College,
    Profession
};

class Protocol
{
public:

    /* 用户数据文件夹目录 */
    static void setWorkPath(QString path); //获取工作目录用户数据文件夹
    static QString getWorkPath(); //返回工作目录
    static QString getAllUserPath(); //返回好友头像目录
    static bool createWorkPath(); //创建工作目录用户数据文件夹

    //设置用户信息
    static void initLoginUserInfo(QString account, QString pwd,qint64 lastLoginTime_temp, qint64 loginTime_temp); //设置用户登录时数据
    static void initUserInfo(QJsonObject json);//设置当前用户信息

    /* 登录用户数据文件夹 */
    static QString getUserPath(); //返回登录用户文件夹位置
    static QString getUserLoginFile(); //返回用户登录初始化文件
    static QString getUserFileRecvPath(); //返回登录用户文件接收文件夹位置
    static QString getUserHeadShot(); //登录用户头像位置

    /* 登录用户信息 */
    static QString getUserAccount(); //返回登录用户账号
    static QString getUserPwd(); //返回登录用户密码
    static QString getUserName(); //返回用户昵称
    static QString getSignature(); //返回用户个签
    static QString getSex();//返回用户性别
    static qint64 getLastLoginTime();//返回上次登录时间
    static qint64 getLoginTime();//返回这次登录时间
    static int getAge();//返回用户年龄
    static QString getBirthday();//返回用户生日
    static QString getLocation();//返回用户地址
    static QString getBloodType();//返回用户血型
    static QString getCollege();//返回用户学院
    static QString getProfession();//返回用户专业

    static QPixmap createHeadShot(QString pixPath,int radius); //返回无锯齿圆形头像
    static QString isFileExist(QString filepath); //查看文件是否存在，不存在直接返回，存在则添加后缀

    static QString scrollbarStyle;

    static bool isFirstLogin; //是否是第一次登录
    static bool isRemember; //是否记住密码
    static bool isAutoLogin; //是否自动登录
    static bool isConnecting; //是否已连接到服务器
private:
    Protocol();//不允许构造实例
    static QString workPath; //工作目录下用户数据文件夹
    static QString allUserPath; //好友头像文件夹

    static QString loginUserPath; //登录账号用户文件夹
    static QString loginUserFileRecvPath; //登录账号用户文件存放文件夹

    static QString userAccount; //登录账号
    static QString userPwd; //登录密码
    static QString userHeadShot; //登录账号头像
    static QString userName; //昵称
    static QString signature; //个性签名
    static QString sex;//性别
    static int age;//年龄
    static QString birthday;//生日
    static QString location;//地址
    static QString bloodType;//血型
    static QString college;//学院
    static QString profession;//专业
    static qint64 lastLoginTime;//上次登录时间
    static qint64 loginTime;//登录时间
};

#endif // PROTOCOL_H
