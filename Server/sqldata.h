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

class SqlData:public QObject
{
    Q_OBJECT
public:
    SqlData();
    void connectDataBase();
    bool searchUser(QString account,QString password);//判断该用户是否存在

    //处理同步问题
    bool judgeUpdateTimeOfRelation(QString account,qint64 lastLoginTime);//判断relation表是否已同步
    bool judgeUpdateTimeOfUser(QString account,qint64 lastLoginTime);//判断用户信息是否同步
    bool judgeLastLoginTimeOfUser(QString account,qint64 lastLoginTime);//判断历史消息是否同步
    QJsonObject getUserDateFile(QString account,bool remember,bool autoLogin);//在数据库中获取用户数据文件信息
    QJsonArray getFriendDataFile(QString account);//在数据库中获取好友列表数据文件信息
    QStringList getAllFriendHeadPhoto(QString account);

    //一些常用的获取单项用户数据的方法
    QString getUserHeadPhoto(QString account);//获取用户头像
    qint64 getUserLoginTime(QString account);//获取用户登录时间

    //用户状态
    bool isUserOnLine(QString account);//判断是否在线
    void userUpLine(QString account,qint64 loginTime);//用户上线设置
    void userDownLine(QString account);//用户下线设置
    void closeDataBase();//关闭所有用户状态

    //任务缓存
    bool addMessageTask(QJsonObject json);//添加消息任务
    bool deleteMessageTask(QString account);//删除消息任务

    //用户注册
    bool judgeUserRegistrationMessage(QString account,QString identity,QString message);//判断用户信息是否匹配
    bool userExist(QString account);//判断用户账号是否已存在
    void userRegister(QString account,QString password);//添加新用户信息

    //任务转发
    QVector<QJsonObject> sendMessageTask(QString account);//转发接收者为该账号的任务

    ~SqlData();
private:
    QSqlDatabase db;
};

#endif // SQLDATA_H
