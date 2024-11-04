#include "searchwindow.h"
#include "ui_searchwindow.h"

SearchWindow::SearchWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SearchWindow)
{
    ui->setupUi(this);

    setStyle();
    setConnect();
    setView();
}

SearchWindow::~SearchWindow()
{
    delete ui;
}

void SearchWindow::setStyle()
{
    //获取屏幕宽高
    QRect rect=QGuiApplication::primaryScreen()->geometry();
    screenWidth=rect.width();
    screenHeight=rect.height();

    //设置窗体无标题栏和提供最小化按钮
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowMinimizeButtonHint);
    setAttribute(Qt::WA_QuitOnClose, false);

    //设置具体阴影
    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(0, 0);
    //阴影颜色
    shadow_effect->setColor(QColor(238, 78, 119, 127));
    //阴影半径
    shadow_effect->setBlurRadius(22);
    ui->frame->setGraphicsEffect(shadow_effect);

    //设置主界面圆角
    this->setObjectName("searchWindow");
    this->setStyleSheet("#searchWindow{border-radius: 20px;}");

    //设置导航栏
    ui->navigate->addTab("找人");
    ui->navigate->addTab("找群");
    ui->navigate->setHaloVisible(false);
    ui->navigate->setRippleStyle(Material::PositionedRipple);
    //设置字体无效，可能是使用绘图事件内部维护了一个字体，无法修改
    //ui->navigate->setFont(QFont("微软雅黑",50));

    //设置图片资源
    ui->close_btn->setIcon(QIcon("://photo/close2.png"));
    ui->min_btn->setIcon(QIcon("://photo/min2.png"));

    //设置登录按键的颜色
    ui->search->setRole(Material::Primary);
    ui->search->setHaloVisible(false);
    ui->search->setOverlayStyle(Material::TintedOverlay);
    ui->search->setRippleStyle(Material::PositionedRipple);

    //设置视图滑动条样式
    ui->user_view->verticalScrollBar()->setStyleSheet(Protocol::scrollbarStyle);
    ui->group_view->verticalScrollBar()->setStyleSheet(Protocol::scrollbarStyle);

}

void SearchWindow::setConnect()
{
    //为最小化按钮和关闭按钮绑定处理函数
    connect(ui->min_btn,&QToolButton::clicked,this,[&](){this->showMinimized();});
    connect(ui->close_btn,&QToolButton::clicked,this,[&](){this->close();this->deleteLater();});

    //导航栏处理
    connect(ui->navigate,&QtMaterialTabs::currentChanged,this,[&](int index){
        if(this->index==index)
            return;
        if(index==0)
        {
            ui->input_edit->setPlaceholderText("请输入学工号/昵称");
            ui->view_stack->setCurrentIndex(0);
        }
        else
        {
            ui->input_edit->setPlaceholderText("请输入群号/群名称");
            ui->view_stack->setCurrentIndex(1);
        }
        ui->input_edit->clear();
        this->index=index;
    });

    connect(ui->search,&QtMaterialRaisedButton::clicked,this,[&](){
        if(!Protocol::isConnecting)
        {
            ElaMessageBar::error(ElaMessageBarType::TopLeft,"Error","登录超时，请检查您的网络或者本地防火墙设置",3000,ui->mid);
            return;
        }
        if(index==0)
        {
            emit searchUser(ui->input_edit->text());
            searchUserModel->clear();
        }
        else
        {
            emit searchGroup(ui->input_edit->text());
            searchGroupModel->clear();
        }
    });

    connect(ui->user_view,&QListView::doubleClicked,this,[&](const QModelIndex& index){
        if(!Protocol::isConnecting)
        {
            ElaMessageBar::error(ElaMessageBarType::TopLeft,"Error","登录超时，请检查您的网络或者本地防火墙设置",3000,this);
            return;
        }
        emit openPersonal(index.data(Qt::UserRole+1).value<MessageItem>().account);
    });
    connect(ui->group_view,&QListView::doubleClicked,this,[&](const QModelIndex& index){
        if(!Protocol::isConnecting)
        {
            ElaMessageBar::error(ElaMessageBarType::TopLeft,"Error","登录超时，请检查您的网络或者本地防火墙设置",3000,this);
            return;
        }
        emit openPersonal(index.data(Qt::UserRole+1).value<MessageItem>().account);
    });
}

