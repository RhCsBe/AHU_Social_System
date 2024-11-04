#include "chatwindow.h"
#include "ui_chatwindow.h"

ChatWindow::ChatWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatWindow)
{
    ui->setupUi(this);
    setStyle();
    setConnect();
    setView();
    /*
    MessageItem item1={"E02114320","RhCsBe","E02114320.jpg","nihaodsjdahkjsadkfjkfjhkadsfdhajhfkjhakjhfadskjhfkdsjhfkjh",123,12};
    MessageItem item2={"","user2","","早安",QDateTime::currentMSecsSinceEpoch(),1};
    MessageItem item3={"","user3","","hello world \nskfdhkhkjhkiu12345678987654321",789,0};
    MessageItem item7={"","user4","","hello world ",1000,0};
    QStandardItem* item4=new QStandardItem();
    QStandardItem* item5=new QStandardItem();
    QStandardItem* item6=new QStandardItem();
    QStandardItem* item8=new QStandardItem();
    item4->setData(QVariant::fromValue(item1),Qt::UserRole+1);
    item5->setData(QVariant::fromValue(item2),Qt::UserRole+1);
    item6->setData(QVariant::fromValue(item3),Qt::UserRole+1);
    item8->setData(QVariant::fromValue(item7),Qt::UserRole+1);
    model->appendRow(item4);
    model->appendRow(item5);
    model->appendRow(item6);
    model->appendRow(item8);*/
}

ChatWindow::~ChatWindow()
{
    delete ui;
}

void ChatWindow::setStyle()
{
    //获取屏幕宽高
    QRect rect=QGuiApplication::primaryScreen()->geometry();
    screenWidth=rect.width();
    screenHeight=rect.height();

    //设置窗体无标题栏和提供最小化按钮
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowMinimizeButtonHint);
    setAttribute(Qt::WA_QuitOnClose, false);
    //setAttribute(Qt::WA_TranslucentBackground);

    //添加图片资源
    ui->close_btn->setIcon(QIcon("://photo/close2.png"));
    ui->min_btn->setIcon(QIcon("://photo/min2.png"));

    //设置发送按键的颜色
    ui->send->setRole(Material::Primary);
    ui->send->setOverlayStyle(Material::TintedOverlay);

    //设置滚动条样式
    QScrollBar* bar=ui->message_list->verticalScrollBar();
    bar->setStyleSheet(Protocol::scrollbarStyle);
}

void ChatWindow::setConnect()
{
    //最小化和关闭按键
    connect(ui->close_btn,&QToolButton::clicked,this,[&](){
        this->close();
        emit closeChatWindow(user->value("account").toString());
        this->deleteLater();
    });
    connect(ui->min_btn,&QToolButton::clicked,this,[&](){this->showMinimized();});
    connect(ui->bottom_close,&QPushButton::clicked,this,[&](){
        this->close();
        emit closeChatWindow(user->value("account").toString());
        this->deleteLater();
    });

    //发送信息
    connect(ui->send,&QtMaterialRaisedButton::clicked,this,[&](){
        if(Protocol::isConnecting)
        {
            if(ui->message->toPlainText().isEmpty())
                return;
            else
            {
                //qDebug()<<"发送消息："<<ui->message->toPlainText();
                emit sendMessage(InfoType::SendMessage,Protocol::getUserAccount(),user->value("account").toString(),ui->message->toPlainText(),"");
                ui->message->clear();
            }
        }
        else
            ElaMessageBar::warning(ElaMessageBarType::Bottom,"warning","连接已断开，请检检查网络设置",3000,this);
    });

    //打开个人界面
    connect(ui->user_name,&QPushButton::clicked,this,[&](){emit openPersonal(user->value("account").toString());});

}

void ChatWindow::initUserData(QJsonObject *user, QJsonArray *messageData)
{
    //给两个数据指针赋值，实际数据存储在主界面对象中
    this->user=user;
    this->messageData=messageData;

    //设置聊天对象用户名
    setUserName(user->value("userName").toString());

    //往模型中写入聊天记录
    for(auto i:*messageData)
    {
        QJsonObject json=i.toObject();
        QString account,userName,headPhoto,message;
        qint64 time=json.value("time").toString().toLongLong();
        account=json.value("account").toString();
        //userName=json.value("userName").toString();
        message=json.value("message").toString();
        if(account==Protocol::getUserAccount())
        {
            headPhoto=Protocol::getUserHeadShot();
            userName=Protocol::getUserName();
        }
        else
        {
            headPhoto=user->value("headPhoto").toString();
            userName=user->value("userName").toString();
        }
        MessageItem messageItem={account,userName,headPhoto,message,time,0};
        QStandardItem* item=new QStandardItem();
        item->setData(QVariant::fromValue(messageItem),Qt::UserRole+1);
        model->appendRow(item);
    }

    //设置任务栏图标
    QString headPhoto;
    if(user->value("headPhoto").toString().isEmpty())
        headPhoto=DefaultPixmap;
    else
        headPhoto=Protocol::getAllUserPath()+"/"+user->value("headPhoto").toString();
    setIcon(headPhoto);
}

