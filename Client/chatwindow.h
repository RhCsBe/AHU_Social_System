#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include <QFontMetrics>
#include <QDebug>
#include <QMouseEvent>

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWindow(QWidget *parent = nullptr);
    ~ChatWindow();
    void setStyle();
    void setConnect();

    //实现拖拽效果
    void mousePressEvent(QMouseEvent *event);//重写鼠标按压事件
    void mouseReleaseEvent(QMouseEvent *event);//重写鼠标释放事件
    void mouseMoveEvent(QMouseEvent *event);//重写鼠标移动事件

private:
    Ui::ChatWindow *ui;
    bool pressed=false;//鼠标点击信号
    QPoint pressPoint;//鼠标点击初始位置
};

#endif // CHATWINDOW_H
