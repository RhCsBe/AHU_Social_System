#ifndef PERSONAL_H
#define PERSONAL_H

#include <QWidget>
#include "protocol.h"
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include "ElaMessageBar.h"
#include <QMouseEvent>
#include <QEvent>
#include <QFileDialog>
#include "messagedelegate.h"
#include <QScreen>
#include <QApplication>
#include <QScrollBar>

enum PersonalType{
    FriendType=0,
    UserType,
    MyGroupType,
    JoinedGroupType,
    OtherGroupType,
    MyselfType
};

namespace Ui {
class Personal;
}

class Personal : public QWidget
{
    Q_OBJECT

public:
    explicit Personal(QWidget *parent = nullptr);
    ~Personal();

    //初始化设置
    void setStyle();//设置基本样式
    void initPersonal(QJsonObject* user,QJsonArray* memberData=nullptr,PersonalType type=FriendType);//初始化信息
    void setConnect();//绑定信号和槽函数
    void setFunction(PersonalType type=FriendType);//设置功能按键
    void initHeadPhoto();//初始化头像

    //修改用户数据
    void modifyUserData();//修改数据
    void resetUserData();//重置数据
    void setEditStatus();//设置编辑状态
    void setHeadPhoto();//设置头像

    //实现拖拽效果
    void mousePressEvent(QMouseEvent* event);//重写鼠标按压事件
    void mouseReleaseEvent(QMouseEvent* event);//重写鼠标释放事件
    void mouseMoveEvent(QMouseEvent* event);//重写鼠标移动事件
    QPoint moveTo(QPoint point);//越界判断并转换

    //对某些组件的事件进行拦截
    bool eventFilter(QObject *watched, QEvent *event);//重写事件过滤
    void setElaComboBoxFilter();//对几个ElaComboBox安装事件过滤器

    //视图和模型
    void setView();//设置视图
    void initModelData();//初始化模型数据

    //返回账号
    QString getAccount();

signals:
    void openChatWindow(QString account);//打开聊天界面
    void openPersonal(QString account);//打开个人界面
    void addFriend(QString account);//添加好友
    void joinGroup(QString account,QString group);//加入群聊
    void closePersonal(QString account);//关闭个人界面
    void modifyData(QString account,QString message,QString file);

private:
    Ui::Personal *ui;
    //界面拖拽
    bool pressed=false;//鼠标点击信号
    QPoint pressPoint;//鼠标点击初始位置
    int screenWidth=0;
    int screenHeight=0;

    //界面类型和标志位
    PersonalType type=MyselfType;//个人界面类型
    bool modify=false;//修改数据状态位，仅在meselfTyle类型下有效

    //数据指针
    QJsonObject* user=nullptr;//用户/群聊数据
    QJsonArray* memberData=nullptr;//群成员数据

    //数据模型
    QStandardItemModel* groupMemberModel=nullptr;
    QSortFilterProxyModel* groupMemberSortModel=nullptr;\

    //头像路径
    QString headPhotoPath="";
};

#endif // PERSONAL_H
