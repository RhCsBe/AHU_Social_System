#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include <QObject>
#include "sqldata.h"
#include "protocol.h"
#include <QFile>
#include <QHash>

class WorkThread : public QObject
{
    Q_OBJECT
public:
    explicit WorkThread(QObject *parent = nullptr);
    //合并数据包
    void mergeDataPackage(QString key,QByteArray dataArray);
    //解析json数据包
    void parseMessage(QString key,QByteArray dataArray);
    //将回复信息转换成json格式
    void replyJson(QString key,InfoType type, QString message, QString messageType = "", QString account = "", QString targetAccount = "", QString fileName = "");
    //创建或更新文件
    void updateFile(QString fileName,QByteArray data);
signals:
    //将数据发回给服务器
    void sendToServer(QString key, int type, QString account, QString targetAccount, QByteArray jsonData, QString messageType, QString fileName);
    void myInformation(QString str);
private:
    SqlData sql;
    QHash<QString,QFile*> fileArray;
    QHash<QString,QPair<int,int>> fileSize;
};

#endif // WORKTHREAD_H
