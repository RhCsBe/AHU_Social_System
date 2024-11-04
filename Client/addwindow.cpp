#include "addwindow.h"
#include "ui_addwindow.h"

AddWindow::AddWindow(QJsonArray* addFriendJson,QJsonArray* joinGroupJson,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddWindow)
{
    ui->setupUi(this);
    this->addFriendJson=addFriendJson;
    this->joinGroupJson=joinGroupJson;

    setStyle();
    setConnect();
    setView();
}

AddWindow::~AddWindow()
{
    delete ui;
}

void AddWindow::setStyle()
{
    //获取屏幕宽高
    QRect rect=QGuiApplication::primaryScreen()->geometry();
    screenWidth=rect.width();
    screenHeight=rect.height();

    //设置窗体无标题栏和提供最小化按钮
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowMinimizeButtonHint);
    setAttribute(Qt::WA_QuitOnClose, false);

    //设置主界面圆角
    this->setObjectName("addWindow");
    this->setStyleSheet("#addWindow{border-radius: 20px;}");

    //设置具体阴影
    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(0, 0);
    //阴影颜色
    shadow_effect->setColor(QColor(238, 78, 119, 127));
    //阴影半径
    shadow_effect->setBlurRadius(22);
    ui->frame->setGraphicsEffect(shadow_effect);

    //设置导航栏
    ui->navigate->addTab("新朋友");
    ui->navigate->addTab("群通知");
    ui->navigate->setHaloVisible(false);
    ui->navigate->setRippleStyle(Material::PositionedRipple);
    //设置字体无效，可能是使用绘图事件内部维护了一个字体，无法修改
    //ui->navigate->setFont(QFont("微软雅黑",50));

    //设置图片资源
    ui->close_btn->setIcon(QIcon("://photo/close2.png"));
    ui->min_btn->setIcon(QIcon("://photo/min2.png"));

    //设置按键的颜色
    //好友申请同意按键
    ui->agree_friend_btn->setRole(Material::Primary);
    ui->agree_friend_btn->setHaloVisible(false);
    ui->agree_friend_btn->setOverlayStyle(Material::TintedOverlay);
    ui->agree_friend_btn->setRippleStyle(Material::PositionedRipple);
    //好友申请删除按键
    ui->delete_friend_btn->setRole(Material::Secondary);
    ui->delete_friend_btn->setHaloVisible(false);
    ui->delete_friend_btn->setOverlayStyle(Material::TintedOverlay);
    ui->delete_friend_btn->setRippleStyle(Material::PositionedRipple);
    //加群申请同意按键
    ui->agree_group_btn->setRole(Material::Primary);
    ui->agree_group_btn->setHaloVisible(false);
    ui->agree_group_btn->setOverlayStyle(Material::TintedOverlay);
    ui->agree_group_btn->setRippleStyle(Material::PositionedRipple);
    //加群申请删除按键
    ui->delete_group_btn->setRole(Material::Secondary);
    ui->delete_group_btn->setHaloVisible(false);
    ui->delete_group_btn->setOverlayStyle(Material::TintedOverlay);
    ui->delete_group_btn->setRippleStyle(Material::PositionedRipple);

    //设置视图多选
    ui->add_friend_list->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->join_group_list->setSelectionMode(QAbstractItemView::MultiSelection);

    //设置视图滑动条样式
    ui->add_friend_list->verticalScrollBar()->setStyleSheet(Protocol::scrollbarStyle);
    ui->join_group_list->verticalScrollBar()->setStyleSheet(Protocol::scrollbarStyle);
}

void AddWindow::setConnect()
{
    //为最小化按钮和关闭按钮绑定处理函数
    connect(ui->min_btn,&QToolButton::clicked,this,[&](){this->showMinimized();});
    connect(ui->close_btn,&QToolButton::clicked,this,[&](){this->close();this->deleteLater();});

    connect(ui->navigate,&QtMaterialTabs::currentChanged,this,[&](int index){
        if(this->index==index)
            return;
        if(index==0)
        {
            ui->view_stack->setCurrentIndex(0);
        }
        else
        {
            ui->view_stack->setCurrentIndex(1);
        }
        this->index=index;
    });

    //四个功能按键处理
    connect(ui->agree_friend_btn,&QtMaterialFlatButton::clicked,this,&AddWindow::agreeAddFriend);
    connect(ui->delete_friend_btn,&QtMaterialFlatButton::clicked,this,&AddWindow::deleteAddFriend);
    connect(ui->agree_group_btn,&QtMaterialFlatButton::clicked,this,&AddWindow::agreeJoinGroup);
    connect(ui->delete_group_btn,&QtMaterialFlatButton::clicked,this,&AddWindow::deleteJoinGroup);
}

void AddWindow::mousePressEvent(QMouseEvent *event)
{
    //判断是否是左键按压
    if(event->button()==Qt::LeftButton)
    {
        //打开按压信号，并记录按压初始位置
        pressed=true;
        pressPoint=event->pos();
    }
}

void AddWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton);
    {
        //关闭按压信号
        pressed=false;
    }
}

void AddWindow::mouseMoveEvent(QMouseEvent *event)
{
    //检查按压信号，然后移动对应距离
    if(pressed)
    {
        this->move(moveTo(event->globalPos()-pressPoint));
    }
}

QPoint AddWindow::moveTo(QPoint point)
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

