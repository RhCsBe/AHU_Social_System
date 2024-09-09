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
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
private:
    Ui::Login *ui;
    bool pressed=false;
    QPoint press_point;
};

#endif // LOGIN_H
