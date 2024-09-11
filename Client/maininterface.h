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

QT_BEGIN_NAMESPACE
namespace Ui { class MainInterface; }
QT_END_NAMESPACE

class MainInterface : public QWidget
{
    Q_OBJECT

public:
    MainInterface(QWidget *parent = nullptr);
    ~MainInterface();
    void setStyle();//设置主界面样式
    void setConnect();//设置相关连接

    //实现拖拽效果
    void mousePressEvent(QMouseEvent* event);//重写鼠标按压事件
    void mouseReleaseEvent(QMouseEvent* event);//重写鼠标释放事件
    void mouseMoveEvent(QMouseEvent* event);//重写鼠标移动事件

private:
    Ui::MainInterface *ui;
    bool pressed=false;//鼠标点击信号
    QPoint pressPoint;//鼠标点击初始位置
    TcpThread tcp;
};
#endif // MAININTERFACE_H
