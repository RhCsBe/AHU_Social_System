#ifndef DYNAMICDELEGATE_H
#define DYNAMICDELEGATE_H

#include <QStyledItemDelegate>
#include <QObject>
#include "protocol.h"

class DynamicDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DynamicDelegate(QObject *parent = nullptr);

    //重写默认代理的绘图事件
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    //下面四个函数暂时没有重写的必要，没啥屌用，model的数据修改用自定义信号和槽来完成，不使用编辑器进行交互
    //返回编辑器  //这个还是有点用的，不重写的话，会调用父类的返回一个QLineEdit并显示
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    //设置编辑器数据
    //void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    //更新编辑器几何数据
    //void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    //设置模型数据
    //void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    int getItemHeight(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    int getTextRows(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    int getValidStringLength(QString str,int pos,double width) const;//获取当前有效字符串长度

private:
    int topSpace=15;//顶部间隔
    int bottomSapce=15;//底部间隔
    int leftSpace=15;//左边间隔
    int rightSapce=20;//右边间隔
    int textFontSize=12;//内容字体大小
    int nameFontSize=10;//姓名字体大小
    int timeFontSize=9;//时间字体大小
    int headPhotoSize=50;//头像大小
    int timeHeight=20;//时间高度
    int textHeight=25;//单行文本高度
    int nameHeight=25;//名字高度
    int centerSpace=8;//头像和内容间隔
    int headPhotoToNameSpace=10;//头像和用户名间隔
};

#endif // DYNAMICDELEGATE_H
