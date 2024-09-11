#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "tcpthread.h"
#include "protocol.h"
#include "sqldata.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
    TcpThread tcp;
    SqlData sql;
};
#endif // WIDGET_H
