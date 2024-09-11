#include "login.h"
#include "ui_login.h"

Login::Login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);
    setStyle();
    setConnect();
    //qDebug()<<sizeof(FileType);
}

Login::~Login()
{
    delete ui;
}

void Login::setStyle()
{
    //设置登录按键的颜色
    ui->login->setRole(Material::Primary);

    //设置两个输入框的文字
    ui->account->setLabel("学号/工号");
    ui->password->setLabel("密码");

    //设置下部主体的背景色以及整体的无边框和背景透明以及窗口显示在最上层
    ui->widget->setStyleSheet("border:none;background-color:white;");
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowMinimizeButtonHint);
    //setAttribute(Qt::WA_TranslucentBackground);

    //设置图片资源
    ui->head_photo->setPixmap(QPixmap("://photo/AHU1.png").scaled(ui->head_photo->size().width(),ui->head_photo->size().height(), Qt::KeepAspectRatio,Qt::SmoothTransformation));
    ui->close_btn->setIcon(QIcon("://photo/close2.png"));
    ui->min_btn->setIcon(QIcon("://photo/min2.png"));
    ui->logo->setPixmap(QPixmap("://photo/AHU1.png").scaled(ui->logo->size().width(),ui->logo->size().height(), Qt::KeepAspectRatio,Qt::SmoothTransformation));

    //设置上部主题动图
    QMovie* back_movie=new QMovie("://photo/main.gif");
    back_movie->setScaledSize(QSize(ui->background->size()));
    ui->background->setMovie(back_movie);
    back_movie->start();

    //设置tip
    ui->close_btn->setToolTip("关闭");
    ui->close_btn->setToolTipDuration(2000);
    ui->min_btn->setToolTip("最小化");
    ui->min_btn->setToolTipDuration(2000);
}

void Login::setConnect()
{
    connect(ui->min_btn,&QToolButton::clicked,this,[&](){this->showMinimized();});
    connect(ui->close_btn,&QToolButton::clicked,this,[&](){this->close();});
    connect(ui->login,&QPushButton::clicked,this,[&](){
        if(this->matchRegExp())
        {
            qDebug()<<"匹配成功";
        }
        else
        {
            qDebug()<<"匹配失败";
        }
    });
}

void Login::mousePressEvent(QMouseEvent *event)
{
    //判断是否是左键按压
    if(event->button()==Qt::LeftButton)
    {
        //打开按压信号，并记录按压初始位置
        pressed=true;
        pressPoint=event->pos();
    }
}

void Login::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton);
    {
        //关闭按压信号
        pressed=false;
    }
}

void Login::mouseMoveEvent(QMouseEvent *event)
{
    //检查按压信号，然后移动对应距离
    if(pressed)
    {
        move(event->globalPos()- pressPoint);
    }
}

bool Login::matchRegExp()
{
    QRegularExpression accountExp("[E|T]\\d{8}");
    QRegularExpression passwordExp("[A-Z|a-z|0-9]{8,16}");
    return accountExp.match(ui->account->text()).hasMatch()&&passwordExp.match(ui->password->text()).hasMatch();
}


