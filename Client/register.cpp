#include "register.h"
#include "ui_register.h"

Register::Register(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Register)
{
    ui->setupUi(this);
    setStyle();
    setConnect();
}

Register::~Register()
{
    delete ui;
}

void Register::setStyle()
{
    //获取屏幕宽高
    QRect rect=QGuiApplication::primaryScreen()->geometry();
    screenWidth=rect.width();
    screenHeight=rect.height();

    //QtDesign设置的尺寸不一定是真是尺寸，所以要在这重新设置ElaComboBox的高度
    ui->identity_box->setMinimumHeight(41);
    ui->profession_box->setMinimumHeight(41);
    ui->college_box->setMinimumHeight(41);
    ui->sex_box->setMinimumHeight(41);
    //qDebug()<<"box高度:"<<ui->identity_box->height();
    //qDebug()<<"account:"<<ui->account->size();

    //设置登录按键的颜色
    ui->register_btn->setRole(Material::Primary);
    ui->register_btn->setBackgroundMode(Qt::OpaqueMode);
    ui->register_btn->setOverlayStyle(Material::TintedOverlay);

    //设置下部主体的背景色以及整体的无边框和背景透明以及窗口显示在最上层
    //ui->widget->setStyleSheet("#widget{border:none;background-color:white;}");
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    //setAttribute(Qt::WA_TranslucentBackground);

    //设置图片资源
    ui->close_btn->setIcon(QIcon("://photo/close.png"));
    ui->min_btn->setIcon(QIcon("://photo/min.png"));
    ui->logo->setPixmap(QPixmap("://photo/AHU1.png").scaled(ui->logo->size().width(),ui->logo->size().height(), Qt::KeepAspectRatio,Qt::SmoothTransformation));

    //设置主体背景
    QPixmap pixmap("://photo/background4.jfif");
    pixmap=pixmap.scaled(this->size(),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);//设置比例扩充加平滑转换
    QPalette palette = this->palette();
    palette.setBrush(QPalette::Background, pixmap);
    this->setPalette(palette);
    this->setAutoFillBackground(true); // 确保背景填充整个控件

    //设置tip
    ui->close_btn->setToolTip("关闭");
    ui->close_btn->setToolTipDuration(2000);
    ui->min_btn->setToolTip("最小化");
    ui->min_btn->setToolTipDuration(2000);
}

void Register::setConnect()
{
    connect(ui->min_btn,&QToolButton::clicked,this,[&](){this->showMinimized();});
    connect(ui->close_btn,&QToolButton::clicked,this,[&](){this->close();this->deleteLater();});
    connect(ui->identity_box,&ElaComboBox::textActivated,this,[&](QString str){
        if(str=="教师")
        {
            ui->profession->setVisible(false);
            ui->profession_box->setVisible(false);
        }
        else
        {
            ui->profession->setVisible(true);
            ui->profession_box->setVisible(true);
        }
    });
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
    connect(ui->register_btn,&QtMaterialFlatButton::clicked,this,[&](){
        if(judgeMessage())
        {
            //未与主机建立连接则提示并退出
            if(!Protocol::isConnecting)
            {
                ElaMessageBar::warning(ElaMessageBarType::Bottom,"warning","连接超时，请检查您的网络或者本地防火墙设置",3000,this);
                return;
            }
            QString account=ui->account_edit->text();

            //学号/工号写入account，身份写入messageType，其他数据全部写入message中，以换行符进行分隔，在tcp线程中对message进行分割读取
            //存储顺序：姓名-》密码-》性别-》学院
            //如果专业选项可见，则在末尾加上专业信息
            QString messageType=ui->identity_box->currentText();
            QString message;
            message+=ui->name_edit->text()+"\n";
            message+=ui->password_edit->text()+"\n";
            message+=ui->sex_box->currentText()+"\n";
            message+=ui->college_box->currentText();
            if(ui->profession_box->isVisible())
                message+="\n"+ui->profession_box->currentText();

            //发射注册信号将信息传给login，再由login传给tcp线程
            emit userRegister(account,message,messageType);
        }
    });
}

void Register::mousePressEvent(QMouseEvent *event)
{
    //判断是否是左键按压
    if(event->button()==Qt::LeftButton)
    {
        //打开按压信号，并记录按压初始位置
        pressed=true;
        pressPoint=event->pos();
    }
}

void Register::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton);
    {
        //关闭按压信号
        pressed=false;
    }
}

void Register::mouseMoveEvent(QMouseEvent *event)
{
    //检查按压信号，然后移动对应距离
    if(pressed)
    {
        this->move(moveTo(event->globalPos()-pressPoint));
    }
}

QPoint Register::moveTo(QPoint point)
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

bool Register::matchRegExp()
{
    //账号为E或T开头，后面有8位数字
    QRegularExpression accountExp("[E|T]\\d{8}");
    //密码为8-16位数字字母组合
    QRegularExpression passwordExp("[A-Z|a-z|0-9]{8,16}");
    //ElaMessageBar::information(ElaMessageBarType::PositionPolicy::BottomRight,"通知","开始进行正则匹配",5000,this);
    return accountExp.match(ui->account_edit->text()).hasMatch()&&passwordExp.match(ui->password_edit->text()).hasMatch();
}

bool Register::judgeMessage()
{
    //第一层判断数据是否有空项
    if(ui->account_edit->text().isEmpty())
    {
        ElaMessageBar::warning(ElaMessageBarType::BottomRight,"warning","学号/工号不能为空",2000,this);
        return false;
    }
    else if(ui->name_edit->text().isEmpty())
    {
        ElaMessageBar::warning(ElaMessageBarType::BottomRight,"warning","姓名不能为空",2000,this);
        return false;
    }
    else if(ui->password_edit->text().isEmpty())
    {
        ElaMessageBar::warning(ElaMessageBarType::BottomRight,"warning","密码不能为空",2000,this);
        return false;
    }
    else if(ui->confirm_edit->text().isEmpty())
    {
        ElaMessageBar::warning(ElaMessageBarType::BottomRight,"warning","请确认密码",2000,this);
        return false;
    }
    else
    {
        //第二层判断账号密码是否符合规格
        if(!matchRegExp())
        {
            ElaMessageBar::warning(ElaMessageBarType::BottomRight,"warning","学号/工号或密码不符合规格",2000,this);
            return false;
        }
        else
        {
            //第三层判断密码前后是否一致
            if(ui->password_edit->text()!=ui->confirm_edit->text())
            {
                ElaMessageBar::warning(ElaMessageBarType::BottomRight,"warning","请确认密码一致",2000,this);
                return false;
            }
            else
            {
                return true;
            }
        }
    }
}

void Register::regiterResult(int result)
{
    switch(result)
    {
        case -1:
        {
            ElaMessageBar::error(ElaMessageBarType::BottomRight,"error","信息不匹配，注册失败",3000,this);
            break;
        }
        case 0:
        {
            ElaMessageBar::information(ElaMessageBarType::BottomRight,"information","该账号已存在，请勿重复注册",3000,this);
            break;
        }
        case 1:
        {
            ElaMessageBar::success(ElaMessageBarType::BottomRight,"success","注册成功，请返回登录界面登录",3000,this);
            break;
        }
        default:
            break;
    }
}
