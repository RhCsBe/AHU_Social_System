#include "login.h"
#include "ui_login.h"

Login::Login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);
    setStyle();
    setConnect();
    initUserData();
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
    ui->widget->setStyleSheet("#widget{border:none;background-color:white;}");
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    //setAttribute(Qt::WA_TranslucentBackground);

    //设置图片资源
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
            emit sendLogin(ui->account->text(),ui->password->text());
        }
        else
        {
            qDebug()<<"匹配失败";
        }
    });

}

void Login::initUserData()
{
    //设置工作路径，并获取工作目录下所有的文件夹
    QDir dir;
    dir.setPath(Protocol::getWorkPath());
    QStringList userList=dir.entryList(QDir::Dirs);
    //除去无关文件夹
    userList.removeOne(".");
    userList.removeOne("..");
    userList.removeOne("allUser");

    if(userList.size()==0)
    {
        setHeadPhoto("://photo/AHU1.png");
    }
    else
    {
        //根据上次登录时间确定选中的账号
        QString selectAccount="";
        int maxNumber=-1;
        //遍历获取所有的login信息，并存入userList
        for(auto user:userList)
        {
            QFile file(Protocol::getWorkPath()+"/"+user+"/login.json");
            if(!file.open(QFile::ReadOnly))
            {
                qDebug()<<"打开"<<user<<"的登录文件失败";
                continue;
            }
            //account.append(user);
            QByteArray data=file.readAll();
            file.close();
            QJsonObject json=QJsonDocument::fromJson(data).object();
            if(json.value("lastLoginTime").toInt()>maxNumber)
            {
                maxNumber=json.value("lastLoginTime").toInt();
                selectAccount=user;
            }
            this->userList.insert(user,json);
            ui->account_box->addItem(user);
        }
        QJsonObject json=this->userList.value(selectAccount);
        initLoginData(json);
    }
    initComboBox();
}

void Login::initLoginData(QJsonObject json)
{
    QString account,password,headPhoto;
    bool remember,autoLogin;
    account=json.value("account").toString();
    password=json.value("password").toString();
    headPhoto=Protocol::getWorkPath()+"/"+account+"/"+json.value("headPhoto").toString();
    remember=json.value("remember").toBool();
    autoLogin=json.value("autoLogin").toBool();
    ui->account->setText(account);
    ui->password->setText(password);
    ui->remenber_password->setChecked(remember);
    ui->auto_login->setChecked(autoLogin);
    setHeadPhoto(headPhoto);
}

void Login::initComboBox()
{
    //这垃圾样式表写不下去了，然后自己组了一个，可以实现类似功能
    //就是使用QToolButton代替QComboBox::down-arrow，点击时触发QComboBox对应处理
    connect(ui->down_btn,&QToolButton::clicked,this,[&](){
        down=!down;
        if(down)
        {
            ui->down_btn->setIcon(QIcon(":/photo/shangjiantou.png"));
            ui->account_box->showPopup();
        }
        else
        {
            ui->down_btn->setIcon(QIcon(":/photo/xiajiantou.png"));
            ui->account_box->hidePopup();
        }
    });
    connect(ui->account_box,&QComboBox::textActivated,this,[&](QString str){
        ui->account->setText(str);
        down=false;
        ui->down_btn->setIcon(QIcon(":/photo/xiajiantou.png"));
        initLoginData(userList.value(str));
    });
    //因为QComboBox没有提供对应的信号，只能使用事件过滤器来实现失去焦点的处理
    //因为QComboBox展示下拉列表后会失去焦点，所以此时获取焦点的是下拉列表，应该监控下拉列表
    ui->account_box->view()->installEventFilter(this);
    //用库有风险，这傻逼bug弄了半天，最后没办法了，因为输入框上半部分有透视效果，
    //且ElaComboBox会自动重设大小，所以将它设置为不可见，但下拉列表仍然可见
    ui->account_box->setVisible(false);
}

void Login::setHeadPhoto(QString str)
{
    //因为给QLabel设置圆角无法对设置的图片直接进行裁剪显示，还是会显示原图，达不到圆形头像的效果，所以不能直接等比缩放显示
    //ui->head_photo->setPixmap(QPixmap(str).scaled(ui->head_photo->size().width(),ui->head_photo->size().height(), Qt::KeepAspectRatio,Qt::SmoothTransformation));

    //第一种方式：缩放->裁剪->重绘
    //获取头像框和半径大小
    int size=ui->head_photo->size().height();
    int radius=size/2;

    //等比缩放原图
    QPixmap src=QPixmap(str).scaled(size,size, Qt::KeepAspectRatio,Qt::SmoothTransformation);

    //新建一个空白画布
    QPixmap result(ui->head_photo->size());
    result.fill(Qt::transparent);//填充透明背景

    //将空白画布设置为画笔的绘图背景
    QPainter painter(&result);
    painter.setRenderHints(QPainter::Antialiasing);//抗锯齿
    painter.setRenderHints(QPainter::SmoothPixmapTransform);//平滑像素图变换

    //设置绘制路径，并添加到画笔中
    QPainterPath path;
    path.addRoundedRect(0,0,size,size,radius,radius);
    painter.setClipPath(path);
    //绘制成果图
    painter.drawPixmap(0,0,size,size,src);
    painter.setClipping(false);//关闭裁剪
    //设置头像
    ui->head_photo->setPixmap(result);


    //第二种方法：创建透明遮罩
    //效果很奇怪，显示安大校徽时正常，但是显示路飞头像时不知道为什么把透明的是中心区域
//    QPixmap pixmap(str);
//    QBitmap mask(ui->head_photo->size());
//    mask.fill(Qt::transparent);
//    QPainter painter(&mask);
//    painter.setRenderHint(QPainter::Antialiasing);
//    painter.setBrush(Qt::white);
//    int radius=ui->head_photo->rect().x()/2;
//    painter.drawRoundedRect(ui->head_photo->rect(),20, 20);
//    painter.end();

//    // 应用遮罩到图片
//    QPixmap roundedPixmap = pixmap.scaled(ui->head_photo->size(), Qt::KeepAspectRatio,Qt::SmoothTransformation);
//    roundedPixmap.setMask(mask);

//    // 设置遮罩后的图片到 QLabel
//    ui->head_photo->setPixmap(roundedPixmap);
}

bool Login::eventFilter(QObject *watched, QEvent *event)
{
    //QComboBox失去后将标志位设置为false，并且箭头设置向下
    if(event->type()==QEvent::FocusOut)
    {
        down=false;
        ui->down_btn->setIcon(QIcon(":/photo/xiajiantou.png"));
    }
    return false;
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
    //账号为E或T开头，后面有8位数字
    QRegularExpression accountExp("[E|T]\\d{8}");
    //密码为8-16位数字字母组合
    QRegularExpression passwordExp("[A-Z|a-z|0-9]{8,16}");
    return accountExp.match(ui->account->text()).hasMatch()&&passwordExp.match(ui->password->text()).hasMatch();
}


