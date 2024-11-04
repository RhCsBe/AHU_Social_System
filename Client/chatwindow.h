#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include <QFontMetrics>
#include <QDebug>
#include <QMouseEvent>
#include <QStandardItemModel>
#include <QStandardItem>
#include "chatwindowdelegate.h"
#include <ElaMessageBar.h>
#include <QScrollBar>
#include <QScreen>

enum ChatWindowType{
    FriendChatWindow=0,
    GroupChatWIndow
};

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWindow(QWidget *parent = nullptr);
    ~ChatWindow();

    //一些对象初始化设置
    void setStyle();//设置一些固定的ui样式
    void setConnect();//绑定可见控件的信号和槽函数
    void setView();//设置视图


    //实现拖拽效果
    void mousePressEvent(QMouseEvent* event);//重写鼠标按压事件
    void mouseReleaseEvent(QMouseEvent* event);//重写鼠标释放事件
    void mouseMoveEvent(QMouseEvent* event);//重写鼠标移动事件
    QPoint moveTo(QPoint point);//越界判断并转换

    //聊天窗口信息设置
    void setUserName(QString str);//设置用户名
    void setIcon(QString str,int radius=50);//设置任务栏图标
    void initUserData(QJsonObject* user ,QJsonArray* messageData);//初始化用户数据（用户信息和聊天记录）(单人)
    void initUserData(QJsonObject* group ,QJsonArray* groupMemberData,QJsonArray* messageData);//初始化界面信息(群聊)

    void addMessage(QString account,QJsonObject json);//添加消息

    void openChatWIndow();//打开窗口

signals:
    void sendMessage(int type, QString account, QString targetAccount, QString message,QString messageType);
    void openPersonal(QString account);
    void closeChatWindow(QString);

private:
    Ui::ChatWindow *ui;
    //界面拖拽
    bool pressed=false;//鼠标点击信号
    QPoint pressPoint;//鼠标点击初始位置
    int screenWidth=0;
    int screenHeight=0;

    QStandardItemModel *model=nullptr;

    QJsonArray* messageData=nullptr;//聊天记录
    QJsonObject* user;//用户json数据
    QHash<QString,QJsonObject> groupMember;
};

#endif // CHATWINDOW_H
