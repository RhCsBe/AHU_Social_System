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
#include <QApplication>
#include <QScreen>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QColor>
#include "searchwindow.h"
#include "addwindow.h"
#include "editdynamicwindow.h"
#include "dynamicdelegate.h"
#include "creategroupwindow.h"

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
    void initAddAndJoin();//读取addFriend和JoinGroup两个文件信息
    void loginSuccess();//登录成功初始化

    //同步历史信息
    void getHistoryMessage();//读取服务器传来的同步历史消息
    void getHistoryDynamic();//读取服务器传来的同步历史动态

    //视图相关
    void setView();//设置视图
    void setMessageView();//设置消息列表视图
    void setFriendView();//设置好友列表视图
    void setGroupView();//设置群聊列表视图
    void setDynamicView();//设置动态列表视图

    //消息通讯
    void getMessage(QString account,QJsonObject json);
    int getMessageItemIndex(QString account);//返回消息列表对应项的行数

    //收到动态
    void getDynamic(QJsonObject json);

    //实现拖拽效果
    void mousePressEvent(QMouseEvent* event);//重写鼠标按压事件
    void mouseReleaseEvent(QMouseEvent* event);//重写鼠标释放事件
    void mouseMoveEvent(QMouseEvent* event);//重写鼠标移动事件
    QPoint moveTo(QPoint point);//移动坐标转换

    //贴边隐藏
    bool isInScreen();//判断是否在屏幕边缘
    void hideWindow();//隐藏窗口
    void showWindow();//显示窗口
    void enterEvent(QEvent *event);//鼠标进入事件
    void leaveEvent(QEvent *event);//鼠标离开事件

    //创建并打开一个其他界面(聊天界面/个人信息界面/搜索界面)
    void openChatWindow(const QModelIndex& index);
    void openChatWindow(QString account);
    void openPersonal(QString account);
    void openSearchWindow();
    void openAddWindow();
    void openEditDynamicWindow();
    void openCreateGroupWindow();

    //不同界面之间的关联
    void connectChatWindowToPersonal(ChatWindow* chatWindow);
    void connectPersonalToChatWindow(Personal* personal);
    void connectPersonalFunction(Personal* personal);

    //系统关闭时的存储操作
    void downLine();
    void writeLoginFile();
    void writeFriendFile();
    void writeMessageFile();
    void writeDynamicFile();
    void writeAddFriendFile();
    void writeJoinGroupFile();

    void closeAllWindow();

    //读取搜索或请求的信息
    void getSearchOrAskData(int type);

signals:
    void connectToServer();//连接服务器，等Tcp任务类移入Tcp线程后启动
    void connectChatWindow(ChatWindow* chatWindow);
    void askForData(int type,QString account,QString message);//请求/搜索用这个信号
    void addFriend(QString sender,QString receiver,QString result);
    void joinGroup(QString sender,QString receiver,QString group,QString result);
    void sendDynamic(QString account,QString message);
    void createGroup(QString account,QString groupName,QString member);
    void modifyUserData(QString account,QString message,QString file);
    void modifyGroupData(QString account,QString message,QString file);

private:
    Ui::MainInterface *ui;
    //界面拖拽
    bool pressed=false;//鼠标点击信号
    QPoint pressPoint;//鼠标点击初始位置
    int screenWidth=0;
    int screenHeight=0;
    bool isHide=false;

    //多线程
    TcpThread* tcp;//tcp线程工作类
    QThread* tcpThread;//tcp线程

    //其他界面
    Login* login;//登录界面
    QSystemTrayIcon* systemTray=nullptr;//系统托盘图标
    QHash<QString,ChatWindow*> chatWindowList;//聊天界面列表
    QHash<QString,Personal*> personalList;//个人信息界面
    SearchWindow* searchWindow=nullptr;//搜索界面
    AddWindow* addWindow=nullptr;//添加好友/加入群聊界面
    EditDynamicWindow* editDynamicWindow=nullptr;//发布动态界面
    CreateGroupWindow* createGroupWindow=nullptr;//创建群聊界面


    //数据存储
    QHash<QString,QJsonArray*> messageHash;//消息映射表
    QHash<QString,QJsonObject*> friendHash;//好友信息映射表
    QHash<QString,QJsonObject*> groupHash;//群聊信息映射表
    QHash<QString,QJsonArray*> groupMemberHash;//群聊成员信息映射表
    QHash<QString,QJsonObject*> otherUserHash;//其他用户信息映射表(包括群聊内非好友用户/搜索用户等)
    QHash<QString,QJsonObject*> otherGroupHash;//其他群聊信息映射表
    QHash<QString,QJsonArray*> otherGroupMemberHash;//其他群聊成员信息映射表
    QJsonArray* addFriendJson=nullptr;//添加好友信息映射表
    QJsonArray* joinGroupJson=nullptr;//加入群聊信息映射表
    QHash<QString,QJsonObject> messageListHash;//消息列表映射表
    QJsonArray dynamicJson;//动态信息表


    //其他数据(非本地数据)
    QJsonArray* searchUserArray=nullptr;
    QJsonArray* searchGroupArray=nullptr;


    //数据模型
    QStandardItemModel* messageModel=nullptr;//消息列表模型
    QStandardItemModel* friendModel=nullptr;//好友列表模型
    QStandardItemModel* groupModel=nullptr;//群聊列表模型
    QStandardItemModel* dynamicModel=nullptr;//动态列表模型
    QSortFilterProxyModel* messageSortModel=nullptr;//消息列表代理排序模型
    QSortFilterProxyModel* friendSortModel=nullptr;//联系人列表代理排序模型
    QSortFilterProxyModel* groupSortModel=nullptr;//群聊列表代理排序模型
    QSortFilterProxyModel* dynamicSortModel=nullptr;//动态列表代理排序模型
};
#endif // MAININTERFACE_H
