#ifndef SQLDATA_H
#define SQLDATA_H

#include <QObject>
#include <QtSql>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>

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
    QJsonObject getUserDateFile(QString account,bool remember,bool autoLogin);//在数据库中获取用户数据文件信息
    QJsonArray getFriendDataFile(QString account);//在数据库中获取好友列表数据文件信息
    QStringList getAllFriendHeadPhoto(QString account);
    QString getUserHeadPhoto(QString account);

private:
    QSqlDatabase db;
};

#endif // SQLDATA_H
