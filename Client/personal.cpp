#include "personal.h"
#include "ui_personal.h"

Personal::Personal(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Personal)
{
    ui->setupUi(this);
    setStyle();
    setConnect();
    setEditStatus();
}

Personal::~Personal()
{
    delete ui;
}

void Personal::setStyle()
{
    //设置几个组件的大小
    ui->bloodType_box->setFixedSize(91,31);
    ui->location_box->setFixedSize(111,31);
    ui->sex_box->setFixedSize(81,31);
    ui->college_box->setFixedSize(251,35);
    ui->profession_box->setFixedSize(251,35);
    ui->birthday_calendar->setFixedSize(181,31);

    //设置功能按键样式
    ui->function_button->setRole(Material::Primary);
    ui->function_button->setHaloVisible(false);
    ui->function_button->setOverlayStyle(Material::TintedOverlay);

    //设置下部主体的背景色以及整体的无边框和背景透明以及窗口显示在最上层
    //ui->widget->setStyleSheet("#widget{border:none;background-color:white;}");
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    //setAttribute(Qt::WA_TranslucentBackground);

    //设置图片资源
    ui->close_btn->setIcon(QIcon("://photo/close.png"));
    ui->min_btn->setIcon(QIcon("://photo/min.png"));
    ui->background->setPixmap(QPixmap("://photo/AHU3.jfif"));
}

void Personal::initPersonal(QJsonObject *user, QJsonArray *memberData, PersonalType type)
{
    //设置数据指针和类型
    this->user=user;
    this->memberData=memberData;
    this->type=type;

    //设置用户数据
    resetUserData();

    //根据Personal类型设置function按键
    setFunction(type);

    //根据用户类型设置一下对应样式
    QString account=user->value("account").toString();
    if(account[0]=='E'||account[0]=='T')
    {
        if(account[0]=='E')
        {

        }
        else if(account[0]=='T')
        {
            ui->profession->setVisible(false);
            ui->profession_box->setVisible(false);
        }
        else
        {
            //翻页
            ui->personal_stack->setCurrentIndex(1);
        }
    }
}

void Personal::setConnect()
{
    //为最小化按钮和关闭按钮绑定处理函数
    connect(ui->min_btn,&QToolButton::clicked,this,[&](){this->showMinimized();});
    connect(ui->close_btn,&QToolButton::clicked,this,[&](){this->close();});

    //设置年龄和生日绑定
    connect(ui->birthday_calendar,&ElaCalendarPicker::selectedDateChanged,this,[&](QDate birthday){
        QDate today=QDate::currentDate();
        //年龄=今年-出生年-1
        int age=today.year()-birthday.year()-1;
        //判断是否过了今年生日
        if(today.month()>birthday.month())
        {
            age++;
        }
        else
        {
            if(today.month()>=birthday.month()&&today.day()>=birthday.day())
                age++;
        }
        //不满一周岁按一周岁算
        if(age==0)
            age=1;
        //数据库设置的岁数范围是1-130岁
        if(age>0&&age<=130)
            ui->age_edit->setText(QString::number(age));
        else
        {
            ElaMessageBar::error(ElaMessageBarType::Right,"error","数据错误，出生日期不能大于今天",3000,this);
        }
    });

    //设置左部用户名label与右部的userName_edit和group_name_edit绑定
    connect(ui->userName_edit,&ElaLineEdit::textChanged,this,[&](QString text){
        ui->user_name->setText(text);
    });
    connect(ui->group_name_edit,&ElaLineEdit::textChanged,this,[&](QString text){
        ui->user_name->setText(text);
    });
}

