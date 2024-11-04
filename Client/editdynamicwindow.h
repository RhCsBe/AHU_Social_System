#ifndef EDITDYNAMICWINDOW_H
#define EDITDYNAMICWINDOW_H

#include <QWidget>
#include "protocol.h"
#include <QGraphicsDropShadowEffect>
#include <QScreen>
#include <QMouseEvent>
#include <ElaMessageBar.h>

namespace Ui {
class EditDynamicWindow;
}

class EditDynamicWindow : public QWidget
{
    Q_OBJECT

public:
    explicit EditDynamicWindow(QWidget *parent = nullptr);
    ~EditDynamicWindow();
    void setStyle();
    void setConnect();

    //实现拖拽效果
    void mousePressEvent(QMouseEvent* event);//重写鼠标按压事件
    void mouseReleaseEvent(QMouseEvent* event);//重写鼠标释放事件
    void mouseMoveEvent(QMouseEvent* event);//重写鼠标移动事件
    QPoint moveTo(QPoint point);//越界判断并转换

    void publish();

signals:
    void sendDynamic(QString message);

private:
    Ui::EditDynamicWindow *ui;

    //界面拖拽
    bool pressed=false;//鼠标点击信号
    QPoint pressPoint;//鼠标点击初始位置
    int screenWidth=0;
    int screenHeight=0;
};

#endif // EDITDYNAMICWINDOW_H
