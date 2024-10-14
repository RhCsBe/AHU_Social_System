#include "chatwindow.h"
#include "ui_chatwindow.h"

ChatWindow::ChatWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatWindow)
{
    ui->setupUi(this);
    setStyle();
    setConnect();
    model=new QStandardItemModel(this);

    ui->message_list->setModel(model);
    ui->message_list->setItemDelegate(new ChatWindowDelegate(this));
    MessageItem item1={"E02114320.jpg","RhCsBe","E02114320","nihaodsjdahkjsadkfjkfjhkadsfdhajhfkjhakjhfadskjhfkdsjhfkjh",123,12};
    MessageItem item2={"","user2","","早安",QDateTime::currentMSecsSinceEpoch(),1};
    MessageItem item3={"","user3","","hello world \nskfdhkhkjhkiu12345678987654321",789,0};
    QStandardItem* item4=new QStandardItem();
    QStandardItem* item5=new QStandardItem();
    QStandardItem* item6=new QStandardItem();
    item4->setData(QVariant::fromValue(item1),Qt::UserRole+1);
    item5->setData(QVariant::fromValue(item2),Qt::UserRole+1);
    item6->setData(QVariant::fromValue(item3),Qt::UserRole+1);
    model->appendRow(item4);
    model->appendRow(item5);
    model->appendRow(item6);
}

ChatWindow::~ChatWindow()
{
    delete ui;
}

void ChatWindow::setStyle()
{
    //设置窗体无标题栏和提供最小化按钮
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowMinimizeButtonHint);
    //setAttribute(Qt::WA_TranslucentBackground);

    //设置聊天窗口的用户名，并调节大小和居中显示
    ui->user_name->setText("hjdshfaksnc,haehkjewhfsdn,mscxc");
    QFontMetrics metrics(ui->user_name->font());
    ui->user_name->setFixedWidth(metrics.horizontalAdvance(ui->user_name->text()));
    //不知道为什么user_name的宽度有问题，所以采用this的宽度
    ui->user_name->move((this->width()-ui->user_name->width())/2,(ui->top->height()-ui->user_name->height())/2);

    //添加图片资源
    ui->close_btn->setIcon(QIcon("://photo/close2.png"));
    ui->min_btn->setIcon(QIcon("://photo/min2.png"));

    //设置发送按键的颜色
    ui->send->setRole(Material::Primary);
}

void ChatWindow::setConnect()
{
    connect(ui->close_btn,&QToolButton::clicked,this,[&](){this->close();});
    connect(ui->min_btn,&QToolButton::clicked,this,[&](){this->showMinimized();});
    connect(ui->bottom_close,&QPushButton::clicked,this,[&](){this->close();});
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
        move(event->globalPos()- pressPoint);
    }
}
