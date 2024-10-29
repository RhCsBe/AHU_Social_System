#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    //初始化两个tcp指针和work指针以及两个线程指针
    tcp=new TcpThread();
    work=new WorkThread();
    tcpThread=new QThread();
    workThread=new QThread();

    //数据接收和发送
    connect(tcp,&TcpThread::receiveFinished,work,&WorkThread::mergeDataPackage);
    connect(work,&WorkThread::sendToServer,tcp,&TcpThread::sendToClient);

    //初始化tcp线程
    connect(this,&Widget::inItServer,tcp,&TcpThread::initServer);

    //线程通知
    connect(tcp,&TcpThread::myInformation,this,&Widget::myInformation);
    connect(work,&WorkThread::myInformation,this,&Widget::myInformation);

    //用户上线/下线
    connect(work,&WorkThread::addUserOnline,tcp,&TcpThread::addUserOnline);
    connect(tcp,&TcpThread::userDownLine,work,&WorkThread::userDownLine);

    //移入线程并开启
    tcp->moveToThread(tcpThread);
    work->moveToThread(workThread);
    tcpThread->start();
    workThread->start();

    //初始化tcp线程对象
    emit inItServer();
}

Widget::~Widget()
{
    delete tcp;
    delete work;
    delete ui;
}

void Widget::myInformation(QString str)
{
    //QMessageBox::information(nullptr,"提示",str);
    qDebug()<<str;
}

