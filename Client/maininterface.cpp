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
        writeLoginFile();
        writeFriendFile();
        writeMessageFile();
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
    //设置主页面无边框和背景透明，不在任务栏中显示，并且出现在画面最上层
    setWindowFlags(Qt::FramelessWindowHint|Qt::SplashScreen);
    setAttribute(Qt::WA_TranslucentBackground);

    //添加导航栏标签
    ui->navigate->addTab("消息");
    ui->navigate->addTab("联系人");
    ui->navigate->addTab("动态");

    //添加图片资源
    ui->min_btn->setIcon(QIcon("://photo/min2.png"));
    ui->close_btn->setIcon(QIcon("://photo/close2.png"));
    ui->logo->setPixmap(QPixmap("://photo/AHU1.png").scaled(ui->logo->size().width(),ui->logo->size().height(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    ui->head_btn->setIcon(QIcon("://photo/AHU1.png"));
    ui->head_btn->setIconSize(QSize(ui->head_btn->size()));

    //设置tip
    ui->close_btn->setToolTip("关闭");
    ui->close_btn->setToolTipDuration(2000);
    ui->min_btn->setToolTip("最小化");
    ui->min_btn->setToolTipDuration(2000);

    //ui->navigate->setStyleSheet("border:none;");

}

void MainInterface::setConnect()
{
    //这里将主界面最小化不使用showMinimized()而是使用hide()
    //因为将主界面设置为SplashScreen后使用showMinimized()无法将界面全部关闭
    //可能是因为设置完毕后，窗体无法托管到任务栏，所以最小化失败
    connect(ui->min_btn,&QToolButton::clicked,this,[&](){this->hide();});
    connect(ui->close_btn,&QToolButton::clicked,this,[&](){this->close();});
    connect(ui->navigate,&QtMaterialTabs::currentChanged,this,[&](int index){ui->stackedWidget->setCurrentIndex(index);});
}

void MainInterface::setTcpThread()
{
    tcp=new TcpThread();
    tcpThread=new QThread();

    //传送登录信息
    connect(login,&Login::sendLogin,tcp,[this](QString account,QString password){
        tcp->getJsonData(InfoType::LoginAccount,account,"",password);
    });
    connect(tcp,&TcpThread::loginSuccess,this,[this](bool result){
        if(result)
        {
            Protocol::isLogin=true;//将登录标签位设置为true
            loginSuccess();//用户登录成功设置
            this->login->hide();
        }
        else
        {
            login->loginResult(0);
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

    //每创建一个聊天界面都要绑定tcp线程
    connect(this,&MainInterface::connectChatWindow,tcp,[&](ChatWindow* chatWindow){
        connect(chatWindow,&ChatWindow::sendMessage,tcp,[&](int type, QString account, QString targetAccount, QString message,QString messageType)
            {tcp->getJsonData(type,account,targetAccount,message,messageType);});
    });

    //tcp移入tcp线程，启动线程，并开始连接服务器
    tcp->moveToThread(tcpThread);
    tcpThread->start();
    emit connectToServer();
}

void MainInterface::setSystemTrayIcon()
{
    systemTray=new QSystemTrayIcon(this);
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
        if(reason==QSystemTrayIcon::Trigger)
            this->showNormal();
        else if(reason==QSystemTrayIcon::DoubleClick)
            this->showNormal();
        else
            return;
        //this->raise();
        //窗体获取焦点，显示在最上层
        this->activateWindow();
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
    qDebug()<<"headPhoto:"<<headPhoto;
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

void MainInterface::loginSuccess()
{
    initUserData();
    initHead();
    setSystemTrayIcon();
    systemTray->show();
    //qDebug()<<"打开主界面";
    initMessageHash();
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
}

void MainInterface::setView()
{
    //设置视图的模型、代理、数据
    setMessageView();
    setRelationView();
    //为视图绑定双击处理函数，即双击选项创建并打开对应的聊天窗口
    connect(ui->friend_list,&QListView::doubleClicked,this,&MainInterface::openChatWindow);
    connect(ui->message_list,&QListView::doubleClicked,this,&MainInterface::openChatWindow);
}

void MainInterface::setMessageView()
{
    //初始化模型指针
    messageModel=new QStandardItemModel(this);

    //初始化代理模型
    messageSortModel=new QSortFilterProxyModel(this);
    messageSortModel->setSourceModel(messageModel);//设置源数据模型
    messageSortModel->setDynamicSortFilter(true);//打开动态排序
    messageSortModel->setSortRole(Qt::UserRole+2);//设置排序角色
    messageSortModel->sort(0,Qt::DescendingOrder);//以第0列降序排序

    //添加一些测试数据
    MessageItem item1={"","user1","","nihaodsjdahkjsadkfjkfjhkadsfdhajhfkjhakjhfadskjhfkdsjhfkjh",123,12};
    MessageItem item2={"","user2","","早安",QDateTime::currentMSecsSinceEpoch(),1};
    MessageItem item3={"","user3","","hello world skfdhkhkjhkiu12345678987654321",789,0};
    QStandardItem* item4=new QStandardItem();
    QStandardItem* item5=new QStandardItem();
    QStandardItem* item6=new QStandardItem();
    item4->setData(QVariant::fromValue(item1),Qt::UserRole+1);
    item5->setData(QVariant::fromValue(item2),Qt::UserRole+1);
    item6->setData(QVariant::fromValue(item3),Qt::UserRole+1);
    item4->setData(item1.time,Qt::UserRole+2);
    item5->setData(item2.time,Qt::UserRole+2);
    item6->setData(item3.time,Qt::UserRole+2);
    messageModel->appendRow(item4);
    messageModel->appendRow(item5);
    messageModel->appendRow(item6);

    //为视图绑定模型和代理
    ui->message_list->setModel(messageSortModel);
    ui->message_list->setItemDelegate(new MessageDelegate(this));
}

void MainInterface::setRelationView()
{
    //初始化model指针
    relationModel=new QStandardItemModel(this);

    //设置排序代理模型
    relationSortModel=new QSortFilterProxyModel(this);
    relationSortModel->setSourceModel(relationModel);//设置源数据模型
    relationSortModel->setDynamicSortFilter(true);//打开动态排序
    relationSortModel->setSortRole(Qt::UserRole+2);//设置排序角色
    relationSortModel->sort(0,Qt::AscendingOrder);//以第0列升序排序

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
        relationModel->appendRow(item);
    }

    //为视图设置模型和代理
    ui->friend_list->setModel(relationSortModel);
    ui->friend_list->setItemDelegate(new MessageDelegate(this));
}

void MainInterface::getMessage(QString account,QJsonObject json)
{
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
        chatWindowList.value(account)->addMessage(sender,json);
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
        this->move(event->globalPos()-pressPoint);
    }
}

void MainInterface::openChatWindow(const QModelIndex& index)
{
    QString account=index.data(Qt::UserRole+1).value<MessageItem>().account;
    ChatWindow* chatWindow=nullptr;
    //如果这个聊天窗口不存在则创建
    if(!chatWindowList.contains(account))
    {
        chatWindow=new ChatWindow();
        chatWindowList.insert(account,chatWindow);
        qDebug()<<"创建聊天界面";

        //绑定与tcp线程的通信信号
        emit connectChatWindow(chatWindow);

        //不存在对应的聊天记录则创建
        if(!messageHash.contains(account))
            messageHash.insert(account,new QJsonArray());
        //初始化聊天界面，写入聊天记录
        chatWindow->initUserData(friendHash.value(account),messageHash.value(account));
    }
    else
        chatWindow=chatWindowList.value(account);

    //判断点击项是来自messageList还是friendList，将messageList中对应的项的sum设置为0
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


