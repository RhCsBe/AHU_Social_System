#ifndef SEARCHWINDOW_H
#define SEARCHWINDOW_H

#include <QWidget>
#include <QScreen>
#include "protocol.h"
#include <QMouseEvent>
#include <ElaPivot.h>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <ElaScrollPageArea.h>
#include <ElaMessageBar.h>
#include "messagedelegate.h"
#include <QGraphicsDropShadowEffect>
#include <QScrollBar>

namespace Ui {
class SearchWindow;
}

class SearchWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SearchWindow(QWidget *parent = nullptr);
    ~SearchWindow();
    void setStyle();
    void setConnect();
    void setView();
    void initSearchUser(QJsonArray* json);
    void initSearchGroup(QJsonArray* json);

    //实现拖拽效果
    void mousePressEvent(QMouseEvent* event);//重写鼠标按压事件
    void mouseReleaseEvent(QMouseEvent* event);//重写鼠标释放事件
    void mouseMoveEvent(QMouseEvent* event);//重写鼠标移动事件
    QPoint moveTo(QPoint point);//越界判断并转换

signals:
    void searchUser(QString message);
    void searchGroup(QString message);
    void openPersonal(QString account);

private:
    Ui::SearchWindow *ui;

    //界面拖拽
    bool pressed=false;//鼠标点击信号
    QPoint pressPoint;//鼠标点击初始位置
    int screenWidth=0;
    int screenHeight=0;

    //导航栏选项
    int index=0;

    //数据指针
    QJsonArray* searchUserJson=nullptr;
    QJsonArray* searchGroupJson=nullptr;

    //数据模型
    QStandardItemModel* searchUserModel=nullptr;
    QSortFilterProxyModel* searchUserSortModel=nullptr;
    QStandardItemModel* searchGroupModel=nullptr;
    QSortFilterProxyModel* searchGroupSortModel=nullptr;
};

#endif // SEARCHWINDOW_H
