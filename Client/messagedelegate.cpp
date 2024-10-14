#include "messagedelegate.h"

MessageDelegate::MessageDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{

}

void MessageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //真他妈麻烦，还得一个一个的画，下次就是内存爆炸老子也不画图了，直接widget
    if (index.isValid()) {
        //设置背景颜色
        QColor backgroundColor=QColor(Qt::white);

        //保存画笔状态
        painter->save();
        painter->setRenderHints(QPainter::Antialiasing);//设置反锯齿
        painter->setRenderHints(QPainter::SmoothPixmapTransform);//设置平滑像素图转换
        painter->setFont(QFont("Microsoft YaHei", 10));//设置字体

        //提取模型数据
        QVariant var = index.data(Qt::UserRole+1);
        MessageItem itemData = var.value<MessageItem>();

        QString headPath=itemData.headPhoto;
        if(headPath.isEmpty())
            headPath=DefalutPixmap;
        else
            headPath=Protocol::getAllUserPath()+"/"+itemData.headPhoto;
        //始终静态方法获得圆角头像
        QPixmap headPhoto=Protocol::createHeadShot(headPath,headRadius);

        // item 矩形区域
        QRectF rect;
        rect.setX(option.rect.x());
        rect.setY(option.rect.y());
        rect.setWidth(option.rect.width()-1);
        rect.setHeight(option.rect.height()-1);

        //设置绘图路线
        QPainterPath path;
        path.moveTo(rect.topRight());
        path.lineTo(rect.topLeft());
        path.quadTo(rect.topLeft(), rect.topLeft());
        path.lineTo(rect.bottomLeft());
        path.quadTo(rect.bottomLeft(), rect.bottomLeft());
        path.lineTo(rect.bottomRight());
        path.quadTo(rect.bottomRight(), rect.bottomRight());
        path.lineTo(rect.topRight());
        path.quadTo(rect.topRight(), rect.topRight());


        // 鼠标悬停或者选中时改变背景色
        if (option.state.testFlag(QStyle::State_MouseOver)) {
            painter->setPen(QPen(QColor("#ebeced")));
            backgroundColor=QColor("#ebeced");
            painter->setBrush(backgroundColor);
            painter->drawPath(path);
        }
        if (option.state.testFlag(QStyle::State_Selected)) {
            painter->setPen(QPen(QColor("#e3e3e5")));
            backgroundColor=QColor("#e3e3e5");
            painter->setBrush(backgroundColor);
            painter->drawPath(path);
        }

        //设置绘图区域
        //注意，别他妈设置错了，到时候渲染出问题，整个程序都要崩溃
        QRect headPhotoRect=QRect(headLeft,rect.top()+(rect.height()-headRadius*2)/2,headRadius*2, headRadius*2);
        QRectF userNameRect=QRect(headPhotoRect.right()+userNameLeft,headPhotoRect.top(),rect.width()-headPhotoRect.right()-timeLeft-userNameLeft,userNameHeight);
        QRectF messageRect=QRect(userNameRect.left(),userNameRect.bottom()+messageTop,rect.width()-userNameRect.left()-timeRight,messageHeight);
        QRectF timeRect=QRect(rect.width()-timeLeft,userNameRect.top(),timeLeft-timeRight,timeHeight);
        QRectF sumBackgroundRect=QRect(timeRect.left(),timeRect.bottom()+sumTop,timeRect.width(),timeRect.height());
        QRectF sumRect=QRect(timeRect.right()-sumRadius*2,sumBackgroundRect.top(),sumRadius*2,sumRadius*2);

        // 绘制头像，用户名，最近消息
        painter->drawPixmap(headPhotoRect, headPhoto);
        painter->setPen(QPen(Qt::black));
        painter->setFont(QFont("Microsoft Yahei", 13));
        painter->drawText(userNameRect,Qt::AlignLeft,itemData.userName);

        //bug修复，当信息过长，超出绘制区域时，如果存在空格，会截断空格以后的内容，也就是一个单词（万恶的洋人）
        //不用这么麻烦了，在绘制函数中添加一个对其方式就解决了，但是找不到默认参数，那我之前没设置的时候调用的是什么对其方式？
//        QFont font("Arial",10);
//        QString str=itemData.message;
//        QFontMetrics metrics(font);
//        int messageWidth=metrics.width(itemData.message);
//        if(messageWidth>messageRect.width())
//        {
//            qDebug()<<"yes"<<str;
//            str="";
//            for(int i=0;i<itemData.message.length()&&metrics.width(str)<=messageRect.width();i++)
//            {
//                str+=itemData.message[i];
//            }
//            str.chop(2);
//        }
//        qDebug()<<str;
        painter->setPen(QPen("#808A87"));
        painter->setFont(QFont("Arial",10));
        painter->drawText(messageRect,Qt::AlignLeft,itemData.message);

        //绘制时间
        QDateTime time=QDateTime::fromMSecsSinceEpoch(itemData.time);
        QDateTime nowTime=QDateTime::currentDateTimeUtc();
        QString str="";
        if(time.date().year()==nowTime.date().year()&&time.date().month()==nowTime.date().month()&&time.date().day()==nowTime.date().day())
        {
            str=time.time().toString("hh:mm");
        }
        else
        {
            str=time.date().toString("M-d");
        }
        painter->drawText(timeRect,Qt::AlignRight,str);

        //绘制消息总数
        if(itemData.sum>0)
        {
            //绘制消息数背景，若消息数大于零，则绘制，会掩盖该区域的消息
            painter->setPen(QPen(backgroundColor));
            painter->setBrush(backgroundColor);
            painter->drawRect(sumBackgroundRect);
            //painter->setFont(QFont("Arial",10));
            painter->setBrush(QBrush(Qt::red));
            painter->drawEllipse(sumRect.center(),sumRadius,sumRadius);
            painter->drawText(sumRect.x(),sumRect.y(),sumRect.width(),sumRect.height(),Qt::AlignCenter,QString::number(itemData.sum));
        }

        //画笔复原
        painter->restore();
    }
}

QWidget *MessageDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return  nullptr;
}

QSize MessageDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //qDebug()<<"width:"<<option.rect.width();
    return QSize(option.rect.width(),75);
}
