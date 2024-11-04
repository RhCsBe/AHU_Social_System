#ifndef CREATEGROUPWINDOW_H
#define CREATEGROUPWINDOW_H

#include <QWidget>
#include "protocol.h"
#include <QGraphicsDropShadowEffect>
#include <QScreen>
#include <QMouseEvent>
#include <QStandardItemModel>
#include "messagedelegate.h"
#include <ElaMessageBar.h>
#include <QScrollBar>
#include <QSortFilterProxyModel>
#include <QJsonDocument>

namespace Ui {
class CreateGroupWindow;
}

class CreateGroupWindow : public QWidget
{
    Q_OBJECT

public:
    explicit CreateGroupWindow(QWidget *parent = nullptr);
    ~CreateGroupWindow();

    void setStyle();
    void setConnect();
    void setView();

    //实现拖拽效果
    void mousePressEvent(QMouseEvent* event);//重写鼠标按压事件
    void mouseReleaseEvent(QMouseEvent* event);//重写鼠标释放事件
    void mouseMoveEvent(QMouseEvent* event);//重写鼠标移动事件
    QPoint moveTo(QPoint point);//越界判断并转换

    //创建群聊
    void create();

signals:
    void createGroup(QString groupName,QString memberList);
    void openPersonal(QString account);


private:
    Ui::CreateGroupWindow *ui;

    //界面拖拽
    bool pressed=false;//鼠标点击信号
    QPoint pressPoint;//鼠标点击初始位置
    int screenWidth=0;
    int screenHeight=0;

    //数据模型
    QStandardItemModel* friendModel=nullptr;
    QSortFilterProxyModel* friendSortModel=nullptr;
};

#endif // CREATEGROUPWINDOW_H
