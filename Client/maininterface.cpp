#include "maininterface.h"
#include "ui_maininterface.h"

MainInterface::MainInterface(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainInterface)
{
    ui->setupUi(this);
    setStyle();
    setConnect();
    login=new Login();
    //login->setWindowFlags(Qt::Window);
    setTcpThread();
    login->show();
    //QListWidgetItem* item=new QListWidgetItem(ui->list);
    //FriendItem* myItem=new FriendItem();
    //ui->list->setItemWidget(item,myItem);
    //item->setSizeHint(QSize(ui->list->size().width(),myItem->size().height()));
}

MainInterface::~MainInterface()
{
    delete ui;
}

void MainInterface::myInformation(QString str)
{
    //如果将父对象设置为空指针，那么关闭掉消息盒后主界面也会关闭，然后程序结束
    //原因：不明，但不是子消息队列的缘故，尝试使用非模态，但还是一样的情况，直到给消息盒设置父对象
    //QMessageBox::information(nullptr,"提示",str);
    QMessageBox::information(this,"提示",str);
//    QMessageBox *box = new QMessageBox(QMessageBox::Information,"提示",str);
//    box->setModal(false);
//    box->setAttribute(Qt::WA_DeleteOnClose);
//    box->show();
    qDebug()<<str;
}

void MainInterface::setStyle()
{
    //设置主页面无边框和背景透明，不在任务栏中显示，并且出现在画面最上层
    setWindowFlags(Qt::FramelessWindowHint|Qt::SplashScreen);
    setAttribute(Qt::WA_TranslucentBackground);

    //添加导航栏标签
    ui->navigate->addTab("消息");
    ui->navigate->addTab("联系人");
    ui->navigate->addTab("动态");

    //添加图片资源
    ui->min_btn->setIcon(QIcon("://photo/min2.png"));
    ui->close_btn->setIcon(QIcon("://photo/close2.png"));
    ui->logo->setPixmap(QPixmap("://photo/AHU1.png").scaled(ui->logo->size().width(),ui->logo->size().height(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    ui->head_btn->setIcon(QIcon("://photo/AHU1.png"));
    ui->head_btn->setIconSize(QSize(ui->head_btn->size()));

    //设置tip
    ui->close_btn->setToolTip("关闭");
    ui->close_btn->setToolTipDuration(2000);
    ui->min_btn->setToolTip("最小化");
    ui->min_btn->setToolTipDuration(2000);

    //ui->navigate->setStyleSheet("border:none;");
}

void MainInterface::setConnect()
{
    //这里将主界面最小化不使用showMinimized()而是使用hide()
    //因为将主界面设置为SplashScreen后使用showMinimized()无法将界面全部关闭
    //可能是因为设置完毕后，窗体无法托管到任务栏，所以最小化失败
    connect(ui->min_btn,&QToolButton::clicked,this,[&](){this->hide();});
    connect(ui->close_btn,&QToolButton::clicked,this,[&](){this->close();});
}

void MainInterface::setTcpThread()
{
    tcp=new TcpThread();
    tcpThread=new QThread();

    connect(login,&Login::sendLogin,tcp,[this](QString account,QString password){tcp->getJsonData(InfoType::LoginAccount,account,"",password);});
    connect(tcp,&TcpThread::loginSuccess,this,[this](bool result){
        if(result)
        {
            setSystemTrayIcon();
            systemTray->show();
            qDebug()<<"打开主界面";
            this->show();
            this->login->hide();
            //this->exec();
        }
        else
        {
            QMessageBox::information(nullptr,"提示","账号或密码错误");
        }
    });
    connect(this,&MainInterface::connectToServer,tcp,&TcpThread::connectServer);
    connect(tcp,&TcpThread::myInformation,this,&MainInterface::myInformation);
    tcp->moveToThread(tcpThread);
    tcpThread->start();
    emit connectToServer();
}

void MainInterface::setSystemTrayIcon()
{
    systemTray=new QSystemTrayIcon(this);
    QIcon icon("://photo/AHU1.png");
    systemTray->setIcon(icon);
    //绑定系统托盘图标的激活信号和处理函数
    connect(systemTray,&QSystemTrayIcon::activated,this,[&](QSystemTrayIcon::ActivationReason reason)->void{
        if(reason==QSystemTrayIcon::Trigger)
            this->showNormal();
        else if(reason==QSystemTrayIcon::DoubleClick)
            this->showNormal();
        else
            return;
        //this->raise();
        //窗体获取焦点，显示在最上层
        this->activateWindow();
    });
}

void MainInterface::mousePressEvent(QMouseEvent* event)
{
    if(event->button()==Qt::LeftButton)
    {
        pressed=true;
        pressPoint=event->pos();
    }
}

void MainInterface::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button()==Qt::LeftButton)
    {
        pressed=false;
    }
}

void MainInterface::mouseMoveEvent(QMouseEvent* event)
{
    if(pressed)
    {
        this->move(event->globalPos()-pressPoint);
    }
}


