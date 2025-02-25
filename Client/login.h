#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <qtmaterialraisedbutton.h>
#include <qtmaterialtextfield.h>
#include <QMovie>
#include <QPixmap>
#include <QMouseEvent>
#include <QPoint>
#include <QDebug>
#include "protocol.h"
#include <QRegularExpression>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <ElaComboBox.h>
#include <QAbstractItemView>
#include <QEvent>
#include <QBitmap>
#include <QPainter>
#include <QDateTime>
#include <ElaMessageBar.h>
#include "register.h"
#include <QGraphicsDropShadowEffect>
#include <QColor>
#include <QScreen>

namespace Ui {
class Login;
}

class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();

    //初始化操作
    void setStyle();//设置登录界面的样式
    void setConnect();//绑定信号和槽函数
    void initUserData();//初始化用户信息
    void initLoginData(QJsonObject json);//初始化登录信息(即一打开登录界面看到的信息）
    void initComboBox();//初始化登录下拉框
    void setHeadPhoto(QString str);//设置登录头像
    bool eventFilter(QObject *watched, QEvent *event);//重写事件过滤函数

    //实现拖拽效果
    void mousePressEvent(QMouseEvent* event);//重写鼠标按压事件
    void mouseReleaseEvent(QMouseEvent* event);//重写鼠标释放事件
    void mouseMoveEvent(QMouseEvent* event);//重写鼠标移动事件
    QPoint moveTo(QPoint point);//越界判断并转换

    //前端正则匹配
    bool matchRegExp();

    //注册相关
    void registerResult(int result);

    //登录相关
    void loginResult(int result);

signals:
    void sendLogin(QString account,QString password);
    void userRegister(QString account,QString message,QString messageType);

private:
    Ui::Login *ui;

    //界面拖拽
    bool pressed=false;//鼠标点击信号
    QPoint pressPoint;//鼠标点击初始位置
    int screenWidth=0;
    int screenHeight=0;

    QHash<QString,QJsonObject> userList;//登录用户信息表
    bool down=false;
    ElaComboBox* box=nullptr;
    Register* registerWindow=nullptr;
};

#endif // LOGIN_H
