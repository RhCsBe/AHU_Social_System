#include "login.h"
#include "ui_login.h"

Login::Login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);
    setStyle();
    setConnect();
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

    //设置下部主体的背景色以及整体的无边框和背景透明
    ui->widget->setStyleSheet("border:none;background-color:white;");
    setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

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
}

void Login::setConnect()
{
    connect(ui->min_btn,&QToolButton::clicked,this,[&](){this->hide();});
    connect(ui->close_btn,&QToolButton::clicked,this,[&](){this->close();});
    qDebug()<<"OK";
}

void Login::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton);
    {
        pressed=true;
        press_point=event->pos();
    }
}

void Login::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton);
    {
        pressed=false;
    }
}

void Login::mouseMoveEvent(QMouseEvent *event)
{
    if(pressed)
    {
        move(event->globalPos()- press_point);
    }
}
