#include "chatwindowdelegate.h"

ChatWindowDelegate::ChatWindowDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{

}

void ChatWindowDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.isValid())
    {
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
        QRectF timeRect=QRect(rect.x(),rect.y(),rect.width(),timeHeight);
        QRectF bubbleRect,textRect;
        QRect headPhotoRect;
        QPolygon triangle;
        int bubbleWidth=getBubbleWidth(option,index);
        int textRows=getTextRows(option,index);

        //绘制时间
        QDateTime time=QDateTime::fromMSecsSinceEpoch(message.time);
        //直接使用QDateTime::currentDateTimeUtc()获取的时间比东八区的慢几个小时
        //QDateTime::fromMSecsSinceEpoch的默认时区改成了本地
        QDateTime nowTime=QDateTime::fromMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch());
        //QDateTime nowTime=QDateTime::currentDateTimeUtc();
        QString str="";
        if(time.date().year()==nowTime.date().year()&&time.date().month()==nowTime.date().month()&&time.date().day()==nowTime.date().day())
        {
            str=time.time().toString("h:mm:ss");
        }
        else
        {
            str=time.toString("yyyy/M/d h:mm:ss");
        }
        painter->setPen(QPen("#808A87"));
        painter->setFont(QFont("Microsoft YaHei", 10));//设置字体
        //如果上一条信息的时间差不超过3分钟且不是第一条信息，则不绘制时间
        QModelIndex lastIndex=index.model()->index(index.row()-1,0);
        if(lastIndex.isValid())
        {
            int time1=index.data(Qt::UserRole+1).value<MessageItem>().time;
            int time2=lastIndex.data(Qt::UserRole+1).value<MessageItem>().time;
            if(std::abs(time1-time2)>=3*60*1000)
                painter->drawText(timeRect,Qt::AlignCenter|Qt::AlignVCenter,str);
            else
                //不绘制时间记得将timeRect的高度设置为零，否则会影响别的参数
                timeRect.setHeight(0);
        }
        else
        {
            painter->drawText(timeRect,Qt::AlignCenter|Qt::AlignVCenter,str);
        }


        //根据账号判断绘图方向
        if(message.account==Protocol::getUserAccount())
        {
            headPhotoRect=QRect(rect.right()-space-iconWidth,timeRect.bottom()+itemSpace,iconWidth,iconWidth);
            bubbleRect=QRectF(headPhotoRect.left()-triWidth-bubbleWidth,headPhotoRect.top(),bubbleWidth,bubbleSpace*2+textRows*textHeight);
            QPoint point(bubbleRect.right(),bubbleRect.top()+bubbleSpace);
            triangle<<point<<QPoint(point.x(),point.y()+7)<<QPoint(point.x()+7,point.y()-3);
            painter->setBrush(QColor(18,183,245));
        }
        else
        {
            headPhotoRect=QRect(rect.left()+space,timeRect.bottom()+itemSpace,iconWidth,iconWidth);
            bubbleRect=QRectF(headPhotoRect.right()+triWidth,headPhotoRect.top(),bubbleWidth,bubbleSpace*2+textRows*textHeight);
            QPoint point(bubbleRect.left(),bubbleRect.top()+bubbleSpace);
            triangle<<point<<QPoint(point.x(),point.y()+7)<<QPoint(point.x()-7,point.y()-3);
            painter->setBrush(QColor(229,229,229));
        }
        textRect=QRect(bubbleRect.left()+bubbleSpace,bubbleRect.top()+bubbleSpace,bubbleRect.width()-2*bubbleSpace,textHeight);

        //绘制头像
        str=message.headPhoto;
        if(str.isEmpty())
            str=DefaultPixmap;
        else
            str=Protocol::getAllUserPath()+"/"+str;
        QPixmap headPhoto=Protocol::createHeadShot(str,iconWidth);
        painter->drawPixmap(headPhotoRect,headPhoto);

        //绘制气泡和三角型
        painter->drawRoundRect(bubbleRect,5,5);
        painter->drawPolygon(triangle);

        //绘制文字
        if(message.account==Protocol::getUserAccount())//根据账号设置画笔颜色
            painter->setPen(QPen(Qt::white));//对方黑色
        else
            painter->setPen(QPen(Qt::black));//我方白色
        painter->setFont(QFont("Microsoft YaHei", textHeight-2*fontSpace));//设置字体
        QFontMetrics metrics(QFont("Microsoft YaHei", textHeight-2*fontSpace));//字体度量
        QStringList list=message.message.split("\n");
        //int fontNum=textRect.width()/metrics.width("黎");//一行最大字数
        //qDebug()<<"list:"<<list.size();
        for(auto i:list)
        {
            //qDebug()<<"str:"<<i;
            //qDebug()<<textRect;
            //qDebug()<<i;
            //如果单行字数超出限制，要分行绘制
//            if(i.length()>fontNum)
//            {
//                qDebug()<<"1";
//                for(int temp=0;temp<i.length()/fontNum;temp++)
//                {
//                    painter->drawText(textRect,Qt::AlignLeft|Qt::AlignVCenter,i.mid(temp*fontNum,fontNum));
//                    //记得要更新绘图区域
//                    textRect.setY(textRect.y()+textHeight);
//                }
//                int num=i.length()%fontNum;
//                //存在残留数据要另起一行
//                if(num!=0)
//                {
//                    qDebug()<<"3";
//                    painter->drawText(textRect,Qt::AlignLeft|Qt::AlignVCenter,i.right(num));
//                    textRect.setY(textRect.y()+textHeight);
//                }
//            }
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
                //qDebug()<<"2";
                painter->drawText(textRect,Qt::AlignLeft|Qt::AlignVCenter,i);
                textRect.setY(textRect.y()+textHeight);
                textRect.setHeight(textHeight);
            }
        }
        //恢复画笔状态
        painter->restore();
    }
}

