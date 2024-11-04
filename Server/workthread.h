#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include <QObject>
#include "sqldata.h"
#include "protocol.h"
#include <QFile>
#include <QHash>
#include <QTimer>
#include <algorithm>
#include <QStringList>

class WorkThread : public QObject
{
    Q_OBJECT
public:
    explicit WorkThread(QObject *parent = nullptr);

    //文件传输和接收以及解析
    //合并数据包
    void mergeDataPackage(QString key,QByteArray dataArray);
    //解析json数据包
    void parseMessage(QString key,QByteArray dataArray);
    //将回复信息转换成json格式
    void replyJson(QString key,int type, QString message, QString messageType = "", QString account = "", QString targetAccount = "", QString fileName = "");
    //创建或更新文件
    void updateFile(QString fileName,QByteArray data);

    //消息存储相关
    void openMessageFile(QString sender,QString receiver);//创建消息缓冲（发送者，接收者）
    void addMessage(QString sender,QString receiver,QJsonObject json);//将json数据添加进消息缓存
    QJsonObject messageTaskToMessage(QJsonObject data);//将收到的messageTask转成可以存储的message

    //多次发送
    void sendGroupMessage(QJsonObject json);//群发消息
    void sendDynamic(QJsonObject json);//发布动态

    //用户上线/下线
    void sendMessageFile(QString account,qint64 lastLoginTime);//发送消息文件，即客户端同步服务器数据
    void sendDynamicFile(QString account,qint64 lastLoginTime);//发送动态文件，同上
    void sendMessageTask(QString account);//发送用户未收到的消息
    //void userUpLine(QString account,qint64 loginTime,qint64 lastLoginTime,bool remember,bool autoLogin,bool firstLogin);
    void userDownLine(QString account);//用户下线设置

    //用户注册
    void userRegister(QString account,QString password);//用户注册

signals:
    //将数据发回给服务器
    void sendToServer(QString key, int type, QString account, QString targetAccount, QByteArray jsonData, QString messageType, QString fileName);
    void myInformation(QString str);
    void addUserOnline(QString account,QString key);
private:
    SqlData sql;
    QHash<QString,QFile*> fileArray;
    QHash<QString,QPair<int,int>> fileSize;
    QHash<QString,QHash<QString,QJsonArray*>> messageArray;
    QHash<QString,QJsonArray*>dynamicArray;

    //定时器发送缓存的任务
    //QTimer timer;
};

#endif // WORKTHREAD_H
