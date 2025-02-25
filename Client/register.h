#ifndef REGISTER_H
#define REGISTER_H

#include <QWidget>
#include "protocol.h"
#include <QPalette>
#include <QMouseEvent>
#include "ElaMessageBar.h"
#include <QScreen>

namespace Ui {
class Register;
}

class Register : public QWidget
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = nullptr);
    ~Register();

    //一些初始化操作
    void setStyle();
    void setConnect();

    //实现拖拽效果
    void mousePressEvent(QMouseEvent* event);//重写鼠标按压事件
    void mouseReleaseEvent(QMouseEvent* event);//重写鼠标释放事件
    void mouseMoveEvent(QMouseEvent* event);//重写鼠标移动事件
    QPoint moveTo(QPoint point);//越界判断并转换

    //数据检测
    bool matchRegExp();//前端正则匹配
    bool judgeMessage();//判断数据是否完整且符合要求

    //注册结果
    void regiterResult(int result);

signals:
    void userRegister(QString account,QString message,QString messageType);

private:
    Ui::Register *ui;
    //界面拖拽
    bool pressed=false;//鼠标点击信号
    QPoint pressPoint;//鼠标点击初始位置
    int screenWidth=0;
    int screenHeight=0;
};

#endif // REGISTER_H
