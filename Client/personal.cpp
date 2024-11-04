#include "personal.h"
#include "ui_personal.h"

Personal::Personal(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Personal)
{
    ui->setupUi(this);
    setStyle();//设置基本样式
    setElaComboBoxFilter();//为ElaComboBox注册事件过滤器
    setConnect();//设置信号和槽函数连接
    setEditStatus();//设置编辑组件的状态
    //setFunction(type);
}

Personal::~Personal()
{
    delete ui;
}

void Personal::setStyle()
{
    //获取屏幕宽高
    QRect rect=QGuiApplication::primaryScreen()->geometry();
    screenWidth=rect.width();
    screenHeight=rect.height();

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
    ui->function_button->setCornerRadius(5);

    //设置下部主体的背景色以及整体的无边框和背景透明以及窗口显示在最上层
    //ui->widget->setStyleSheet("#widget{border:none;background-color:white;}");
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    setAttribute(Qt::WA_QuitOnClose, false);
    setAttribute(Qt::WA_TranslucentBackground);

    //设置图片资源
    ui->close_btn->setIcon(QIcon("://photo/close.png"));
    ui->min_btn->setIcon(QIcon("://photo/min.png"));

    //取消了布局管理器，这玩意智能，size和sizeHint返回的都不是实际大小，只能取消left_top的布局管理器，使用它的真实大小
    ui->background->setPixmap(QPixmap("://photo/AHU3.jfif").scaled(ui->left_top->size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation));

    //设置头像按键的Icon大小
    ui->headPhoto->setIconSize(ui->headPhoto->size());

    //设置视图滑动条样式
    ui->member_view->verticalScrollBar()->setStyleSheet(Protocol::scrollbarStyle);

    //ui->signature->setStyleSheet("QLineEdit::placeholder {color: red;}");

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
        //设置视图
        setView();
        //翻页
        ui->personal_stack->setCurrentIndex(1);
    }
}

void Personal::setConnect()
{
    //为最小化按钮和关闭按钮绑定处理函数
    connect(ui->min_btn,&QToolButton::clicked,this,[&](){this->showMinimized();});
    connect(ui->close_btn,&QToolButton::clicked,this,[&](){
        this->close();
        emit closePersonal(getAccount());
        this->deleteLater();
    });

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
            ElaMessageBar::error(ElaMessageBarType::Right,"error","数据错误，出生日期不能大于今天或大于130岁",3000,this);
            ui->birthday_calendar->setSelectedDate(QDate::fromString(user->value("birthday").toString(),"yyyy/M/d"));
        }
    });

    //设置左部用户名label与右部的userName_edit和group_name_edit绑定
    connect(ui->userName_edit,&ElaLineEdit::textChanged,this,[&](QString text){
        ui->user_name->setText(text);
    });
    connect(ui->group_name_edit,&ElaLineEdit::textChanged,this,[&](QString text){
        ui->user_name->setText(text);
    });

    //设置学院和专业的关联
    connect(ui->college_box,&ElaComboBox::textActivated,this,[&](QString str){
        if(str=="计算机科学与技术学院")
        {
            ui->profession_box->clear();
            QStringList list{"计算机科学与技术","软件工程","信息安全"};
            ui->profession_box->addItems(list);
        }
        else
        {
            ui->profession_box->clear();
            QStringList list{"语文","数学","英语","物理","化学","生物","政治","历史","地理"};
            ui->profession_box->addItems(list);
        }
    });

    //设置头像按键功能
    connect(ui->headPhoto,&QToolButton::clicked,this,&Personal::setHeadPhoto);
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
                //qDebug()<<"请求向"<<ui->account_edit->text()<<"通信";
            });
            break;
        }
        case UserType:
        {
            ui->function_button->setText("添加好友");
            connect(ui->function_button,&QtMaterialRaisedButton::clicked,this,[&](){
                if(Protocol::isConnecting)
                {
                    emit addFriend(ui->account_edit->text());
                    ElaMessageBar::success(ElaMessageBarType::BottomLeft,"success","发送成功",3000,this);
                }
                else
                {
                    ElaMessageBar::warning(ElaMessageBarType::BottomLeft,"warning","连接超时，请检查您的网络或者本地防火墙设置",3000,this);
                }
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
                if(Protocol::isConnecting)
                {
                    emit joinGroup(user->value("hostAccount").toString(),user->value("account").toString());
                    ElaMessageBar::success(ElaMessageBarType::BottomLeft,"success","发送成功",3000,this);
                }
                else
                {
                    ElaMessageBar::warning(ElaMessageBarType::BottomLeft,"warning","连接超时，请检查您的网络或者本地防火墙设置",3000,this);
                }
            });
            break;
        }
        case MyselfType:
        {
            ui->function_button->setText("修改数据");
            connect(ui->function_button,&QtMaterialRaisedButton::clicked,this,&Personal::modifyUserData);
            break;
        }
        case MyGroupType:
        {
            ui->function_button->setText("修改数据");
            connect(ui->function_button,&QtMaterialRaisedButton::clicked,this,&Personal::modifyUserData);
            break;
        }
    default:
        break;
    }
}

