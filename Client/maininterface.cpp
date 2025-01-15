#include "maininterface.h"
#include "ui_maininterface.h"

MainInterface::MainInterface(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainInterface)
{
    ui->setupUi(this);
    setStyle();
    setConnect();
    login=new Login();
    //login->setWindowFlags(Qt::Window);
    setTcpThread();
    login->show();

    //setView();

}

MainInterface::~MainInterface()
{
    if(Protocol::isLogin)
    {
        qDebug()<<"关闭客户端";
        downLine();
    }
    delete ui;
}

void MainInterface::myInformation(QString str)
{
    //如果将父对象设置为空指针，那么关闭掉消息盒后主界面也会关闭，然后程序结束
    //原因：不明，但不是子消息队列的缘故，尝试使用非模态，但还是一样的情况，直到给消息盒设置父对象
    //QMessageBox::information(nullptr,"提示",str);
    //QMessageBox::information(this,"提示",str);
//    QMessageBox *box = new QMessageBox(QMessageBox::Information,"提示",str);
//    box->setModal(false);
//    box->setAttribute(Qt::WA_DeleteOnClose);
//    box->show();
    //qDebug()<<str;
}

void MainInterface::setStyle()
{
    //获取屏幕宽高
    QRect rect=QGuiApplication::primaryScreen()->geometry();
    screenWidth=rect.width();
    screenHeight=rect.height();

    //设置主页面无边框和背景透明，不在任务栏中显示，并且出现在画面最上层
    setWindowFlags(Qt::FramelessWindowHint|Qt::SplashScreen|Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    //setAttribute(Qt::WA_Hover);

    //设置具体阴影
    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(0, 0);
    //阴影颜色
    shadow_effect->setColor(QColor(238, 78, 119, 127));
    //阴影半径
    shadow_effect->setBlurRadius(22);
    ui->frame->setGraphicsEffect(shadow_effect);

    //添加导航栏标签
    ui->navigate->addTab("消息");
    ui->navigate->addTab("联系人");
    ui->navigate->addTab("动态");

    //设置"联系人"下导航栏，实际上是按键组
    ui->friend_btn->setRole(Material::Primary);
    ui->friend_btn->setRippleStyle(Material::PositionedRipple);
    ui->friend_btn->applyPreset(Material::CheckablePreset);
    ui->friend_btn->setHaloVisible(false);
    ui->friend_btn->setOverlayStyle(Material::TintedOverlay);
    ui->friend_btn->setBackgroundMode(Qt::TransparentMode);

    ui->group_btn->setRole(Material::Primary);
    ui->group_btn->setRippleStyle(Material::PositionedRipple);
    ui->group_btn->applyPreset(Material::CheckablePreset);
    ui->group_btn->setHaloVisible(false);
    ui->group_btn->setOverlayStyle(Material::TintedOverlay);
    ui->group_btn->setBackgroundMode(Qt::TransparentMode);
//    ElaPivot* _pivot=new ElaPivot(ui->relation_navigate);
//    _pivot->resize(640,48);
//    _pivot->setStyleSheet("background-color: red;");
//    _pivot->setTextPixelSize(18);
//    qDebug()<<_pivot->size();
//    qDebug()<<ui->relation_navigate->size();
//    _pivot->setPivotSpacing(30);
//    _pivot->setMarkWidth(75);
//    _pivot->appendPivot("本地歌曲");
//    _pivot->appendPivot("下载歌曲");
    //ElaScrollPageArea* pivotArea = new ElaScrollPageArea(ui->relation_navigate);
    //QVBoxLayout* pivotLayout = new QVBoxLayout(pivotArea);
    //pivotLayout->addWidget(_pivot);

    //添加图片资源
    //顶部
    ui->min_btn->setIcon(QIcon("://photo/min2.png"));
    ui->close_btn->setIcon(QIcon("://photo/close2.png"));
    ui->logo->setPixmap(QPixmap("://photo/AHU1.png").scaled(ui->logo->size().width(),ui->logo->size().height(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    ui->head_btn->setIcon(QIcon("://photo/AHU1.png"));
    ui->head_btn->setIconSize(QSize(ui->head_btn->size()));
    //底部
    ui->search_btn->setIconSize(ui->search_btn->size());
    ui->add_friend_btn->setIconSize(ui->add_friend_btn->size());
    ui->dynamic_btn->setIconSize(ui->dynamic_btn->size());
    ui->create_group_btn->setIconSize(ui->create_group_btn->size());
    ui->search_btn->setIcon(QIcon("://photo/search2.jfif"));
    ui->add_friend_btn->setIcon(QIcon("://photo/addfriend4.jfif"));
    ui->dynamic_btn->setIcon(QIcon("://photo/dynamic2.jfif"));
    ui->create_group_btn->setIcon(QIcon("://photo/createGroup3.jfif"));

    //设置tip
    ui->close_btn->setToolTip("关闭");
    ui->close_btn->setToolTipDuration(2000);
    ui->min_btn->setToolTip("最小化");
    ui->min_btn->setToolTipDuration(2000);
    ui->add_friend_btn->setToolTip("添加好友/加入群聊");
    ui->add_friend_btn->setToolTipDuration(2000);
    ui->search_btn->setToolTip("搜索");
    ui->search_btn->setToolTipDuration(2000);
    ui->dynamic_btn->setToolTip("发布动态");
    ui->dynamic_btn->setToolTipDuration(2000);
    ui->create_group_btn->setToolTip("创建群聊");
    ui->create_group_btn->setToolTipDuration(2000);

    //设置视图滚动条样式
    ui->message_list->verticalScrollBar()->setStyleSheet(Protocol::scrollbarStyle);
    ui->friend_list->verticalScrollBar()->setStyleSheet(Protocol::scrollbarStyle);
    ui->group_list->verticalScrollBar()->setStyleSheet(Protocol::scrollbarStyle);
    ui->dynamic_list->verticalScrollBar()->setStyleSheet(Protocol::scrollbarStyle);
}

void MainInterface::setConnect()
{
    //这里将主界面最小化不使用showMinimized()而是使用hide()
    //因为将主界面设置为SplashScreen后使用showMinimized()无法将界面全部关闭
    //可能是因为设置完毕后，窗体无法托管到任务栏，所以最小化失败
    connect(ui->min_btn,&QToolButton::clicked,this,[&](){this->hide();});
    connect(ui->close_btn,&QToolButton::clicked,this,[&](){this->close();});

    //导航栏设置
    connect(ui->navigate,&QtMaterialTabs::currentChanged,this,[&](int index){ui->view_stack->setCurrentIndex(index);});

    //头像按键设置
    connect(ui->head_btn,&QToolButton::clicked,this,[&](){openPersonal(Protocol::getUserAccount());});

    //联系人设置
    connect(ui->friend_btn,&QtMaterialFlatButton::clicked,this,[&](){ui->relation_stack->setCurrentIndex(0);});
    connect(ui->group_btn,&QtMaterialFlatButton::clicked,this,[this](){ui->relation_stack->setCurrentIndex(1);});

    //绑定底部按键
    connect(ui->search_btn,&QToolButton::clicked,this,&MainInterface::openSearchWindow);
    connect(ui->add_friend_btn,&QToolButton::clicked,this,&MainInterface::openAddWindow);
    connect(ui->dynamic_btn,&QToolButton::clicked,this,&MainInterface::openEditDynamicWindow);
    connect(ui->create_group_btn,&QToolButton::clicked,this,&MainInterface::openCreateGroupWindow);
}

void MainInterface::setTcpThread()
{
    tcp=new TcpThread();
    tcpThread=new QThread();

    //传送登录信息
    connect(login,&Login::sendLogin,tcp,[this](QString account,QString password){
        tcp->getJsonData(InfoType::LoginAccount,account,"",password);
    });
    connect(tcp,&TcpThread::loginSuccess,this,[this](int result){
        //-1——登录失败，0——重复登录，1——登录成功
        if(result==1)
        {
            Protocol::isLogin=true;//将登录标签位设置为true
            Protocol::isAutoLogin=false;
            loginSuccess();//用户登录成功设置
            this->login->hide();
        }
        else
        {
            login->loginResult(result);
            //QMessageBox::information(nullptr,"提示","账号或密码错误");
        }
    });

    //传送注册信息
    connect(login,&Login::userRegister,tcp,[this](QString account,QString message,QString messageType){
        tcp->getJsonData(InfoType::Registration,account,"",message,messageType);
    });
    connect(tcp,&TcpThread::registerResult,this,[&](int result){login->registerResult(result);});

    //连接服务器
    connect(this,&MainInterface::connectToServer,tcp,&TcpThread::connectServer);

    //发送/接收消息
    connect(tcp,&TcpThread::myInformation,this,&MainInterface::myInformation);
    connect(tcp,&TcpThread::getMessage,this,&MainInterface::getMessage);

    //同步历史信息，即异地登陆同步
    connect(tcp,&TcpThread::getHistoryMessage,this,&MainInterface::getHistoryMessage);
    connect(tcp,&TcpThread::getHistoryDynamic,this,&MainInterface::getHistoryDynamic);

    //每创建一个聊天界面都要绑定tcp线程
    connect(this,&MainInterface::connectChatWindow,tcp,[&](ChatWindow* chatWindow){
        connect(chatWindow,&ChatWindow::sendMessage,tcp,[&](int type, QString account, QString targetAccount, QString message,QString messageType)
            {tcp->getJsonData(type,account,targetAccount,message,messageType);});
    });

    //请求某人/某群的数据，搜索数据
    connect(this,&MainInterface::askForData,tcp,[&](int type,QString account,QString message){
        tcp->getJsonData(type,account,"",message);
    });
    connect(tcp,&TcpThread::getSearchOrAskData,this,&MainInterface::getSearchOrAskData);

    //掉线重连
    connect(tcp,&TcpThread::reconnection,this,&MainInterface::downLine);

    //发送好友申请，加群申请
    connect(this,&MainInterface::addFriend,tcp,[&](QString sender,QString receiver,QString result){
        tcp->getJsonData(InfoType::AddFriend,sender,receiver,result);
    });
    connect(this,&MainInterface::joinGroup,tcp,[&](QString sender,QString receiver,QString group,QString result){
        tcp->getJsonData(InfoType::JoinGroup,sender,receiver,result,group);
    });
    connect(tcp,&TcpThread::addFriend,this,[&](QJsonObject json){
        if(!addFriendJson)
        {
            addFriendJson=new QJsonArray();
        }
        //qDebug()<<"收到好友申请";
        addFriendJson->push_front(json);//使用头插法插入数据
        if(addWindow)
        {
            addWindow->initAddFriend();//更新模型和视图
        }
    });
    connect(tcp,&TcpThread::joinGroup,this,[&](QJsonObject json){
        if(!joinGroupJson)
        {
            joinGroupJson=new QJsonArray();
        }
        joinGroupJson->push_front(json);//使用头插法插入数据
        if(addWindow)
        {
            addWindow->initJoinGroup();//更新模型和视图
        }
    });

    //发布动态
    connect(this,&MainInterface::sendDynamic,tcp,[&](QString account,QString message){
        tcp->getJsonData(InfoType::SendDynamic,account,"",message,"");
    });
    connect(tcp,&TcpThread::getDynamic,this,&MainInterface::getDynamic);

    //通知重读数据表
    connect(tcp,&TcpThread::updateFriend,this,&MainInterface::setFriendView);
    connect(tcp,&TcpThread::updateGroup,this,&MainInterface::setGroupView);

    //创建群聊
    connect(this,&MainInterface::createGroup,tcp,[&](QString account,QString groupName,QString member){
        tcp->getJsonData(InfoType::CreateGroup,account,member,groupName,"");
    });

    //修改用户/群聊数据
    connect(this,&MainInterface::modifyUserData,tcp,[&](QString account,QString message,QString file){
        tcp->getJsonData(InfoType::ModifyUserData,account,"",message,"",file);
    });
    connect(this,&MainInterface::modifyGroupData,tcp,[&](QString account,QString message,QString file){
        tcp->getJsonData(InfoType::ModifyGroupData,account,"",message,"",file);
    });
    connect(tcp,&TcpThread::updateUserData,this,[&](){this->initUserData();this->initHead();});

    //tcp移入tcp线程，启动线程，并开始连接服务器
    tcp->moveToThread(tcpThread);
    tcpThread->start();
    emit connectToServer();
}

void MainInterface::setSystemTrayIcon()
{
    if(systemTray)
        return;
    systemTray=new QSystemTrayIcon();
    QIcon icon("://photo/AHU1.png");
    systemTray->setIcon(icon);

    //给系统托盘添加菜单
    QMenu* menu=new QMenu(this);
    QAction* normal=new QAction("显示主界面");
    QAction* quit=new QAction("退出程序");
    menu->addAction(normal);
    menu->addSeparator();
    menu->addAction(quit);
    menu->setFont(QFont("Microsoft YaHei", 10));
    systemTray->setContextMenu(menu);

    //给对应的action绑定处理函数
    connect(normal,&QAction::triggered,this,[&](){this->showNormal();});
    connect(quit,&QAction::triggered,this,[&](){QApplication::quit();});

    //绑定系统托盘图标的激活信号和处理函数
    connect(systemTray,&QSystemTrayIcon::activated,this,[&](QSystemTrayIcon::ActivationReason reason)->void{

        if(reason==QSystemTrayIcon::Trigger||reason==QSystemTrayIcon::DoubleClick)
        {
            this->show();
            this->showNormal();
            //窗体获取焦点，显示在最上层
            this->activateWindow();
            //模拟鼠标进入事件
            QApplication::postEvent(static_cast<QObject*>(this),new QEvent(QEvent::Enter));
        }
        else
            return;
        //this->raise();

    });
}

void MainInterface::initUserData()
{
    QString fileName=Protocol::getUserLoginFile();
    QFile file(fileName);
    if(!file.open(QFile::ReadOnly))
    {
        //qDebug()<<fileName<<"打开失败";
        return;
    }
    QByteArray data=file.readAll();
    QJsonObject json=QJsonDocument::fromJson(data).object();
    Protocol::initUserInfo(json);
}

void MainInterface::initHead()
{
    //根据存储在协议类中的静态数据来初始化主界面头部
    //qDebug()<<Protocol::getUserHeadShot();
    QString headPhoto;
    if(Protocol::getUserHeadShot().isEmpty())
        headPhoto=DefaultPixmap;
    else
        headPhoto=Protocol::getAllUserPath()+"/"+Protocol::getUserHeadShot();
    //qDebug()<<"headPhoto:"<<headPhoto;
    ui->head_btn->setIcon(Protocol::createHeadShot(headPhoto,ui->head_btn->size().width()/2));
    ui->user_name->setText(Protocol::getUserName());
    if(Protocol::getSignature()=="")
    {
        ui->signature->setText("编辑个性签名");
    }
    else
    {
        ui->signature->setText(Protocol::getSignature());
    }
}

void MainInterface::initMessageHash()
{
    //不为空则清空
    if(!messageHash.empty())
    {
        QStringList list=messageHash.keys();
        for(QString str:list)
        {
            delete messageHash.value(str);
        }
        messageHash.clear();
    }
    QString path=Protocol::getUserPath()+"/message";
    QDir dir;
    dir.setPath(path);
    QStringList list=dir.entryList(QDir::Files);
    for(auto i:list)
    {
        QFile file(path+"/"+i);
        //qDebug()<<i.split(".").first();
        file.open(QFile::ReadOnly);
        QJsonArray data=QJsonDocument::fromJson(file.readAll()).array();
        messageHash.insert(i.split(".").first(),new QJsonArray(data));
        file.close();
    }
}

void MainInterface::initAddAndJoin()
{
    if(!addFriendJson)
        addFriendJson=new QJsonArray();
    else
    {
        for(int i=0;i<addFriendJson->size();i++)
        {
            addFriendJson->removeFirst();
        }
    }
    if(!joinGroupJson)
        joinGroupJson=new QJsonArray();
    else
    {
        for(int i=0;i<joinGroupJson->size();i++)
        {
            joinGroupJson->removeFirst();
        }
    }
    QDir dir;
    if(dir.exists(Protocol::getUserPath()+"/addFriend.json"))
    {
        QFile file(Protocol::getUserPath()+"/addFriend.json");
        file.open(QFile::ReadOnly);
        *addFriendJson=QJsonDocument::fromJson(file.readAll()).array();
        file.close();
    }
    if(dir.exists(Protocol::getUserPath()+"/joinGroup.json"))
    {
        QFile file(Protocol::getUserPath()+"/joinGroup.json");
        file.open(QFile::ReadOnly);
        *joinGroupJson=QJsonDocument::fromJson(file.readAll()).array();
        file.close();
    }
}

void MainInterface::loginSuccess()
{
    initUserData();
    initHead();
    setSystemTrayIcon();
    systemTray->show();
    //qDebug()<<"打开主界面";
    initMessageHash();
    initAddAndJoin();
    //设置视图
    setView();
    this->show();
}

void MainInterface::getHistoryMessage()
{
    QDir dir;
    //设置dir路径，并获取所有的时间戳文件夹名
    dir.setPath(Protocol::getUserPath()+"/historyMessage");
    QStringList message=dir.entryList(QDir::Dirs);
    //去除垃圾数据
    message.removeOne(".");
    message.removeOne("..");
    //把时间戳排序一下，从小到大开始读数据
    QVector<qint64> timeSort;
    for(QString i:message)
    {
        timeSort.push_back(i.toLongLong());
    }
    std::sort(timeSort.begin(),timeSort.end());

    //遍历每一个时间戳文件夹下的所有消息文件
    for(qint64 num:timeSort)
    {
        //获取当前时间戳文件夹下的所有文件名
        QDir dir;
        dir.setPath(Protocol::getUserPath()+"/historyMessage/"+QString::number(num));
        QStringList allFile=dir.entryList(QDir::Files);
        //去除垃圾数据
        allFile.removeOne(".");
        allFile.removeOne("..");

        //遍历将数据写入messageHash中对应的QJsonArray
        for(QString fileName:allFile)
        {
            QFile file(dir.path()+"/"+fileName);
            file.open(QFile::ReadOnly);
            QJsonArray array=QJsonDocument::fromJson(file.readAll()).array();
            QString account=fileName.split(".").first();
            if(!messageHash.contains(account))
                messageHash.insert(account,new QJsonArray(array));
            else
            {
                for(QJsonValue item:array)
                {
                    messageHash.value(account)->append(item);
                }
            }
        }
    }
    //读取结束后清空historyMessage
    dir.removeRecursively();
    setMessageView();//设置消息列表
}

void MainInterface::getHistoryDynamic()
{
    QDir dir;
    //设置dir路径，并获取所有的时间戳文件名
    dir.setPath(Protocol::getUserPath()+"/historyDynamic");
    QStringList message=dir.entryList(QDir::Files);
    //去除垃圾数据
    message.removeOne(".");
    message.removeOne("..");
    //把时间戳排序一下，从小到大开始读数据
    QVector<qint64> timeSort;
    for(QString i:message)
    {
        timeSort.push_back(i.split(".").first().toLongLong());
    }
    std::sort(timeSort.begin(),timeSort.end());

    //遍历每一个时间戳动态文件
    for(qint64 num:timeSort)
    {
        QFile file(dir.path()+"/"+QString::number(num)+".json");
        file.open(QFile::ReadOnly);
        QJsonArray array=QJsonDocument::fromJson(file.readAll()).array();
        for(QJsonValue i:array)
        {
            dynamicJson.push_front(i.toObject());
        }
        file.close();
    }
    //读取结束后清空historyDynamic
    dir.removeRecursively();

    //把所有数据写回dynamic文件，然后读取并设置视图
    QFile file(Protocol::getUserPath()+"/dynamic.json");
    file.open(QFile::WriteOnly);
    file.write(QJsonDocument(dynamicJson).toJson());
    file.close();
    setDynamicView();//设置动态列表
}

void MainInterface::setView()
{
    //设置视图的模型、代理、数据
    setFriendView();
    setGroupView();
    setDynamicView();
    setMessageView();
    //emit ui->view_stack->setCurrentIndex(0);
    emit ui->friend_btn->click();

}

void MainInterface::setMessageView()
{
    if(messageModel)
    {
        messageModel->clear();
    }
    else
    {
        //初始化模型指针
        messageModel=new QStandardItemModel(this);

        //初始化代理模型
        messageSortModel=new QSortFilterProxyModel(this);
        messageSortModel->setSourceModel(messageModel);//设置源数据模型
        messageSortModel->setDynamicSortFilter(true);//打开动态排序
        messageSortModel->setSortRole(Qt::UserRole+2);//设置排序角色
        messageSortModel->sort(0,Qt::DescendingOrder);//以第0列降序排序

        //为视图绑定模型和代理
        ui->message_list->setModel(messageSortModel);
        ui->message_list->setItemDelegate(new MessageDelegate(this));

        //为视图绑定双击处理函数，即双击选项创建并打开对应的聊天窗口
        connect(ui->message_list,&QListView::doubleClicked,this,[&](const QModelIndex &index){openChatWindow(index);});
    }

    for(QString key:messageHash.keys())
    {
        QJsonObject json=messageHash[key]->last().toObject();
        MessageItem messageItem;
        if(key[0]=='G')
        {
            messageItem={key,
                        groupHash.value(key)->value("userName").toString(),
                        groupHash.value(key)->value("headPhoto").toString(),
                        json.value("message").toString(),
                        json.value("time").toString().toLongLong(),
                        0};
        }
        else
        {
            messageItem={key,
                       friendHash.value(key)->value("userName").toString(),
                       friendHash.value(key)->value("headPhoto").toString(),
                       json.value("message").toString(),
                       json.value("time").toString().toLongLong(),
                       0};
        }
        QStandardItem* item=new QStandardItem();
        item->setData(QVariant::fromValue(messageItem),Qt::UserRole+1);
        item->setData(messageItem.time,Qt::UserRole+2);
        messageModel->appendRow(item);
    }

    //添加一些测试数据
//    MessageItem item1={"","user1","","nihaodsjdahkjsadkfjkfjhkadsfdhajhfkjhakjhfadskjhfkdsjhfkjh",123,12};
//    MessageItem item2={"","user2","","早安",QDateTime::currentMSecsSinceEpoch(),1};
//    MessageItem item3={"","user3","","hello world skfdhkhkjhkiu12345678987654321",789,0};
//    QStandardItem* item4=new QStandardItem();
//    QStandardItem* item5=new QStandardItem();
//    QStandardItem* item6=new QStandardItem();
//    item4->setData(QVariant::fromValue(item1),Qt::UserRole+1);
//    item5->setData(QVariant::fromValue(item2),Qt::UserRole+1);
//    item6->setData(QVariant::fromValue(item3),Qt::UserRole+1);
//    item4->setData(item1.time,Qt::UserRole+2);
//    item5->setData(item2.time,Qt::UserRole+2);
//    item6->setData(item3.time,Qt::UserRole+2);
//    messageModel->appendRow(item4);
//    messageModel->appendRow(item5);
//    messageModel->appendRow(item6);

}

void MainInterface::setFriendView()
{
    if(friendModel)
    {
        friendModel->clear();
        for(QString key:friendHash.keys())
        {
            //不能直接删除，聊天界面还要靠这两个指针绘图，否则会越界访问崩溃的
            //delete friendHash[key];
        }
        friendHash.clear();
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
        ui->friend_list->setModel(friendSortModel);
        ui->friend_list->setItemDelegate(new MessageDelegate(this,MessageDelegateType::RelationType));

        //为视图绑定双击处理函数，即双击选项创建并打开对应的聊天窗口
        connect(ui->friend_list,&QListView::doubleClicked,this,[&](const QModelIndex &index){openChatWindow(index);});
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
        //存储好友信息
        friendHash.insert(jsonObj.value("account").toString(),new QJsonObject(jsonObj));
        //将需要显示的好友信息添加进模型中
        MessageItem message={jsonObj.value("account").toString(),jsonObj.value("userName").toString(),jsonObj.value("headPhoto").toString(),jsonObj.value("signature").toString(),0,0};
        item->setData(QVariant::fromValue(message),Qt::UserRole+1);
        //添加排序角色
        item->setData(jsonObj.value("userName").toString(),Qt::UserRole+2);
        friendModel->appendRow(item);
    }
}

void MainInterface::setGroupView()
{
    if(groupModel)
    {
        //清空模型和哈希表数据
        groupModel->clear();
        for(QString key:groupHash.keys())
        {
            //不能直接删除，聊天界面还要靠这两个指针绘图，否则会越界访问崩溃的
            //delete groupHash[key];
        }
        groupHash.clear();
        for(QString key:groupMemberHash.keys())
        {
            //delete groupMemberHash[key];
        }
        groupMemberHash.clear();
    }
    else
    {
        //初始化model指针
        groupModel=new QStandardItemModel(this);

        //设置排序代理模型
        groupSortModel=new QSortFilterProxyModel(this);
        groupSortModel->setSourceModel(groupModel);//设置源数据模型
        groupSortModel->setDynamicSortFilter(true);//打开动态排序
        groupSortModel->setSortRole(Qt::UserRole+2);//设置排序角色
        groupSortModel->sort(0,Qt::AscendingOrder);//以第0列升序排序

        //为视图设置模型和代理
        ui->group_list->setModel(groupSortModel);
        ui->group_list->setItemDelegate(new MessageDelegate(this,MessageDelegateType::RelationType));

        //为视图绑定双击处理函数，即双击选项创建并打开对应的聊天窗口
        connect(ui->group_list,&QListView::doubleClicked,this,[&](const QModelIndex &index){openChatWindow(index);});
    }

    //打开对应的群聊信息文件
    QFile file(Protocol::getUserPath()+"/group.json");
    file.open(QFile::ReadOnly);
    QByteArray data=file.readAll();
    QJsonDocument jsonDoc=QJsonDocument::fromJson(data);
    QJsonArray jsonArray=jsonDoc.array();
    for(auto i:jsonArray)
    {
        QStandardItem* item=new QStandardItem();
        QJsonObject jsonObj=i.toObject();
        QString account=jsonObj.value("account").toString();
        //存储群聊信息和群聊成员信息
        if(groupHash.contains(account))
        {
            qDebug()<<"groupHash包含该项";
            //groupHash[account]=
        }
        groupHash.insert(jsonObj.value("account").toString(),new QJsonObject(jsonObj));
        groupMemberHash.insert(jsonObj.value("account").toString(),new QJsonArray(jsonObj.value("member").toArray()));

        //将需要显示的群聊信息添加进模型中
        MessageItem message={jsonObj.value("account").toString(),jsonObj.value("userName").toString(),jsonObj.value("headPhoto").toString(),jsonObj.value("signature").toString(),0,0};
        item->setData(QVariant::fromValue(message),Qt::UserRole+1);
        //添加排序角色
        item->setData(jsonObj.value("userName").toString(),Qt::UserRole+2);
        groupModel->appendRow(item);
    }
}

void MainInterface::setDynamicView()
{
    if(dynamicModel)
    {
        //清空模型和数据表
        groupModel->clear();
        int size=dynamicJson.size();
        for(int i=0;i<size;i++)
        {
            dynamicJson.removeFirst();
        }
    }
    else
    {
        //初始化model指针
        dynamicModel=new QStandardItemModel(this);

        //设置排序代理模型
        dynamicSortModel=new QSortFilterProxyModel(this);
        dynamicSortModel->setSourceModel(dynamicModel);//设置源数据模型
        dynamicSortModel->setDynamicSortFilter(true);//打开动态排序
        dynamicSortModel->setSortRole(Qt::UserRole+2);//设置排序角色
        dynamicSortModel->sort(0,Qt::DescendingOrder);//以第0列升序排序

        //为视图设置模型和代理
        ui->dynamic_list->setModel(dynamicSortModel);
        //需要重写代理
        ui->dynamic_list->setItemDelegate(new DynamicDelegate(this));

    }
    //打开动态文件
    QFile file(Protocol::getUserPath()+"/dynamic.json");
    if(!file.open(QFile::ReadOnly))
        return;
    QByteArray data=file.readAll();
    QJsonDocument jsonDoc=QJsonDocument::fromJson(data);
    QJsonArray jsonArray=jsonDoc.array();
    for(auto i:jsonArray)
    {
        QStandardItem* item=new QStandardItem();
        QJsonObject jsonObj=i.toObject();
        //存储好友信息
        dynamicJson.append(jsonObj);
        //将需要显示的个人动态信息添加进模型中
        QString account=jsonObj.value("account").toString();
        MessageItem message;
        if(account==Protocol::getUserAccount())
        {
            message=MessageItem{account,Protocol::getUserName(),Protocol::getUserHeadShot(),
                       jsonObj.value("message").toString(),
                       jsonObj.value("time").toString().toLongLong(),
                       0};
        }
        else
        {
            message=MessageItem{account,friendHash.value(account)->value("userName").toString(),
                       friendHash.value(account)->value("headPhoto").toString(),
                       jsonObj.value("message").toString(),
                       jsonObj.value("time").toString().toLongLong(),
                       0};
        }
        item->setData(QVariant::fromValue(message),Qt::UserRole+1);
        //添加排序角色
        item->setData(jsonObj.value("time").toString().toLongLong(),Qt::UserRole+2);
        dynamicModel->appendRow(item);
    }
}

void MainInterface::getMessage(QString account,QJsonObject message)
{
    QJsonObject json;
    //如果发送发是群聊，那么真实的发送者是messageType
    if(message.value("sender").toString()[0]=="G")
    {
        json.insert("account",message.value("messageType").toString());
    }
    else
    {
        json.insert("account",message.value("sender").toString());
    }
    json.insert("message",message.value("message").toString());
    json.insert("time",message.value("time").toString());

    if(!messageHash.contains(account))
        messageHash.insert(account,new QJsonArray());
    //添加到消息缓存末尾
    messageHash.value(account)->append(json);
    //真正的发送者
    QString sender=json.value("account").toString();
    //判断消息列表中是否存在该项，没有则创建，有则更新信息
    int pos=getMessageItemIndex(account);
    if(pos<0)
    {
        QStandardItem* item=new QStandardItem();
        MessageItem messageItem={account,friendHash.value(account)->value("userName").toString(),
                                    friendHash.value(account)->value("headPhoto").toString(),
                                    json.value("message").toString(),
                                    json.value("time").toString().toLongLong(),
                                    1};
        if(sender==Protocol::getUserAccount())
            messageItem.sum=0;
        item->setData(QVariant::fromValue(messageItem),Qt::UserRole+1);
        item->setData(messageItem.time,Qt::UserRole+2);
        messageModel->appendRow(item);
    }
    else
    {
        MessageItem messageItem=messageModel->item(pos)->data(Qt::UserRole+1).value<MessageItem>();
        messageItem.message=json.value("message").toString();
        messageItem.time=json.value("time").toString().toLongLong();
        if(sender!=Protocol::getUserAccount())
            messageItem.sum++;
        messageModel->item(pos)->setData(QVariant::fromValue(messageItem),Qt::UserRole+1);
        messageModel->item(pos)->setData(messageItem.time,Qt::UserRole+2);
    }

    //若该聊天界面已存在，则更新模型信息
    if(chatWindowList.contains(account))
    {
        //函数参数中的account仅仅是用来判断该条信息应该转发给哪个聊天界面，并不一定就一定是发送者
        //即自己发送的信息account也是接收者
        chatWindowList.value(account)->addMessage(sender,message);
    }
}

int MainInterface::getMessageItemIndex(QString account)
{
    for(int i=0;i<messageModel->rowCount();i++)
    {
        if(messageModel->item(i)->data(Qt::UserRole+1).value<MessageItem>().account==account)
            return i;
    }
    return -1;
}

void MainInterface::getDynamic(QJsonObject json)
{
    //json已经在服务端处理好了，可以直接读
    json.remove("type");//移除垃圾数据
    dynamicJson.push_front(json);
    QStandardItem* item=new QStandardItem();
    MessageItem messageItem={json.value("account").toString(),
                            json.value("userName").toString(),
                            json.value("headPhoto").toString(),
                            json.value("message").toString(),
                            json.value("time").toString().toLongLong()
                           };
    item->setData(QVariant::fromValue(messageItem),Qt::UserRole+1);
    item->setData(json.value("time").toString().toLongLong(),Qt::UserRole+2);
    dynamicModel->appendRow(item);
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
        //pressPoint=event->globalPos()-pressPoint;
        //qDebug()<<pressPoint;
        //isInScreen();
        this->move(moveTo(event->globalPos()-pressPoint));
        isHide=!isInScreen();
    }
}

QPoint MainInterface::moveTo(QPoint point)
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

bool MainInterface::isInScreen()
{
    if(this->pos().y()==0)
        return false;
    else
        return true;
}

void MainInterface::hideWindow()
{
    QPropertyAnimation * animation = new QPropertyAnimation(this, "geometry");
    animation->setStartValue(QRect(x(),y(),width(),height()));
    animation->setEndValue(QRect(x(),2-height(),width(),height()));
    animation->setDuration(250);
    animation->start();
}

void MainInterface::showWindow()
{
    QPropertyAnimation * animation = new QPropertyAnimation(this, "geometry");
    animation->setStartValue(QRect(x(),y(),width(),height()));
    animation->setEndValue(QRect(x(),0,width(),height()));
    animation->setDuration(250);
    animation->start();
}

void MainInterface::enterEvent(QEvent *event)
{
    //qDebug()<<"进入主界面";
    if(isHide)
    {
        showWindow();
    }
}

void MainInterface::leaveEvent(QEvent *event)
{
    //qDebug()<<"离开主界面";

    //因为ui->nagetive的焦点穿透问题，要加一个判断鼠标是否还在主界面区域上的判断
    //目前还没找到解决焦点穿透问题的方法，不知道它设置了什么鬼东西，我是真的懒得去看它的私密类了
    if(this->geometry().contains(QCursor::pos()))
    {
        return;
    }
    if(isHide)
    {
        hideWindow();
        //qDebug()<<this->pos();
    }
}

void MainInterface::openChatWindow(const QModelIndex& index)
{
    ChatWindow* chatWindow=nullptr;
    QString account=index.data(Qt::UserRole+1).value<MessageItem>().account;
    //如果这个聊天窗口不存在则创建
    if(!chatWindowList.contains(account))
    {
        chatWindow=new ChatWindow();
        chatWindowList.insert(account,chatWindow);
        qDebug()<<"创建聊天界面";
        //绑定来聊天界面和个人界面
        connectChatWindowToPersonal(chatWindow);

        //绑定与tcp线程的通信信号
        emit connectChatWindow(chatWindow);

        //关闭时清除哈希表指针
        connect(chatWindow,&ChatWindow::closeChatWindow,this,[&](QString account){
            chatWindowList.remove(account);
        });

        //不存在对应的聊天记录则创建
        if(!messageHash.contains(account))
            messageHash.insert(account,new QJsonArray());
        //初始化聊天界面，写入聊天记录
        if(account[0]=='G')
        {
            //qDebug()<<"打开群聊";
            chatWindow->initUserData(groupHash.value(account),groupMemberHash.value(account),messageHash.value(account));
        }
        else
        {
            //qDebug()<<"打开个人";
            chatWindow->initUserData(friendHash.value(account),messageHash.value(account));
        }
    }
    else
        chatWindow=chatWindowList.value(account);

    //判断点击项是来自messageList还是friendList/groupList，将messageList中对应的项的sum设置为0
    if(index.model()==messageSortModel)
    {
        //因为messageList设置的model是代理model，所以在这使用的是messageSortModel，反正能映射过去
        MessageItem messageItem=index.data(Qt::UserRole+1).value<MessageItem>();
        messageItem.sum=0;
        messageSortModel->setData(index,QVariant::fromValue(messageItem),Qt::UserRole+1);
    }
    else
    {
        QString str=index.data(Qt::UserRole+1).value<MessageItem>().account;
        int pos=getMessageItemIndex(str);
        QModelIndex temp=messageModel->index(pos,0);
        MessageItem messageItem=temp.data(Qt::UserRole+1).value<MessageItem>();
        messageItem.sum=0;
        messageModel->setData(temp,QVariant::fromValue(messageItem),Qt::UserRole+1);
    }
    //将聊天界面视图滑动条移动到底部
    chatWindow->openChatWIndow();
    //显示聊天界面并正常化
    chatWindow->show();
    chatWindow->showNormal();
    chatWindow->activateWindow();
}

void MainInterface::openChatWindow(QString account)
{
    ChatWindow* chatWindow=nullptr;
    //如果这个聊天窗口不存在则创建
    if(!chatWindowList.contains(account))
    {
        chatWindow=new ChatWindow();
        chatWindowList.insert(account,chatWindow);
        qDebug()<<"创建聊天界面";

        //绑定来聊天界面和个人界面
        connectChatWindowToPersonal(chatWindow);

        //绑定与tcp线程的通信信号
        emit connectChatWindow(chatWindow);

        //关闭时清除哈希表指针
        connect(chatWindow,&ChatWindow::closeChatWindow,this,[&](QString account){
            chatWindowList.remove(account);
        });

        //不存在对应的聊天记录则创建
        if(!messageHash.contains(account))
            messageHash.insert(account,new QJsonArray());
        //初始化聊天界面，写入聊天记录
        if(account[0]=='G')
        {
            //qDebug()<<"打开群聊";
            chatWindow->initUserData(groupHash.value(account),groupMemberHash.value(account),messageHash.value(account));
        }
        else
        {
            //qDebug()<<"打开个人";
            chatWindow->initUserData(friendHash.value(account),messageHash.value(account));
        }
    }
    else
        chatWindow=chatWindowList.value(account);

    //将对应消息列表的sum设置为零
    for(int i=0;i<messageModel->rowCount();i++)
    {
        QStandardItem* item=messageModel->item(i,0);
        if(item->data(Qt::UserRole+1).value<MessageItem>().account==account)
        {
            MessageItem message=item->data((Qt::UserRole+1)).value<MessageItem>();
            message.sum=0;
            item->setData(QVariant::fromValue(message));
            break;
        }
    }

    //显示三件套
    chatWindow->show();
    chatWindow->showNormal();
    chatWindow->activateWindow();
}

void MainInterface::openPersonal(QString account)
{
    Personal* personal=nullptr;
    qDebug()<<"打开："<<account;
    if(!personalList.contains(account))
    {
        //该界面是自己或者好友或者加入的群聊
        if(friendHash.contains(account)||groupHash.contains(account)||account==Protocol::getUserAccount())
        {
            QJsonObject* jsonObj=nullptr;
            QJsonArray* jsonArray=nullptr;
            personal=new Personal();
            personalList.insert(account,personal);
            //绑定个人界面功能
            connectPersonalFunction(personal);
            //绑定个人界面和聊天界面
            connectPersonalToChatWindow(personal);

            //自己的个人主页
            if(account==Protocol::getUserAccount())
            {
                //写入自己的信息
                jsonObj=new QJsonObject();
                jsonObj->insert("account",Protocol::getUserAccount());
                jsonObj->insert("age",Protocol::getAge());
                jsonObj->insert("birthday",Protocol::getBirthday());
                jsonObj->insert("bloodType",Protocol::getBloodType());
                jsonObj->insert("college",Protocol::getCollege());
                jsonObj->insert("headPhoto",Protocol::getUserHeadShot());
                jsonObj->insert("location",Protocol::getLocation());
                jsonObj->insert("sex",Protocol::getSex());
                jsonObj->insert("signature",Protocol::getSignature());
                jsonObj->insert("userName",Protocol::getUserName());
                personal->initPersonal(jsonObj,nullptr,PersonalType::MyselfType);
            }
            else
            {
                //用户类型
                if(account[0]=='E'||account[0]=='T')
                {
                    jsonObj=friendHash.value(account);
                    personal->initPersonal(jsonObj,nullptr,PersonalType::FriendType);
                }
                //群聊类型
                else
                {
                    jsonObj=groupHash.value(account);
                    jsonArray=groupMemberHash.value(account);
                    //判断是不是我创建的群聊
                    if(jsonObj->value("hostAccount").toString()==Protocol::getUserAccount())
                        personal->initPersonal(jsonObj,jsonArray,PersonalType::MyGroupType);
                    else
                        personal->initPersonal(jsonObj,jsonArray,PersonalType::JoinedGroupType);
                }
            }
        }
        //判断是否在已缓存的其他信息中
        else if(otherUserHash.contains(account)||otherGroupHash.contains(account))
        {
            QJsonObject* jsonObj=nullptr;
            QJsonArray* jsonArray=nullptr;
            personal=new Personal();
            personalList.insert(account,personal);
            //绑定个人界面功能
            connectPersonalFunction(personal);
            //绑定个人界面和聊天界面
            connectPersonalToChatWindow(personal);

            //如果是用户类型
            if(otherUserHash.contains(account))
            {
                jsonObj=otherUserHash.value(account);
                personal->initPersonal(jsonObj,nullptr,PersonalType::UserType);
            }
            //如果是群聊类型
            else
            {
                jsonObj=otherGroupHash.value(account);
                jsonArray=otherGroupMemberHash.value(account);
                personal->initPersonal(jsonObj,jsonArray,PersonalType::OtherGroupType);
            }

        }
        //否则要向服务器请求
        else
        {
            //判断请求类型
            if(account[0]=='G')
            {
                emit askForData(InfoType::AskForGroupData,Protocol::getUserAccount(),account);
            }
            else
            {
                emit askForData(InfoType::AskForUserData,Protocol::getUserAccount(),account);
            }
        }
    }
    //如果已经缓存了该界面，直接获取
    else
        personal=personalList.value(account);

    if(!personal)
        return;

    //经典显示三件套
    personal->show();
    personal->showNormal();
    personal->activateWindow();
}

void MainInterface::openSearchWindow()
{
    if(!searchWindow)
    {
        searchWindow=new SearchWindow();
        //搜索用户
        connect(searchWindow,&SearchWindow::searchUser,this,[&](QString message){
            emit this->askForData(InfoType::SearchUser,Protocol::getUserAccount(),message);
        });
        //搜索群聊
        connect(searchWindow,&SearchWindow::searchGroup,this,[&](QString message){
            emit this->askForData(InfoType::SearchGroup,Protocol::getUserAccount(),message);
        });
        //打开个人主页
        connect(searchWindow,&SearchWindow::openPersonal,this,[&](QString account){
            openPersonal(account);
        });
        //关闭搜索界面
        connect(searchWindow,&SearchWindow::destroyed,this,[&](){
            searchWindow=nullptr;
        });
    }
    searchWindow->show();
    searchWindow->showNormal();
    searchWindow->activateWindow();
}

void MainInterface::openAddWindow()
{
    //初始化界面指针
    if(!addWindow)
    {
        addWindow=new AddWindow(addFriendJson,joinGroupJson);
        //添加好友
        connect(addWindow,&AddWindow::addFriend,this,[&](QString account){
            emit addFriend(Protocol::getUserAccount(),account,"agree");
        });
        //加入群聊
        connect(addWindow,&AddWindow::joinGroup,this,[&](QString account,QString group){
            emit joinGroup(Protocol::getUserAccount(),account,group,"agree");
        });
        //关闭添加界面
        connect(addWindow,&AddWindow::destroyed,this,[&](){addWindow=nullptr;});
    }
    //显示三件套
    addWindow->show();
    addWindow->showNormal();
    addWindow->activateWindow();
}

void MainInterface::openEditDynamicWindow()
{
    if(!editDynamicWindow)
    {
        editDynamicWindow=new EditDynamicWindow();
        connect(editDynamicWindow,&EditDynamicWindow::destroyed,this,[&](){
            editDynamicWindow=nullptr;
        });
        connect(editDynamicWindow,&EditDynamicWindow::sendDynamic,this,[&](QString message){
            emit sendDynamic(Protocol::getUserAccount(),message);
        });
    }
    //显示三件套
    editDynamicWindow->show();
    editDynamicWindow->showNormal();
    editDynamicWindow->activateWindow();
}

void MainInterface::openCreateGroupWindow()
{
    if(!createGroupWindow)
    {
        createGroupWindow=new CreateGroupWindow();
        connect(createGroupWindow,&CreateGroupWindow::destroyed,this,[&](){
            createGroupWindow=nullptr;
        });
        connect(createGroupWindow,&CreateGroupWindow::createGroup,this,[&](QString groupName,QString member){
            emit createGroup(Protocol::getUserAccount(),groupName,member);
        });
        connect(createGroupWindow,&CreateGroupWindow::openPersonal,this,[&](QString account){
            openPersonal(account);
        });
    }
    createGroupWindow->show();
    createGroupWindow->showNormal();
    createGroupWindow->activateWindow();
}

void MainInterface::connectChatWindowToPersonal(ChatWindow *chatWindow)
{
    connect(chatWindow,&ChatWindow::openPersonal,this,&MainInterface::openPersonal);
}

void MainInterface::connectPersonalToChatWindow(Personal *personal)
{
    connect(personal,&Personal::openChatWindow,this,[&](QString account){openChatWindow(account);});
}

void MainInterface::connectPersonalFunction(Personal *personal)
{
    //绑定Personal的销毁信号和处理函数
    connect(personal,&Personal::closePersonal,this,[&](QString senderAccount){
        //因为使用lambda表达式作为槽函数导致sender()无法获取到正确的发送者指针，所以将资源的释放留在了Personal内，
        //由它自己完成，这里只负责清除哈希表
        //Personal* sender=static_cast<Personal*>(this->sender());
        personalList.remove(senderAccount);//从映射表中删除该项
        //                if(!sender)
        //                {
        //                    qDebug()<<"这是一个空指针";
        //                }
        //sender->deleteLater();//释放资源
        //qDebug()<<senderAccount<<"个人界面已经释放";
    });
    connect(personal,&Personal::addFriend,this,[&](QString account){
        //qDebug()<<"请求添加好友："<<account;
        emit addFriend(Protocol::getUserAccount(),account,"apply");
    });
    connect(personal,&Personal::joinGroup,this,[&](QString account,QString group){
        //qDebug()<<"请求加入群聊："<<account;
        emit joinGroup(Protocol::getUserAccount(),account,group,"apply");
    });
    connect(personal,&Personal::openPersonal,this,&MainInterface::openPersonal);
    connect(personal,&Personal::modifyData,this,[&](QString account,QString message,QString file){
        if(account[0]=='G')
            emit modifyGroupData(account,message,file);
        else
            emit modifyUserData(account,message,file);
    });
}

void MainInterface::downLine()
{
    writeLoginFile();
    writeFriendFile();
    writeMessageFile();
    writeDynamicFile();
    writeAddFriendFile();
    writeJoinGroupFile();
}

void MainInterface::writeLoginFile()
{
    QJsonObject json;
    json.insert("account",Protocol::getUserAccount());
    json.insert("age",Protocol::getAge());
    json.insert("autoLogin",Protocol::isAutoLogin);
    json.insert("birthday",Protocol::getBirthday());
    json.insert("bloodType",Protocol::getBloodType());
    json.insert("college",Protocol::getCollege());
    json.insert("headPhoto",Protocol::getUserHeadShot());
    json.insert("lastLoginTime",QString::number(Protocol::getLoginTime()));
    json.insert("location",Protocol::getLocation());
    json.insert("password",Protocol::getUserPwd());
    json.insert("profession",Protocol::getProfession());
    json.insert("remember",Protocol::isRemember);
    json.insert("sex",Protocol::getSex());
    json.insert("signature",Protocol::getSignature());
    json.insert("userName",Protocol::getUserName());
    QFile file(Protocol::getUserPath()+"/login.json");
    file.open(QFile::WriteOnly|QFile::Truncate);
    file.write(QJsonDocument(json).toJson());
    file.close();
}

void MainInterface::writeFriendFile()
{
    QJsonArray friendArray;
    for(QJsonObject* data:friendHash)
    {
        friendArray.append(*data);
        //qDebug()<<data->value("account").toString();
    }
    QFile file(Protocol::getUserPath()+"/friend.json");
    //qDebug()<<file.fileName();
    file.open(QFile::WriteOnly|QFile::Truncate);
    file.write(QJsonDocument(friendArray).toJson());
    file.close();
}

void MainInterface::writeMessageFile()
{
    for(QString account:messageHash.keys())
    {
        QByteArray data=QJsonDocument(*(messageHash.value(account))).toJson();
        QFile file(Protocol::getUserPath()+"/message/"+account+".json");
        qDebug()<<file.fileName();
        file.open(QFile::WriteOnly|QFile::Truncate);
        file.write(data);
        file.close();
    }
}

void MainInterface::writeDynamicFile()
{
    QFile file(Protocol::getUserPath()+"/dynamic.json");
    file.open(QFile::WriteOnly);
    file.write(QJsonDocument(dynamicJson).toJson());
    file.close();
}

void MainInterface::writeAddFriendFile()
{
    QFile file(Protocol::getUserPath()+"/addFriend.json");
    file.open(QFile::WriteOnly);
    file.write(QJsonDocument(*addFriendJson).toJson());
    file.close();
}

void MainInterface::writeJoinGroupFile()
{
    QFile file(Protocol::getUserPath()+"/joinGroup.json");
    file.open(QFile::WriteOnly);
    file.write(QJsonDocument(*joinGroupJson).toJson());
    file.close();
}

void MainInterface::closeAllWindow()
{
    this->close();
    systemTray->hide();
    if(searchWindow)
    {
        searchWindow->close();
        searchWindow->deleteLater();
    }
    if(addWindow)
    {
        addWindow->close();
        addWindow->deleteLater();
    }
    if(editDynamicWindow)
    {
        editDynamicWindow->close();
        editDynamicWindow->deleteLater();
    }
    for(QString key:chatWindowList.keys())
    {
        chatWindowList[key]->close();
        chatWindowList[key]->deleteLater();
    }
    chatWindowList.clear();
    for(QString key:personalList.keys())
    {
        personalList[key]->close();
        personalList[key]->deleteLater();
    }
    personalList.clear();
}

void MainInterface::getSearchOrAskData(int type)
{
    switch(type)
    {
        case SearchUser:
        {
            if(!searchUserArray)
            {
                searchUserArray=new QJsonArray();
            }
            //清空原有数据
            int size=searchUserArray->size();
            for(int i=0;i<size;i++)
            {
                searchUserArray->removeFirst();
            }

            //打开文件读取数据
            QFile file(Protocol::getUserPath()+"/searchUser.json");
            if(!file.open(QFile::ReadOnly))
                return;
            (*searchUserArray)=QJsonDocument::fromJson(file.readAll()).array();

            //如果搜索窗口存在则初始化
            if(searchWindow)
            {
                searchWindow->initSearchUser(searchUserArray);
            }
            break;
        }
        case SearchGroup:
        {
            if(!searchGroupArray)
                searchGroupArray=new QJsonArray();
            //清空原有数据
            int size=searchGroupArray->size();
            for(int i=0;i<size;i++)
            {
                searchGroupArray->removeFirst();
            }

            //打开文件读取数据
            QFile file(Protocol::getUserPath()+"/searchGroup.json");
            if(!file.open(QFile::ReadOnly))
                return;
            (*searchGroupArray)=QJsonDocument::fromJson(file.readAll()).array();

            //如果搜索窗口存在则初始化
            if(searchWindow)
            {
                searchWindow->initSearchGroup(searchGroupArray);
            }
            break;
        }
        case AskForGroupData:
        {
            //打开文件读取数据
            QFile file(Protocol::getUserPath()+"/askForGroupData.json");
            if(!file.open(QFile::ReadOnly))
                return;
            QJsonObject* json=new QJsonObject(QJsonDocument::fromJson(file.readAll()).object());
            QJsonArray* jsonArray=new QJsonArray(json->value("member").toArray());
            QString account=json->value("account").toString();

            //判断是否存在旧的群聊数据，有则更新
            if(otherGroupHash.contains(account))
            {
                delete otherGroupHash.value(account);
                otherGroupHash[account]=json;
            }
            else
            {
                otherGroupHash.insert(account,json);
            }

            //判断是否存在旧的群聊成员数据，有则更新
            if(otherGroupMemberHash.contains(account))
            {
                delete otherGroupMemberHash.value(account);
                otherGroupMemberHash[account]=jsonArray;
            }
            else
            {
                otherGroupMemberHash.insert(account,jsonArray);
            }

            //写入其他用户，并判断是否存在旧的用户数据，有则更新
            int size=jsonArray->size();
            for(int i=0;i<size;i++)
            {
                QJsonObject* jsonObj=new QJsonObject(jsonArray->at(i).toObject());
                QString account=jsonObj->value("account").toString();
                if(otherUserHash.contains(account))
                {
                    delete otherUserHash.value(account);
                    otherUserHash[account]=jsonObj;
                }
                else
                {
                    otherUserHash.insert(account,jsonObj);
                }
            }

            //打开个人界面
            openPersonal(account);
            break;
        }
        case AskForUserData:
        {
            //打开文件读取数据
            QFile file(Protocol::getUserPath()+"/askForUserData.json");
            if(!file.open(QFile::ReadOnly))
                return;
            QJsonObject* json=new QJsonObject(QJsonDocument::fromJson(file.readAll()).object());
            QString account=json->value("account").toString();

            //写入其他用户，并判断是否存在旧的用户数据，有则更新
            if(otherUserHash.contains(account))
            {
                delete otherUserHash.value(account);
                otherUserHash[account]=json;
            }
            else
            {
                otherUserHash.insert(account,json);
            }

            //打开个人界面
            openPersonal(account);
            break;
        }
        default:
            break;
    }
}


