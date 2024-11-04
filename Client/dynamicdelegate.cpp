#include "dynamicdelegate.h"

DynamicDelegate::DynamicDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{

}

void DynamicDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.isValid())
    {
        //设置背景颜色
        QColor backgroundColor=QColor(Qt::white);

        //保存画笔状态
        painter->save();
        painter->setRenderHints(QPainter::Antialiasing);//设置反锯齿
        painter->setRenderHints(QPainter::SmoothPixmapTransform);//设置平滑像素图转换


        // item 矩形区域
        QRectF rect;
        rect.setX(option.rect.x());
        rect.setY(option.rect.y());
        rect.setWidth(option.rect.width()-1);
        rect.setHeight(option.rect.height()-1);
        //qDebug()<<"选项高度："<<option.rect.height();

        //读取数据
        MessageItem message=index.data(Qt::UserRole+1).value<MessageItem>();

        //设置绘图区域
        QRectF timeRect,userNameRect,textRect;
        QRect headPhotoRect;
        //int textRows=getTextRows(option,index);

        headPhotoRect=QRect(rect.x()+leftSpace,rect.y()+topSpace,headPhotoSize,headPhotoSize);
        textRect=QRectF(headPhotoRect.x(),headPhotoRect.bottom()+centerSpace,rect.width()-leftSpace-rightSapce,textHeight);
        userNameRect=QRect(headPhotoRect.right()+headPhotoToNameSpace,headPhotoRect.y(),textRect.width()-headPhotoRect.width()-headPhotoToNameSpace,nameHeight);
        timeRect=QRect(userNameRect.left(),userNameRect.bottom(),userNameRect.width(),timeHeight);

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

        //绘制时间
        QDateTime time=QDateTime::fromMSecsSinceEpoch(message.time);
        //直接使用QDateTime::currentDateTimeUtc()获取的时间比东八区的慢几个小时
        //QDateTime::fromMSecsSinceEpoch的默认时区改成了本地
        QDateTime nowTime=QDateTime::fromMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch());
        //QDateTime nowTime=QDateTime::currentDateTimeUtc();
        QString str="";
        if(time.date().year()==nowTime.date().year()&&time.date().month()==nowTime.date().month()&&time.date().day()==nowTime.date().day())
        {
            str=time.time().toString("今天hh:mm");
        }
        else
        {
            str=time.toString("M月/d日 hh:mm");
        }
        painter->setPen(QPen("#808A87"));
        painter->setFont(QFont("Microsoft YaHei", timeFontSize));//设置字体
        painter->drawText(timeRect,Qt::AlignLeft|Qt::AlignVCenter,str);

        //绘制用户名
        painter->setPen(QPen(Qt::black));//设置颜色
        painter->setFont(QFont("Microsoft YaHei", nameFontSize));//设置字体
        painter->drawText(userNameRect,Qt::AlignLeft|Qt::AlignVCenter,message.userName);

        //绘制头像
        str=message.headPhoto;
        if(str.isEmpty())
            str=DefaultPixmap;
        else
            str=Protocol::getAllUserPath()+"/"+str;
        QPixmap headPhoto=Protocol::createHeadShot(str,headPhotoSize);
        painter->drawPixmap(headPhotoRect,headPhoto);

        //绘制文字
        painter->setPen(QPen(Qt::black));//设置颜色
        painter->setFont(QFont("Microsoft YaHei", textFontSize));//设置字体
        QFontMetrics metrics(QFont("Microsoft YaHei", textFontSize));//字体度量
        QStringList list=message.message.split("\n");
        for(auto i:list)
        {
            if(metrics.width(i)>textRect.width())
            {
                int pos=0;
                int length=getValidStringLength(i,pos,textRect.width());
                while(pos+length<i.length())
                {
                    painter->drawText(textRect,Qt::AlignLeft|Qt::AlignVCenter,i.mid(pos,length));
                    pos+=length;
                    length=getValidStringLength(i,pos,textRect.width());
                    //记得要更新绘图区域,修改y后还得修改高度
                    textRect.setY(textRect.y()+textHeight);
                    textRect.setHeight(textHeight);
                }
                painter->drawText(textRect,Qt::AlignLeft|Qt::AlignVCenter,i.mid(pos,length));
                textRect.setY(textRect.y()+textHeight);
                textRect.setHeight(textHeight);
            }
            else
            {
                painter->drawText(textRect,Qt::AlignLeft|Qt::AlignVCenter,i);
                textRect.setY(textRect.y()+textHeight);
                textRect.setHeight(textHeight);
            }
        }
        //恢复画笔状态
        painter->restore();
    }
}

QWidget* DynamicDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return nullptr;
}

QSize DynamicDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(option.rect.width(),getItemHeight(option,index));
}

int DynamicDelegate::getItemHeight(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //总高度=上下留白+中间留白+时间高度+文本高度
    int totalHeight=topSpace+bottomSapce+headPhotoSize+centerSpace;
    //判断如果与上一条信息的时间不超过3分钟且不是第一条信息，则不添加时间高度
    totalHeight+=getTextRows(option,index)*textHeight;
    return totalHeight;
}

int DynamicDelegate::getTextRows(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    MessageItem message=index.data(Qt::UserRole+1).value<MessageItem>();//根据索引读数据
    QStringList list=message.message.split("\n");
    int maxWidth=option.rect.width()-(leftSpace+rightSapce);//文本显示最大宽度(总宽度-两侧留白)
    int row=0;;
    //计算文本行数
    for(auto i:list)
    {
        int pos=0;
        while(pos+getValidStringLength(i,pos,maxWidth)<i.length())
        {
            row++;
            pos+=getValidStringLength(i,pos,maxWidth);
        }
        row++;
    }
    return row;
}

int DynamicDelegate::getValidStringLength(QString str, int pos, double width) const
{
    if(width<=0)
        width=500;
    QFont font("Microsoft YaHei",textFontSize);//字体
    QFontMetrics metrics(font);//字体度量
    QString temp="";
    int length=0;
    while(metrics.width(temp)<=width&&pos<str.length())
    {
        temp+=str[pos];
        pos++;
        length++;
    }
    if(metrics.width(temp)>width)
        length--;
    return length;
}