QWidget *ChatWindowDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return nullptr;
}

QSize ChatWindowDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //qDebug()<<"宽度："<<option.rect.width()<<"\t内容："<<index.data(Qt::UserRole+1).value<MessageItem>().message;
    //qDebug()<<"高度："<<option.rect.height()<<"设置的高度："<<getItemHeight(option,index);
    return QSize(option.rect.width(),getItemHeight(option,index));
    //return QSize(option.rect.width(),getItemHeight(option,index));
}

int ChatWindowDelegate::getBubbleWidth(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //气泡最大宽度为item-两倍的侧边留白加头像宽度加三角形宽度
    int maxWidth=option.rect.width()-(space+triWidth+iconWidth)*2;
    MessageItem message=index.data(Qt::UserRole+1).value<MessageItem>();//根据索引读数据
    QFont font("Microsoft YaHei",textHeight-2*fontSpace);//字体
    QFontMetrics metrics(font);//字体度量
    QStringList list=message.message.split("\n");//支持换行符输入
    int maxTextWidth=0;//最大文本宽度
    //遍历每行文本
    for(auto i:list)
    {
        if(metrics.width(i)+2*bubbleSpace*2>maxTextWidth)
            maxTextWidth=metrics.width(i)+2*bubbleSpace;
    }
    //返回最大值和最大文本宽度中最小的一个
    return std::min(maxWidth,maxTextWidth);
}

int ChatWindowDelegate::getItemHeight(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //总高度=item上下留白+时间高度+气泡上下留白+文本高度
    int totalHeight=2*(itemSpace+bubbleSpace);
    //判断如果与上一条信息的时间不超过3分钟且不是第一条信息，则不添加时间高度
    QModelIndex lastIndex=index.model()->index(index.row()-1,0);
    if(lastIndex.isValid())
    {
        int time1=index.data(Qt::UserRole+1).value<MessageItem>().time;
        int time2=lastIndex.data(Qt::UserRole+1).value<MessageItem>().time;
        if(std::abs(time1-time2)>=3*60*1000)
            totalHeight+=timeHeight;
    }
    else
    {
        totalHeight+=timeHeight;
    }
    totalHeight+=getTextRows(option,index)*textHeight;
    return totalHeight;
}

int ChatWindowDelegate::getTextRows(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    MessageItem message=index.data(Qt::UserRole+1).value<MessageItem>();//根据索引读数据
    QStringList list=message.message.split("\n");
    int maxWidth=option.rect.width()-(space+triWidth+iconWidth+bubbleSpace)*2;//文本显示最大宽度(气泡最大宽度-气泡两侧留白)
    int row=0;
    //int fontNum=maxWidth/metrics.width("黎");
    //计算文本行数
    for(auto i:list)
    {
//        int num=i.length()/fontNum;
//        if(i.length()%fontNum!=0)
//            num++;
//        row+=num;
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

int ChatWindowDelegate::getValidStringLength(QString str, int pos, double width) const
{
    //你永远不知道你会遇到什么脑残bug，我以为之前那个地址迁移就够离谱了，这个更是重量级
    //代理绑定后，view不知道发生了什么，会有一组稳定的option.rect().width()的值是0，就导致传进来的width是一个负数，
    //然后在计算item高度的函数中陷入死循环，且之后option.rect().width()还会出现稳定的波动，原因不明
    //所以在这要设置一个判断，过滤掉这组垃圾数据，但是为什么这个垃圾数据这么稳定，要不是老子没空，我一定要搞清楚它的调用机制和产生原因

    //补充：经过追踪代理的调用的顺序，view在更换代理后首先会调用代理的sizeHint对所有的数据项进行设置，
    //然后对第一项再次进行设置，此时将第一项的宽高设置为零，就是在这出现了这个垃圾数据，
    //接着开始正式将每一项都设置大小并调用绘图函数进行绘，依旧不清楚前两步有什么屌用
    //而且在每次在代理重绘之前，视图会将option.rect().height()的返回值设置为一个内部维护的数据，反正不是你设置的那个
    //必须要经过你在sizeHint中的修改，才能在paint中的option.rect().height()拿到你设置的数据，什么牛马机制
    //简单来说就是，你建议归你建议，你说一次我改一次，到下次你不说了，那我就用我内部维护的这个值
    if(width<=0)
        width=500;
    QFont font("Microsoft YaHei",textHeight-2*fontSpace);//字体
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

