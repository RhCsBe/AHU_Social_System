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
    //为最小化按钮和关闭按钮绑定处理函数
    connect(ui->min_btn,&QToolButton::clicked,this,[&](){this->showMinimized();});
    connect(ui->close_btn,&QToolButton::clicked,this,[&](){this->close();});

    //创建注册界面并绑定处理函数
    connect(ui->make_account,&QPushButton::clicked,this,[&](){
        if(registerWindow)
        {
            //qDebug()<<"显示register";
            //直接显示三件套
            registerWindow->show();
            registerWindow->showNormal();
            registerWindow->activateWindow();
        }
        else
        {
            //qDebug()<<"创建register";
            registerWindow=new Register();
            //为注册界面绑定处理函数
            connect(registerWindow,&Register::userRegister,this,[&](QString account,QString message,QString messageType){
                emit userRegister(account,message,messageType);
            });
            //注册窗口销毁时记得将指针置空，防止越界访问导致系统崩溃
            connect(registerWindow,&Register::destroyed,this,[&](){registerWindow=nullptr;});
            registerWindow->show();
        }
    });
    //登录按钮处理
    connect(ui->login,&QPushButton::clicked,this,[&](){
        if(this->matchRegExp())
        {
            if(!Protocol::isConnecting)
            {
                ElaMessageBar::error(ElaMessageBarType::BottomRight,"Error","登录超时，请检查您的网络或者本地防火墙设置",3000,this);
                return;
            }
            qint64 lastLoginTime=-1;
            qint64 loginTime=QDateTime::currentDateTime().toMSecsSinceEpoch();
            QJsonObject json=userList.value(ui->account->text());
            if(!json.isEmpty())
            {
                //有登录记录则获取上次登录时间，交给服务端处理同步问题
                lastLoginTime=json.value("lastLoginTime").toString().toLongLong();
            }
            else
            {
                lastLoginTime=-1;
                //不存在登录记录则将第一次登录位设置为true
                Protocol::isFirstLogin=true;
            }
            Protocol::initLoginUserInfo(ui->account->text(),ui->password->text(),lastLoginTime,loginTime);
            Protocol::isAutoLogin=ui->auto_login->isChecked();
            Protocol::isRemember=ui->remenber_password->isChecked();
            emit sendLogin(ui->account->text(),ui->password->text());
        }
        else
        {
            ElaMessageBar::warning(ElaMessageBarType::BottomLeft,"Warning","账号或密码不符合要求",3000,this);
        }
    });
    //设置当账号发生改变时，若找不到历史记录，则将密码清空，头像设置成默认头像，若匹配上历史记录，则设置成历史记录
    connect(ui->account,&QtMaterialTextField::textChanged,this,[&](QString str){
        QJsonObject json = userList.value(str);
        if(json.isEmpty())
        {
            //设置默认头像
            setHeadPhoto("://photo/AHU1.png");
            ui->password->clear();
        }
        else
        {
            initLoginData(json);
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
        //设置默认头像
        setHeadPhoto("://photo/AHU1.png");
    }
    else
    {
        //根据上次登录时间确定选中的账号
        QString selectAccount="";
        qint64 maxNumber=-1;
        //遍历获取所有的login信息，并存入userList
        for(auto user:userList)
        {
            QFile file(Protocol::getWorkPath()+"/"+user+"/login.json");
            if(!file.open(QFile::ReadOnly))
            {
                //qDebug()<<"打开"<<user<<"的登录文件失败";
                continue;
            }
            //account.append(user);
            QByteArray data=file.readAll();
            file.close();
            QJsonObject json=QJsonDocument::fromJson(data).object();
            if(json.value("lastLoginTime").toString().toLongLong()>maxNumber)
            {
                maxNumber=json.value("lastLoginTime").toString().toLongLong();
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
    //从json文件中读取用户信息
    QString account,password,headPhoto;
    bool remember,autoLogin;
    account=json.value("account").toString();
    password=json.value("password").toString();
    //用户头像为空则设置为默认头像
    if(json.value("headPhoto").toString()=="")
        headPhoto=DefaultPixmap;
    else
        headPhoto=Protocol::getAllUserPath()+"/"+json.value("headPhoto").toString();
    remember=json.value("remember").toBool();
    autoLogin=json.value("autoLogin").toBool();

    //根据读取的数据设置ui参数
    ui->account->setText(account);
    ui->password->setText(password);
    ui->remenber_password->setChecked(remember);
    ui->auto_login->setChecked(autoLogin);

    //设置头像
    setHeadPhoto(headPhoto);
}

void Login::initComboBox()
{
    //这垃圾样式表写不下去了，然后自己组了一个，可以实现类似功能
    //就是使用QToolButton代替QComboBox::down-arrow，点击时触发QComboBox对应处理
    ui->down_btn->setIcon(QIcon(":/photo/xiajiantou.png"));
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
    //qDebug()<<"headPhoto:"<<str;
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
    //ElaMessageBar::information(ElaMessageBarType::PositionPolicy::BottomRight,"通知","开始进行正则匹配",5000,this);
    return accountExp.match(ui->account->text()).hasMatch()&&passwordExp.match(ui->password->text()).hasMatch();
}

void Login::registerResult(int result)
{
    //注册界面指针不为空则显示注册结果
    if(registerWindow)
        registerWindow->regiterResult(result);
}

void Login::loginResult(bool result)
{
    if(!result)
    {
        ElaMessageBar::error(ElaMessageBarType::BottomRight,"error","账号或密码错误",3000,this);
    }
}


