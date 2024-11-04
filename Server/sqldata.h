#ifndef SQLDATA_H
#define SQLDATA_H

#include <QObject>
#include <QtSql>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <QDebug>
#include <QVector>
#include "protocol.h"
#include <QtGlobal>

class SqlData:public QObject
{
    Q_OBJECT
public:
    SqlData();
    void connectDataBase();
    bool judgeUserExist(QString account,QString password);//判断该用户是否存在

    //处理同步问题
    bool judgeUpdateTimeOfRelation(QString account,qint64 lastLoginTime);//判断relation表是否已同步
    bool judgeUpdateTimeOfUser(QString account,qint64 lastLoginTime);//判断用户信息是否同步
    bool judgeLastLoginTimeOfUser(QString account,qint64 lastLoginTime);//判断历史消息是否同步
    QJsonObject getUserDateFile(QString account,bool remember,bool autoLogin);//在数据库中获取用户数据文件信息
    QJsonArray getFriendDataFile(QString account);//在数据库中获取好友列表数据文件信息
    QByteArray getGroupDataFile(QString account,QString& headPhoto);//获取用户的所有群聊信息
    QStringList getAllFriendHeadPhoto(QString account);//获取所有好友头像名单(只有文件名，没有路径，需要自己添加)

    //一些常用的获取单项用户数据的方法
    QString getUserHeadPhoto(QString account);//获取用户头像
    qint64 getUserLoginTime(QString account);//获取用户登录时间
    QString getUserName(QString account);//获取用户昵称
    QString getGroupName(QString account);//获取群名称
    QStringList getAllFriend(QString account);//获取所有好友
    QStringList getAllMember(QString account);//获取所有群成员
    QString getGroupHeadPhoto(QString account);//获取群头像
    QString getGroupHostAccount(QString account);//获取群主账号

    //搜索相关
    QByteArray searchUser(QString condition,QString& headPhoto);//搜索用户
    QByteArray searchGroup(QString condition,QString& headPhoto);//搜索群聊
    QJsonObject askForUserData(QString account,QString& headPhoto);//请求用户信息
    QJsonObject askForGroupData(QString account,QString& headPhoto);//请求群聊信息

    //用户状态
    bool isUserOnLine(QString account);//判断是否在线
    void userUpLine(QString account,qint64 loginTime);//用户上线设置
    void userDownLine(QString account);//用户下线设置
    void closeDataBase();//关闭所有用户状态

    //添加好友/加入群聊
    void addFriend(QString account_1,QString account_2,qint64 updateTime);//添加好友
    void joinGroup(QString account,QString group,qint64 updateTime);//加入群聊
    QJsonObject taskToAddFriend(QJsonObject json);//任务json格式转成客户端能读懂的addFriend格式
    QJsonObject taskToJoinGroup(QJsonObject json);//任务json格式转成客户端能读懂的joinGroup格式
    void setRelationUpdateTime(QString account,qint64 updateTime);//更新用户的联系人表更新时间,用于通知用户联系人信息发生变化

    //发布动态
    QJsonObject taskToSendDynamic(QJsonObject json);//任务json格式转成客户端能读懂的sendDynamic格式

    //创建群聊
    bool isGroupExist(QString account);
    QString createGroup(QString groupName,QString hostAccount,qint64 time);

    //修改用户/群聊信息
    void modifyUserData(QString account,QJsonObject json);
    void modifyGroupData(QString account,QJsonObject json);

    //用户注册
    bool judgeUserRegistrationMessage(QString account,QString identity,QString message);//判断用户信息是否匹配
    bool userExist(QString account);//判断用户账号是否已存在
    void userRegister(QString account,QString password);//添加新用户信息

    //任务缓存
    bool addMessageTask(QJsonObject json);//添加消息任务
    bool deleteMessageTask(QString account);//删除消息任务

    //任务转发
    QVector<QJsonObject> sendMessageTask(QString account);//转发接收者为该账号的任务

    ~SqlData();
private:
    QSqlDatabase db;
};

#endif // SQLDATA_H