void Personal::initHeadPhoto()
{
    QString headPhotoPath=user->value("headPhoto").toString();
    if(headPhotoPath.isEmpty())
    {
        headPhotoPath=DefaultPixmap;
    }
    else
    {
        headPhotoPath=Protocol::getAllUserPath()+"/"+headPhotoPath;
    }
    ui->headPhoto->setIcon(QIcon(QPixmap(Protocol::createHeadShot(headPhotoPath,ui->headPhoto->size().width()/2))));
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
            if(type==MyselfType)
            {
                if(ui->userName_edit->text().isEmpty())
                {
                    ElaMessageBar::warning(ElaMessageBarType::BottomLeft,"warning","用户昵称不能为空",3000,this);
                    return;
                }
                else
                {
                    //数据顺序：用户名-》个性签名-》性别-》年龄-》生日-》血型-》位置-》学院-》专业(只有学生用户有专业信息)
                    QString message="";
                    message+=ui->userName_edit->text()+";";
                    message+=ui->signature->text()+";";
                    message+=ui->sex_box->currentText()+";";
                    message+=ui->age_edit->text()+";";
                    message+=ui->birthday_calendar->getSelectedDate().toString("yyyy/M/d")+";";
                    message+=ui->bloodType_box->currentText()+";";
                    message+=ui->location_box->currentText()+";";
                    message+=ui->college_box->currentText();
                    if(ui->account_edit->text()[0]=='E')
                    {
                        message+=";";
                        message+=ui->profession_box->currentText();
                    }
                    emit modifyData(ui->account_edit->text(),message,headPhotoPath);
                    ElaMessageBar::success(ElaMessageBarType::BottomLeft,"success","数据修改成功",3000,this);
                    return;
                }
            }
            else
            {
                //群聊类型
                if(ui->group_name_edit->text().isEmpty())
                {
                    ElaMessageBar::warning(ElaMessageBarType::BottomLeft,"warning","群聊名称不能为空",3000,this);
                    return;
                }
                else
                {
                    //数据顺序：用户名-》个性签名
                    QString message="";
                    message+=ui->group_name_edit->text()+";";
                    message+=ui->signature->text();
                    emit modifyData(ui->group_account_edit->text(),message,headPhotoPath);
                    ElaMessageBar::success(ElaMessageBarType::BottomLeft,"success","数据修改成功",3000,this);
                    return;
                }
            }
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
        ui->birthday_calendar->setSelectedDate(QDate::fromString(user->value("birthday").toString(),"yyyy/M/d"));
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
        ui->group_name_edit->setText(user->value("userName").toString());
        ui->group_host_edit->setText(memberData->at(0).toObject().value("userName").toString());
        ui->sum_edit->setText(QString::number(memberData->count()));
    }

    //设置左半部分数据
    //设置用户名和个性签名
    ui->user_name->setText(user->value("userName").toString());
    ui->signature->setText(user->value("signature").toString());

    //设置头像
    initHeadPhoto();
}

