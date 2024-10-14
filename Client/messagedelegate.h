#ifndef MESSAGEDELEGATE_H
#define MESSAGEDELEGATE_H

#include <QStyledItemDelegate>
#include <QObject>
#include "protocol.h"
#include <QDateTime>
#include <QFontMetrics>

class MessageDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit MessageDelegate(QObject *parent = nullptr);

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

private:
    int headRadius=28;
    int sumRadius=11;

    int headTop=10;
    int headLeft=15;

    int userNameLeft=10;
    int userNameTop=25;
    int userNameHeight=24;

    int messageTop=3;
    //int messageRight=20;
    int messageHeight=18;

    int timeLeft=60;
    int timeRight=10;
    int timeHeight=20;
    int sumTop=3;
};

#endif // MESSAGEDELEGATE_H
