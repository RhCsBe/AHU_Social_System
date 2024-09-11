#include "maininterface.h"
#include "ui_maininterface.h"

MainInterface::MainInterface(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainInterface)
{
    ui->setupUi(this);
    setStyle();
    setConnect();
    QListWidgetItem* item=new QListWidgetItem(ui->list);
    FriendItem* myItem=new FriendItem();
    ui->list->setItemWidget(item,myItem);
    item->setSizeHint(QSize(ui->list->size().width(),myItem->size().height()));
}

MainInterface::~MainInterface()
{
    delete ui;
}

void MainInterface::setStyle()
{
    //设置主页面无边框和背景透明，不在任务栏中显示，并且出现在画面最上层
    setWindowFlags(Qt::FramelessWindowHint|Qt::SplashScreen|Qt::WindowStaysOnTopHint);
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
    connect(ui->min_btn,&QToolButton::clicked,this,[&](){this->hide();});
    connect(ui->close_btn,&QToolButton::clicked,this,[&](){this->close();});
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


