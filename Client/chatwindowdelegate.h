#ifndef CHATWINDOWDELEGATE_H
#define CHATWINDOWDELEGATE_H

#include <QStyledItemDelegate>
#include <QObject>
#include "protocol.h"
#include <algorithm>
#include <cmath>

class ChatWindowDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ChatWindowDelegate(QObject *parent = nullptr);

    //重写默认代理的绘图事件
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    //下面四个函数暂时没有重写的必要，没啥屌用，model的数据修改用自定义信号和槽来完成，不使用编辑器进行交互
    //返回编辑器  //这个还是有点用的，不重写的话，会调用父类的返回一个QLineEdit并显示
    QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    //设置编辑器数据
    //void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    //更新编辑器几何数据
    //void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    //设置模型数据
    //void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    int getBubbleWidth(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    int getItemHeight(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    int getTextRows(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    int getValidStringLength(QString str,int pos,double width) const;//获取当前有效字符串长度

private:
    int space=15;//侧边留白
    int iconWidth=50;//头像宽度
    int triWidth=15;//三角形宽度
    int bubbleSpace=10;//气泡留白
    int timeHeight=14;//时间高度
    int itemSpace=8;//消息留白（上下间隔一半）
    int textHeight=25;//单行文本高度(包含了文本的上下间隔）
    int fontSpace=7;//文字上下间隔（文字大小=textHeight-2*fontSpace)
};

#endif // CHATWINDOWDELEGATE_H