void Personal::setFunction(PersonalType type)
{
    switch (type)
    {
        case FriendType:
        {
            ui->function_button->setText("发消息");
            connect(ui->function_button,&QtMaterialRaisedButton::clicked,this,[&](){
                emit openChatWindow(ui->account_edit->text());
            });
            break;
        }
        case UserType:
        {
            ui->function_button->setText("添加好友");
            connect(ui->function_button,&QtMaterialRaisedButton::clicked,this,[&](){
                emit addFriend(ui->account_edit->text());
            });
            break;
        }
        case JoinedGroupType:
        {
            ui->function_button->setText("发消息");
            ui->personal_stack->setCurrentIndex(1);
            connect(ui->function_button,&QtMaterialRaisedButton::clicked,this,[&](){
                emit openChatWindow(ui->group_account_edit->text());
            });
            break;
        }
        case OtherGroupType:
        {
            ui->function_button->setText("加入群聊");
            ui->personal_stack->setCurrentIndex(1);
            connect(ui->function_button,&QtMaterialRaisedButton::clicked,this,[&](){
                emit joinGroup(ui->group_account_edit->text());
            });
            break;
        }
        case MyselfType:
        {
            ui->function_button->setText("修改信息");
            connect(ui->function_button,&QtMaterialRaisedButton::clicked,this,&Personal::modifyUserData);
            break;
        }
    default:
        break;
    }
}

void Personal::modifyUserData()
{
    //先将状态位取反,然后设置组件编辑状态
    modify=!modify;
    setEditStatus();

    if(modify)
    {
        //保留，以后可能会拓展
    }
    else
    {
        //确认连接保持且已经登录
        if(Protocol::isConnecting&&Protocol::isLogin)
        {
            //一连串数据检查


            //发送信号
        }
        else
        {
            resetUserData();
            ElaMessageBar::warning(ElaMessageBarType::BottomLeft,"warning","连接超时，请检查您的网络或者本地防火墙设置",3000,this);
        }
    }
}

void Personal::resetUserData()
{
    QString account=user->value("account").toString();

    //设置右半部分数据
    //用户信息复原
    if(account[0]=='E'||account[0]=='T')
    {

        ui->account_edit->setText(user->value("account").toString());
        ui->userName_edit->setText(user->value("userName").toString());
        ui->sex_box->setCurrentText(user->value("sex").toString());
        ui->age_edit->setText(QString::number(user->value("age").toInt()));
        ui->birthday_calendar->setText(user->value("birthday").toString());
        ui->bloodType_box->setCurrentText(user->value("bloodType").toString());
        ui->location_box->setCurrentText(user->value("location").toString());
        ui->college_box->setCurrentText(user->value("college").toString());
        //如果是学生用户还要设置专业信息
        if(account[0]=='E')
        {
            ui->profession_box->setCurrentText(user->value("profession").toString());
        }
    }
    //群聊信息复原
    else
    {
        ui->group_account_edit->setText(user->value("account").toString());
        ui->group_name_edit->setText(user->value("groupName").toString());

        //中间缺少了一步往模型中重新加载数据的过程，后面会补上
        ui->sum_edit->setText(QString::number(groupMemberModel->rowCount()));
    }

    //设置左半部分数据
    //设置用户名
    ui->user_name->setText(user->value("userName").toString());
    //设置头像
    QString headPhotoPath=user->value("headPhoto").toString();
    if(headPhotoPath.isEmpty())
    {
        headPhotoPath=DefaultPixmap;
    }
    else
    {
        headPhotoPath=Protocol::getAllUserPath()+"/"+headPhotoPath;
    }
    ui->headPhoto->setIcon(QIcon(headPhotoPath));
}

void Personal::setEditStatus()
{
    //学生和教师用户页面
    ui->userName_edit->setEnabled(modify);
    ui->sex_box->setEnabled(modify);
    ui->birthday_calendar->setEnabled(modify);
    ui->bloodType_box->setEnabled(modify);
    ui->location_box->setEnabled(modify);
    ui->college_box->setEnabled(modify);
    ui->profession_box->setEnabled(modify);

    //群聊页面
    ui->group_name_edit->setEnabled(modify);

    //修改功能按键样式
    if(modify)
    {
        ui->function_button->setText("保存数据");
        ui->function_button->setRole(Material::Secondary);
    }
    else
    {
        ui->function_button->setText("修改数据");
        ui->function_button->setRole(Material::Primary);
    }
}