void AddWindow::initAddFriend()
{
    //检查指针
    if(!addFriendJson)
        return;

    //清空原来的数据
    addFriendModel->clear();

    //遍历添加
    int size=addFriendJson->size();
    for(int i=0;i<size;i++)
    {
        QStandardItem* item=new QStandardItem();
        QJsonObject json=addFriendJson->at(i).toObject();
        MessageItem message={json.value("account").toString(),json.value("userName").toString(),json.value("headPhoto").toString(),"请求添加好友",0,0};
        item->setData(QVariant::fromValue(message),Qt::UserRole+1);
        addFriendModel->appendRow(item);
    }
}

void AddWindow::initJoinGroup()
{
    if(!joinGroupJson)
        return;

    //清空原来的数据
    joinGroupModel->clear();

    //遍历添加
    int size=joinGroupJson->size();
    for(int i=0;i<size;i++)
    {
        QStandardItem* item=new QStandardItem();
        QJsonObject json=joinGroupJson->at(i).toObject();
        MessageItem message={json.value("account").toString(),json.value("userName").toString(),json.value("headPhoto").toString(),"请求加入群聊:"+json.value("groupName").toString(),0,0};
        item->setData(QVariant::fromValue(message),Qt::UserRole+1);
        joinGroupModel->appendRow(item);
    }
}

void AddWindow::setView()
{
    //设置模型、视图、代理
    if(!addFriendModel)
    {
        addFriendModel=new QStandardItemModel(this);
        ui->add_friend_list->setModel(addFriendModel);
        ui->add_friend_list->setItemDelegate(new MessageDelegate(this,MessageDelegateType::RelationType));
    }
    if(!joinGroupModel)
    {
        joinGroupModel=new QStandardItemModel(this);
        ui->join_group_list->setModel(joinGroupModel);
        ui->join_group_list->setItemDelegate(new MessageDelegate(this,MessageDelegateType::RelationType));
    }

    //加载数据
    initAddFriend();
    initJoinGroup();
}

void AddWindow::removeAddFriendModel(QVector<int> &vector)
{
    //vecotr中是排序好的降序的行数
    for(int i:vector)
    {
        addFriendModel->removeRow(i);
    }
}

void AddWindow::removeJoinGroupModel(QVector<int> &vector)
{
    //vecotr中是排序好的降序的行数
    for(int i:vector)
    {
        joinGroupModel->removeRow(i);
    }
}

void AddWindow::removeAddFriendJson(QVector<int> &vector)
{
    //vecotr中是排序好的降序的行数
    for(int i:vector)
    {
        addFriendJson->removeAt(i);
    }
}

void AddWindow::removeJoinGroupJson(QVector<int> &vector)
{
    //vecotr中是排序好的降序的行数
    for(int i:vector)
    {
        joinGroupJson->removeAt(i);
    }
}

void AddWindow::agreeAddFriend()
{
    if(!Protocol::isConnecting)
    {
        ElaMessageBar::error(ElaMessageBarType::BottomLeft,"Error","登录超时，请检查您的网络或者本地防火墙设置",3000,ui->buttom);
        return;
    }
    else
    {
        QItemSelectionModel* model=ui->add_friend_list->selectionModel();
        if(model->hasSelection())
        {
            QModelIndexList list=model->selectedIndexes();
            QVector<int> vector;
            for(QModelIndex index:list)
            {
                emit addFriend(index.data(Qt::UserRole+1).value<MessageItem>().account);
                vector.push_back(index.row());
            }
            //按照升序排列
            std::sort(vector.begin(),vector.end(),[](int x,int y){ return x>y;});
            removeAddFriendJson(vector);
            initAddFriend();
        }
        ElaMessageBar::success(ElaMessageBarType::BottomLeft,"success","添加成功",3000,ui->buttom);
    }
}

void AddWindow::deleteAddFriend()
{
    QItemSelectionModel* model=ui->add_friend_list->selectionModel();
    if(model->hasSelection())
    {
        QModelIndexList list=model->selectedIndexes();
        QVector<int> vector;
        for(QModelIndex index:list)
        {
            vector.push_back(index.row());
        }
        //按照升序排列
        std::sort(vector.begin(),vector.end(),[](int x,int y){ return x>y;});
        removeAddFriendJson(vector);
        initAddFriend();
    }
}

void AddWindow::agreeJoinGroup()
{
    //检查连接状态
    if(!Protocol::isConnecting)
    {
        ElaMessageBar::error(ElaMessageBarType::BottomLeft,"Error","登录超时，请检查您的网络或者本地防火墙设置",3000,ui->buttom);
        return;
    }
    else
    {
        QItemSelectionModel* model=ui->join_group_list->selectionModel();
        if(model->hasSelection())
        {
            QModelIndexList list=model->selectedIndexes();
            QVector<int> vector;
            for(QModelIndex index:list)
            {
                emit joinGroup(index.data(Qt::UserRole+1).value<MessageItem>().account,joinGroupJson->at(index.row()).toObject().value("groupAccount").toString());
                vector.push_back(index.row());
            }
            //按照升序排列
            std::sort(vector.begin(),vector.end(),[](int x,int y){ return x>y;});
            removeJoinGroupJson(vector);
            initJoinGroup();
        }
        ElaMessageBar::success(ElaMessageBarType::BottomLeft,"success","添加成功",3000,ui->buttom);
    }
}

void AddWindow::deleteJoinGroup()
{
    QItemSelectionModel* model=ui->join_group_list->selectionModel();
    if(model->hasSelection())
    {
        QModelIndexList list=model->selectedIndexes();
        QVector<int> vector;
        for(QModelIndex index:list)
        {
            vector.push_back(index.row());
        }
        //按照升序排列
        std::sort(vector.begin(),vector.end(),[](int x,int y){ return x>y;});
        removeJoinGroupJson(vector);
        initJoinGroup();
    }
}

