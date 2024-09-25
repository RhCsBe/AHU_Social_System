#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    tcp=new TcpThread();
    work=new WorkThread();
    tcpThread=new QThread();
    workThread=new QThread();
    connect(tcp,&TcpThread::receiveFinished,work,&WorkThread::mergeDataPackage);
    connect(work,&WorkThread::sendToServer,tcp,&TcpThread::sendToClient);
    connect(this,&Widget::inItServer,tcp,&TcpThread::initServer);
    connect(tcp,&TcpThread::myInformation,this,&Widget::myInformation);
    connect(work,&WorkThread::myInformation,this,&Widget::myInformation);
    tcp->moveToThread(tcpThread);
    work->moveToThread(workThread);
    tcpThread->start();
    workThread->start();
    emit inItServer();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::myInformation(QString str)
{
    QMessageBox::information(nullptr,"提示",str);
}

