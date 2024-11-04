#include "editdynamicwindow.h"
#include "ui_editdynamicwindow.h"

EditDynamicWindow::EditDynamicWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EditDynamicWindow)
{
    ui->setupUi(this);
    setStyle();
    setConnect();
}

EditDynamicWindow::~EditDynamicWindow()
{
    delete ui;
}

void EditDynamicWindow::setStyle()
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
}

void EditDynamicWindow::setConnect()
{
    //为最小化按钮和关闭按钮绑定处理函数
    connect(ui->min_btn,&QToolButton::clicked,this,[&](){this->showMinimized();});
    connect(ui->close_btn,&QToolButton::clicked,this,[&](){this->close();this->deleteLater();});
    //发布动态
    connect(ui->send_btn,&QtMaterialFlatButton::clicked,this,&EditDynamicWindow::publish);
}

void EditDynamicWindow::mousePressEvent(QMouseEvent *event)
{
    //判断是否是左键按压
    if(event->button()==Qt::LeftButton)
    {
        //打开按压信号，并记录按压初始位置
        pressed=true;
        pressPoint=event->pos();
    }
}

void EditDynamicWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton);
    {
        //关闭按压信号
        pressed=false;
    }
}

void EditDynamicWindow::mouseMoveEvent(QMouseEvent *event)
{
    //检查按压信号，然后移动对应距离
    if(pressed)
    {
        this->move(moveTo(event->globalPos()-pressPoint));
    }
}

QPoint EditDynamicWindow::moveTo(QPoint point)
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

void EditDynamicWindow::publish()
{
    if(!Protocol::isConnecting)
    {
        ElaMessageBar::error(ElaMessageBarType::BottomRight,"Error","登录超时，请检查您的网络或者本地防火墙设置",3000,ui->frame);
        return;
    }
    else
    {
        if(ui->edit->toPlainText().isEmpty())
        {
            ElaMessageBar::warning(ElaMessageBarType::BottomRight,"warning","内容不能为空",3000,ui->frame);
            return;
        }
        else if(ui->edit->toPlainText().length()>300)
        {
            ElaMessageBar::warning(ElaMessageBarType::BottomRight,"warning","字数不能大于300",3000,ui->frame);
            return;
        }
        else
        {
            emit sendDynamic(ui->edit->toPlainText());
            ElaMessageBar::success(ElaMessageBarType::BottomRight,"success","发布成功",3000,ui->frame);
            return;
        }
    }
}
