#ifndef ADDWINDOW_H
#define ADDWINDOW_H

#include <QWidget>
#include "protocol.h"
#include <QGraphicsDropShadowEffect>
#include <QScreen>
#include <QMouseEvent>
#include <QStandardItemModel>
#include "messagedelegate.h"
#include <ElaMessageBar.h>
#include <QScrollBar>

namespace Ui {
class AddWindow;
}

class AddWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AddWindow(QJsonArray* addFriendJson,QJsonArray* joinGroupJson,QWidget *parent = nullptr);
    ~AddWindow();
    void setStyle();
    void setConnect();

    //实现拖拽效果
    void mousePressEvent(QMouseEvent* event);//重写鼠标按压事件
    void mouseReleaseEvent(QMouseEvent* event);//重写鼠标释放事件
    void mouseMoveEvent(QMouseEvent* event);//重写鼠标移动事件
    QPoint moveTo(QPoint point);//越界判断并转换

    //更新模型信息
    void initAddFriend();
    void initJoinGroup();
    void setView();

    //清除数据
    void removeAddFriendModel(QVector<int>& vector);
    void removeJoinGroupModel(QVector<int>& vector);
    void removeAddFriendJson(QVector<int>& vector);
    void removeJoinGroupJson(QVector<int>& vector);

    //按键功能函数
    void agreeAddFriend();
    void deleteAddFriend();
    void agreeJoinGroup();
    void deleteJoinGroup();

signals:
    void addFriend(QString account);
    void joinGroup(QString account,QString group);

private:
    Ui::AddWindow *ui;

    //界面拖拽
    bool pressed=false;//鼠标点击信号
    QPoint pressPoint;//鼠标点击初始位置
    int screenWidth=0;
    int screenHeight=0;

    //导航栏选项
    int index=0;

    //数据指针
    QJsonArray* addFriendJson=nullptr;
    QJsonArray* joinGroupJson=nullptr;

    //数据模型
    QStandardItemModel* addFriendModel=nullptr;
    QStandardItemModel* joinGroupModel=nullptr;
};

#endif // ADDWINDOW_H