void ChatWindow::initUserData(QJsonObject *group, QJsonArray *groupMemberData, QJsonArray *messageData)
{
    //将代理换成群聊代理
    ui->message_list->setItemDelegate(new ChatWindowDelegate(this,1));

    //给两个数据指针赋值，实际数据存储在主界面对象中
    this->user=group;
    this->messageData=messageData;
    //读取成员信息
    for(QJsonValue i:*groupMemberData)
    {
        QJsonObject json=i.toObject();
        QString account=json.value("account").toString();
        groupMember.insert(account,json);
    }

    //设置聊天对象用户名
    setUserName(user->value("userName").toString());

    //往模型中写入聊天记录
    for(auto i:*messageData)
    {
        QJsonObject json=i.toObject();
        QString account,userName,headPhoto,message;
        qint64 time=json.value("time").toString().toLongLong();
        account=json.value("account").toString();
        userName=groupMember.value(account).value("userName").toString();
        message=json.value("message").toString();
        headPhoto=groupMember.value(account).value("headPhoto").toString();
        MessageItem messageItem={account,userName,headPhoto,message,time,0};
        QStandardItem* item=new QStandardItem();
        item->setData(QVariant::fromValue(messageItem),Qt::UserRole+1);
        model->appendRow(item);
    }

    //设置任务栏图标
    QString headPhoto;
    if(user->value("headPhoto").toString().isEmpty())
        headPhoto=DefaultPixmap;
    else
        headPhoto=Protocol::getAllUserPath()+"/"+user->value("headPhoto").toString();
    setIcon(headPhoto);
}

void ChatWindow::addMessage(QString account,QJsonObject json)
{
    qDebug()<<account;
    QString userName,headPhoto,message;
    qint64 time=json.value("time").toString().toLongLong();
    //根据账号获取用户名和头像
    if(account==Protocol::getUserAccount())
    {
        qDebug()<<"这是我发的消息";
        userName=Protocol::getUserName();
        headPhoto=Protocol::getUserHeadShot();
    }
    else
    {
        //如果是群发的消息，要将发送者进行替换
        if(!groupMember.isEmpty())
        {
            qDebug()<<"这是群发的消息";
            account=json.value("messageType").toString();
            userName=groupMember.value(account).value("userName").toString();
            headPhoto=groupMember.value(account).value("headPhoto").toString();
        }
        else//不是群发
        {
            qDebug()<<"这不是群发的消息";
            userName=user->value("userName").toString();
            headPhoto=user->value("headPhoto").toString();
        }
    }
    message=json.value("message").toString();
    MessageItem messageItem={account,userName,headPhoto,message,time,0};
    QStandardItem* item=new QStandardItem();
    item->setData(QVariant::fromValue(messageItem),Qt::UserRole+1);
    model->appendRow(item);
}

void ChatWindow::openChatWIndow()
{
    ui->message_list->scrollToBottom();
}

void ChatWindow::mousePressEvent(QMouseEvent *event)
{
    //判断是否是左键按压
    if(event->button()==Qt::LeftButton)
    {
        //判断点击位置是否在顶部
        if(event->pos().y()<=ui->top->size().height())
        {
            //打开按压信号，并记录按压初始位置
            pressed=true;
            pressPoint=event->pos();
        }
    }
}

void ChatWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton);
    {
        //关闭按压信号
        pressed=false;
    }
}

void ChatWindow::mouseMoveEvent(QMouseEvent *event)
{
    //检查按压信号，然后移动对应距离
    if(pressed)
    {
        this->move(moveTo(event->globalPos()-pressPoint));
    }
}

QPoint ChatWindow::moveTo(QPoint point)
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

void ChatWindow::setView()
{
    model=new QStandardItemModel(this);
    ui->message_list->setModel(model);//设置模型
    ui->message_list->setItemDelegate(new ChatWindowDelegate(this));//设置代理
}

void ChatWindow::setUserName(QString str)
{
    //设置聊天窗口的用户名，并调节大小和居中显示
    ui->user_name->setText(str);
    QFontMetrics metrics(ui->user_name->font());
    ui->user_name->setFixedWidth(metrics.horizontalAdvance(ui->user_name->text()));
    //不知道为什么user_name的宽度有问题，所以采用this的宽度
    ui->user_name->move((this->width()-ui->user_name->width())/2,(ui->top->height()-ui->user_name->height())/2);
}

void ChatWindow::setIcon(QString str,int radius)
{
    //设置任务栏图标
    this->setWindowIcon(QIcon(Protocol::createHeadShot(str,radius)));
}
