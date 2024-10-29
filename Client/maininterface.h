#ifndef MAININTERFACE_H
#define MAININTERFACE_H

#include <QWidget>
#include <qtmaterialtabs.h>
#include <QPixmap>
#include <QMouseEvent>
#include <QPoint>
#include <QDebug>
#include "frienditem.h"
#include "chatwindow.h"
#include "tcpthread.h"
#include <QThread>
#include "login.h"
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QStandardItemModel>
#include <QStandardItem>
#include "messagedelegate.h"
#include <QMenu>
#include <QAction>
#include <QSortFilterProxyModel>
#include <QHashIterator>
#include "personal.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainInterface; }
QT_END_NAMESPACE

class MainInterface : public QWidget
{
    Q_OBJECT

public:
    MainInterface(QWidget *parent = nullptr);
    ~MainInterface();
    void myInformation(QString str);

    //程序启动初始化
    void setStyle();//设置主界面样式
    void setConnect();//设置相关连接
    void setTcpThread();//设置tcp线程

    //登录成功初始化相关操作
    void setSystemTrayIcon();//设置系统托盘图标
    void initUserData();//初始化用户数据
    void initHead();//初始化主界面头部
    void initMessageHash();//初始化消息映射表，即读取历史通讯记录
    void loginSuccess();//登录成功初始化
    void getHistoryMessage();//读取服务器传来的同步历史消息

    //视图相关
    void setView();//设置视图
    void setMessageView();//设置消息列表视图
    void setRelationView();//设置联系人列表视图

    //消息通讯
    void getMessage(QString account,QJsonObject json);
    int getMessageItemIndex(QString account);//返回消息列表对应项的行数

    //实现拖拽效果
    void mousePressEvent(QMouseEvent* event);//重写鼠标按压事件
    void mouseReleaseEvent(QMouseEvent* event);//重写鼠标释放事件
    void mouseMoveEvent(QMouseEvent* event);//重写鼠标移动事件

    //创建并打开一个聊天界面
    void openChatWindow(const QModelIndex& index);

    //系统关闭时的存储操作
    void writeLoginFile();
    void writeFriendFile();
    void writeMessageFile();

signals:
    void connectToServer();//连接服务器，等Tcp任务类移入Tcp线程后启动
    void connectChatWindow(ChatWindow* chatWindow);
private:
    Ui::MainInterface *ui;
    bool pressed=false;//鼠标点击信号
    QPoint pressPoint;//鼠标点击初始位置


    TcpThread* tcp;//tcp线程工作类
    QThread* tcpThread;//tcp线程
    Login* login;//登录界面

    QSystemTrayIcon* systemTray=nullptr;//系统托盘图标

    QHash<QString,QJsonArray*> messageHash;//消息映射表
    QHash<QString,QJsonObject*> friendHash;//好友信息映射表
    QStandardItemModel* messageModel=nullptr;//消息列表模型
    QStandardItemModel* relationModel=nullptr;//联系人列表模型
    QSortFilterProxyModel* messageSortModel=nullptr;//消息列表代理排序模型
    QSortFilterProxyModel* relationSortModel=nullptr;//联系人列表代理排序模型
    QHash<QString,ChatWindow*> chatWindowList;//聊天界面列表
};
#endif // MAININTERFACE_H