void Personal::setEditStatus()
{
    //因为disabled样式的问题，ElaComboBox的处理交给事件过滤

    //头像按键
    //ui->headPhoto->setEnabled(modify);
    //个性签名
    ui->signature->setEnabled(modify);

    //学生和教师用户页面
    ui->userName_edit->setEnabled(modify);
    ui->birthday_calendar->setEnabled(modify);

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

void Personal::setHeadPhoto()
{
    if(!modify)
        return;
    QString headPhotoPath=QFileDialog::getOpenFileName(this, "Open File", Protocol::getAllUserPath());
    qDebug()<<headPhotoPath;
    QPixmap pixmap(headPhotoPath);
    if(pixmap.isNull())
    {
        //文件路径置空，在发送文件时会过滤掉
        this->headPhotoPath="";
        initHeadPhoto();
        ElaMessageBar::warning(ElaMessageBarType::Left,"warning","文件打开失败，请选择图片文件打开",3000,ui->left_bottom);
    }
    else
    {
        //记录头像路径
        this->headPhotoPath=headPhotoPath;
        pixmap=Protocol::createHeadShot(headPhotoPath,ui->headPhoto->size().width()/2);
        ui->headPhoto->setIcon(pixmap);
    }
}



void Personal::mousePressEvent(QMouseEvent *event)
{
    //判断是否是左键按压
    if(event->button()==Qt::LeftButton)
    {
        //打开按压信号，并记录按压初始位置
        pressed=true;
        pressPoint=event->pos();
    }
}

void Personal::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton);
    {
        //关闭按压信号
        pressed=false;
    }
}

void Personal::mouseMoveEvent(QMouseEvent *event)
{
    //检查按压信号，然后移动对应距离
    if(pressed)
    {
        this->move(moveTo(event->globalPos()-pressPoint));
    }
}

QPoint Personal::moveTo(QPoint point)
{
    if(point.x()<3)
        point.setX(3);
    if(point.y()<0)
        point.setY(0);
    if(point.x()+this->size().width()>screenWidth-3)
        point.setX(screenWidth-3-this->size().width());
    if(point.y()+this->size().height()>screenHeight-3)
        point.setY(screenHeight-3-this->size().height());
    return point;
}

bool Personal::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type()==QEvent::MouseButtonPress||event->type()==QEvent::MouseButtonDblClick)
    {
        if(modify)
            return false;
        else
            return true;
    }
}

void Personal::setElaComboBoxFilter()
{
    //因为ElaComboBox可能设置了什么样式表导致我对QComboBox的被禁用时的样式修改失效，所以直接使用事件拦截进行处理
    ui->sex_box->installEventFilter(this);
    ui->bloodType_box->installEventFilter(this);
    ui->location_box->installEventFilter(this);
    ui->college_box->installEventFilter(this);
    ui->profession_box->installEventFilter(this);
}

void Personal::setView()
{
    //初始化模型指针
    groupMemberModel=new QStandardItemModel(this);
    groupMemberSortModel=new QSortFilterProxyModel(this);

    //设置代理模型动态排序
    groupMemberSortModel->setDynamicSortFilter(true);
    groupMemberSortModel->setSourceModel(groupMemberModel);
    groupMemberSortModel->setSortRole(Qt::UserRole+2);
    groupMemberSortModel->sort(0,Qt::AscendingOrder);

    //为视图设置模型和代理
    ui->member_view->setModel(groupMemberSortModel);
    ui->member_view->setItemDelegate(new MessageDelegate(this,MessageDelegateType::RelationType));

    //打开群成员个人主页
    connect(ui->member_view,&QListView::doubleClicked,this,[&](const QModelIndex &index){
        QString account=index.data(Qt::UserRole+1).value<MessageItem>().account;
        emit openPersonal(account);
    });

    //设置模型数据
    initModelData();
}

void Personal::initModelData()
{
    //模型指针为空直接返回
    if(!groupMemberModel)
        return;
    //先清空模型数据
    groupMemberModel->clear();

    //遍历群聊成员
    for(QJsonValue i:*memberData)
    {
        QStandardItem* item=new QStandardItem();
        QJsonObject json=i.toObject();
        MessageItem message={
            json.value("account").toString(),
            json.value("userName").toString(),
            json.value("headPhoto").toString(),
            json.value("signature").toString(),
        };
        item->setData(QVariant::fromValue(message),Qt::UserRole+1);
        item->setData(json.value("userName").toString(),Qt::UserRole+2);//以用户名作为排序依据
        groupMemberModel->appendRow(item);
    }
    qDebug()<<"群人数："<<memberData->size();
}

QString Personal::getAccount()
{
    return user->value("account").toString();
}

