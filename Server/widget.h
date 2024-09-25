#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "tcpthread.h"
#include "protocol.h"
#include "sqldata.h"
#include "workthread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    void myInformation(QString str);

signals:
    void inItServer();

private:
    Ui::Widget *ui;

    //Tcp任务类及Tcp线程
    TcpThread* tcp;
    QThread* tcpThread;

    //数据处理任务类及数据处理线程
    WorkThread* work;
    QThread* workThread;

    //数据库操作对象
    //SqlData sql;
};
#endif // WIDGET_H
