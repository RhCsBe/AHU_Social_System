#ifndef PERSONAL_H
#define PERSONAL_H

#include <QWidget>
#include "protocol.h"
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include "ElaMessageBar.h"

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

    //修改用户数据
    void modifyUserData();//修改数据
    void resetUserData();//重置数据
    void setEditStatus();//设置编辑状态

signals:
    void openChatWindow(QString account);//打开聊天界面
    void addFriend(QString account);//添加好友
    void joinGroup(QString account);//加入群聊
    void updateUserData(QJsonObject json);//修改用户信息

private:
    Ui::Personal *ui;
    PersonalType type=FriendType;//个人界面类型
    bool modify=false;//修改数据状态位，仅在meselfTyle类型下有效

    //数据指针
    QJsonObject* user=nullptr;//用户/群聊数据
    QJsonArray* memberData=nullptr;//群成员数据

    //数据模型
    QStandardItemModel* groupMemberModel=nullptr;
    QSortFilterProxyModel* groupMemberSortModel=nullptr;
};

#endif // PERSONAL_H