void SearchWindow::setView()
{
    //设置用户搜索模型
    searchUserModel=new QStandardItemModel(this);
    searchUserSortModel= new QSortFilterProxyModel(this);
    searchUserSortModel->setSourceModel(searchUserModel);
    searchUserSortModel->setDynamicSortFilter(true);
    searchUserSortModel->setSortRole(Qt::UserRole+2);
    searchUserSortModel->sort(0,Qt::AscendingOrder);

    //设置群聊搜索模型
    searchGroupModel=new QStandardItemModel(this);
    searchGroupSortModel= new QSortFilterProxyModel(this);
    searchGroupSortModel->setSourceModel(searchGroupModel);
    searchGroupSortModel->setDynamicSortFilter(true);
    searchGroupSortModel->setSortRole(Qt::UserRole+2);
    searchGroupSortModel->sort(0,Qt::AscendingOrder);

    //绑定用户搜索视图和模型，设置代理
    ui->user_view->setModel(searchUserSortModel);
    ui->user_view->setItemDelegate(new MessageDelegate(this,MessageDelegateType::RelationType));

    //绑定群聊搜索视图和模型，设置代理
    ui->group_view->setModel(searchGroupSortModel);
    ui->group_view->setItemDelegate(new MessageDelegate(this,MessageDelegateType::RelationType));
}

void SearchWindow::initSearchUser(QJsonArray* json)
{
    searchUserJson=json;
    if(searchUserJson->isEmpty())
        return;
    for(QJsonValue i:*searchUserJson)
    {
        QStandardItem* item=new QStandardItem();
        QJsonObject jsonObj=i.toObject();
        //将需要显示的用户信息添加进模型中
        MessageItem message={jsonObj.value("account").toString(),jsonObj.value("userName").toString(),jsonObj.value("headPhoto").toString(),jsonObj.value("signature").toString(),0,0};
        item->setData(QVariant::fromValue(message),Qt::UserRole+1);
        //添加排序角色
        item->setData(jsonObj.value("userName").toString(),Qt::UserRole+2);
        searchUserModel->appendRow(item);
    }
}

void SearchWindow::initSearchGroup(QJsonArray* json)
{
    searchGroupJson=json;
    if(searchGroupJson->isEmpty())
        return;
    for(QJsonValue i:*searchGroupJson)
    {
        qDebug()<<"开始填充Group信息";
        QStandardItem* item=new QStandardItem();
        QJsonObject jsonObj=i.toObject();
        //将需要显示的用户信息添加进模型中
        MessageItem message={jsonObj.value("account").toString(),jsonObj.value("userName").toString(),jsonObj.value("headPhoto").toString(),jsonObj.value("signature").toString(),0,0};
        item->setData(QVariant::fromValue(message),Qt::UserRole+1);
        //添加排序角色
        item->setData(jsonObj.value("userName").toString(),Qt::UserRole+2);
        searchGroupModel->appendRow(item);
    }
}

void SearchWindow::mousePressEvent(QMouseEvent *event)
{
    //判断是否是左键按压
    if(event->button()==Qt::LeftButton)
    {
        //打开按压信号，并记录按压初始位置
        pressed=true;
        pressPoint=event->pos();
    }
}

void SearchWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton);
    {
        //关闭按压信号
        pressed=false;
    }
}

void SearchWindow::mouseMoveEvent(QMouseEvent *event)
{
    //检查按压信号，然后移动对应距离
    if(pressed)
    {
        this->move(moveTo(event->globalPos()-pressPoint));
    }
}

QPoint SearchWindow::moveTo(QPoint point)
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
