#include "creategroupwindow.h"
#include "ui_creategroupwindow.h"

CreateGroupWindow::CreateGroupWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CreateGroupWindow)
{
    ui->setupUi(this);
    setStyle();
    setConnect();
    setView();
}

CreateGroupWindow::~CreateGroupWindow()
{
    delete ui;
}

void CreateGroupWindow::setStyle()
{
    //获取屏幕宽高
    QRect rect=QGuiApplication::primaryScreen()->geometry();
    screenWidth=rect.width();
    screenHeight=rect.height();

    //设置窗体无标题栏和提供最小化按钮
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowMinimizeButtonHint);
    setAttribute(Qt::WA_QuitOnClose, false);

    //设置主界面圆角
    this->setObjectName("editDynamicWindow");
    this->setStyleSheet("#editDynamicWindow{border-radius: 20px;}");

    //设置具体阴影
    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(0, 0);
    //阴影颜色
    shadow_effect->setColor(QColor(238, 78, 119, 127));
    //阴影半径
    shadow_effect->setBlurRadius(22);
    ui->frame->setGraphicsEffect(shadow_effect);

    //设置图片资源
    ui->close_btn->setIcon(QIcon("://photo/close2.png"));
    ui->min_btn->setIcon(QIcon("://photo/min2.png"));

    //设置按键的颜色
    ui->send_btn->setRole(Material::Primary);
    ui->send_btn->setHaloVisible(false);
    ui->send_btn->setOverlayStyle(Material::TintedOverlay);
    ui->send_btn->setRippleStyle(Material::PositionedRipple);

    //设置视图多选
    ui->friend_view->setSelectionMode(QAbstractItemView::MultiSelection);
    //设置视图滑动条样式
    ui->friend_view->verticalScrollBar()->setStyleSheet(Protocol::scrollbarStyle);
}

void CreateGroupWindow::setConnect()
{
    //为最小化按钮和关闭按钮绑定处理函数
    connect(ui->min_btn,&QToolButton::clicked,this,[&](){this->showMinimized();});
    connect(ui->close_btn,&QToolButton::clicked,this,[&](){this->close();this->deleteLater();});
    //发布动态
    connect(ui->send_btn,&QtMaterialFlatButton::clicked,this,&CreateGroupWindow::create);
}

void CreateGroupWindow::setView()
{
    if(friendModel)
    {
        friendModel->clear();
    }
    else
    {
        //初始化model指针
        friendModel=new QStandardItemModel(this);

        //设置排序代理模型
        friendSortModel=new QSortFilterProxyModel(this);
        friendSortModel->setSourceModel(friendModel);//设置源数据模型
        friendSortModel->setDynamicSortFilter(true);//打开动态排序
        friendSortModel->setSortRole(Qt::UserRole+2);//设置排序角色
        friendSortModel->sort(0,Qt::AscendingOrder);//以第0列升序排序

        //为视图设置模型和代理
        ui->friend_view->setModel(friendSortModel);
        ui->friend_view->setItemDelegate(new MessageDelegate(this,MessageDelegateType::RelationType));

        //为视图绑定双击处理函数，即双击选项创建并打开对应的聊天窗口
        connect(ui->friend_view,&QListView::doubleClicked,this,[&](const QModelIndex &index){
            QString account=index.data(Qt::UserRole+1).value<MessageItem>().account;
            emit openPersonal(account);
        });
    }

    //打开对应的好友信息文件
    QFile file(Protocol::getUserPath()+"/friend.json");
    file.open(QFile::ReadOnly);
    QByteArray data=file.readAll();
    QJsonDocument jsonDoc=QJsonDocument::fromJson(data);
    QJsonArray jsonArray=jsonDoc.array();
    for(auto i:jsonArray)
    {
        QStandardItem* item=new QStandardItem();
        QJsonObject jsonObj=i.toObject();
        //将需要显示的好友信息添加进模型中
        MessageItem message={jsonObj.value("account").toString(),jsonObj.value("userName").toString(),jsonObj.value("headPhoto").toString(),jsonObj.value("signature").toString(),0,0};
        item->setData(QVariant::fromValue(message),Qt::UserRole+1);
        //添加排序角色
        item->setData(jsonObj.value("userName").toString(),Qt::UserRole+2);
        friendModel->appendRow(item);
    }
}

void CreateGroupWindow::mousePressEvent(QMouseEvent *event)
{
    //判断是否是左键按压
    if(event->button()==Qt::LeftButton)
    {
        //打开按压信号，并记录按压初始位置
        pressed=true;
        pressPoint=event->pos();
    }
}

void CreateGroupWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton);
    {
        //关闭按压信号
        pressed=false;
    }
}

void CreateGroupWindow::mouseMoveEvent(QMouseEvent *event)
{
    //检查按压信号，然后移动对应距离
    if(pressed)
    {
        this->move(moveTo(event->globalPos()-pressPoint));
    }
}

QPoint CreateGroupWindow::moveTo(QPoint point)
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

void CreateGroupWindow::create()
{
    if(!Protocol::isConnecting)
    {
        ElaMessageBar::error(ElaMessageBarType::BottomLeft,"Error","登录超时，请检查您的网络或者本地防火墙设置",3000,ui->frame);
        return;
    }
    else
    {
        if(ui->group_name_edit->text().isEmpty())
        {
            ElaMessageBar::error(ElaMessageBarType::BottomLeft,"Error","群名称不能为空",3000,ui->frame);
            return;
        }
        QItemSelectionModel* model=ui->friend_view->selectionModel();
        if(model->hasSelection())
        {
            QString member="";
            QModelIndexList list=model->selectedIndexes();
            for(QModelIndex index:list)
            {
                member+=index.data(Qt::UserRole+1).value<MessageItem>().account;
                member+=";";
            }
            //去掉最后的;字符
            member.chop(1);
            emit createGroup(ui->group_name_edit->text(),member);
            return;
        }
        else
        {
            ElaMessageBar::error(ElaMessageBarType::BottomLeft,"Error","群成员不能为空",3000,ui->frame);
            return;
        }
    }
}
