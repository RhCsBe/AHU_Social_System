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

namespace Ui {
class Login;
}

class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();
    void setStyle();//设置登录界面的样式
    void setConnect();//绑定信号和槽函数

    //实现拖拽效果
    void mousePressEvent(QMouseEvent *event);//重写鼠标按压事件
    void mouseReleaseEvent(QMouseEvent *event);//重写鼠标释放事件
    void mouseMoveEvent(QMouseEvent *event);//重写鼠标移动事件

private:
    Ui::Login *ui;
    bool pressed=false;//鼠标点击信号
    QPoint press_point;//鼠标点击初始位置
};

#endif // LOGIN_H
