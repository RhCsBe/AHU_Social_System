#include "sqldata.h"

SqlData::SqlData()
{
    connectDataBase();
}

void SqlData::connectDataBase()
{
    db=QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setDatabaseName("ahu_social_system");
    db.setUserName("root");
    db.setPassword("LJH030825");
    if(db.open())
    {
        //QMessageBox::information(nullptr,"提示","连接数据库成功");
    }
    else
    {
        //QMessageBox::warning(nullptr,"警告","连接数据库失败");
    }
}

bool SqlData::judgeUserExist(QString account, QString password)
{
    QString table;
    if(account[0]=='T')
    {
        table="teacher_user_data";
    }
    else
    {
        table="student_user_data";
    }
    QString str=QString::asprintf("select * from %1 where account='%2' and pwd= '%3';").arg(table).arg(account).arg(password);
    qDebug()<<str;
    QSqlQuery sql(db);
    if(sql.exec(str))
    {
        if(sql.next())
            return true;
        else
            return false;
    }
    else
    {
        qDebug()<<"数据库查找失败";
        return false;
    }
}

bool SqlData::judgeUpdateTimeOfRelation(QString account,qint64 lastLoginTime)
{
    //判断联系人是否已同步到最新，是返回true，不是返回false
    QString str=QString::asprintf("select update_time from relation where account='%1';").arg(account);
    QSqlQuery sql(db);
    if(sql.exec(str))
    {
        sql.next();
        qDebug()<<"update_time_1:"<<sql.value("update_time").toLongLong()<<"\tuser_update_time_1"<<lastLoginTime;
        if(sql.value("update_time").toLongLong()>lastLoginTime)
            return false;
        else
            return true;
    }
    else
    {
        qDebug()<<"数据库查找失败";
        return true;
    }
}

bool SqlData::judgeUpdateTimeOfUser(QString account, qint64 lastLoginTime)
{
    QString table;
    if(account[0]=='T')
    {
        table="teacher_user_data";
    }
    else
    {
        table="student_user_data";
    }
    QString str=QString::asprintf("select update_time from %1 where account='%2';").arg(table).arg(account);
    QSqlQuery sql(db);
    if(sql.exec(str))
    {
        sql.next();
        qDebug()<<"update_time_2:"<<sql.value("update_time").toLongLong()<<"\tuser_update_time_2"<<lastLoginTime;
        if(sql.value("update_time").toLongLong()>lastLoginTime)
            return false;
        else
            return true;
    }
    else
    {
        qDebug()<<"数据库查找失败";
        return true;
    }
}

bool SqlData::judgeLastLoginTimeOfUser(QString account, qint64 lastLoginTime)
{
    QString table;
    if(account[0]=='T')
    {
        table="teacher_user_data";
    }
    else
    {
        table="student_user_data";
    }
    QString str=QString::asprintf("select login_time from %1 where account = '%2';").arg(table).arg(account);
    QSqlQuery sql(db);
    if(sql.exec(str))
    {
        sql.next();
        if(sql.value("login_time").toLongLong()>lastLoginTime)
        {
            return false;
        }
        else
            return true;
    }
    else
        return true;
}

QJsonObject SqlData::getUserDateFile(QString account,bool remember,bool autoLogin)
{
    QString table;
    if(account[0]=='T')
    {
        table="teacher_user_data";
    }
    else
    {
        table="student_user_data";
    }
    QString str=QString::asprintf("select * from %1 where account = '%2';").arg(table).arg(account);
    QSqlQuery sql(db);
    QJsonObject json;
    if(sql.exec(str))
    {
        sql.next();
        json.insert("account",sql.value("account").toString());//账号
        json.insert("password",sql.value("pwd").toString());//密码
        json.insert("userName",sql.value("username").toString());//用户名
        json.insert("signature",sql.value("signature").toString());//个性签名
        json.insert("sex",sql.value("sex").toString());//性别
        json.insert("age",sql.value("age").toInt());//年龄
        json.insert("birthday",sql.value("birthday").toString());//生日
        json.insert("location",sql.value("location").toString());//地址
        json.insert("bloodType",sql.value("blood_type").toString());//血型
        json.insert("college",sql.value("college").toString());//学院
        if(table=="student_user_data")
            json.insert("profession",sql.value("profession").toString());//专业
        //json不提供长整形的存储和转换，所以要将长整形转成字符串存储在json中
        json.insert("lastLoginTime",QString::number(sql.value("login_time").toLongLong()));//上次登录时间(其实就是这次的）
        json.insert("headPhoto",sql.value("head_photo").toString());//头像
        json.insert("remember",remember);//是否记住密码
        json.insert("autoLogin",autoLogin);//是否自动登录
    }
    return json;
}

QJsonArray SqlData::getFriendDataFile(QString account)
{
    //根据账号检索好友信息
    QString str=QString::asprintf("select friends from relation where account = '%1';").arg(account);
    QSqlQuery sql(db);
    QJsonArray jsonArray;
    if(sql.exec(str))
    {
        sql.next();
        QString friends=sql.value("friends").toString();

        //friends为空直接返回jsonArray，你妈的为什么空串还能分割，什么牛马写的函数
        if(friends.isEmpty())
        {
            return jsonArray;
        }
        QStringList list=friends.split(";");
        qDebug()<<"friends:"<<friends;
        qDebug()<<"list_num:"<<list.size();
        //遍历好友
        for(auto i:list)
        {
            QString table;
            if(i[0]=='T')
            {
                table="teacher_user_data";
            }
            else
            {
                table="student_user_data";
            }
            QString str=QString::asprintf("select * from %1 where account = '%2';").arg(table).arg(i);
            QJsonObject json;
            if(sql.exec(str))
            {
                sql.next();
                json.insert("account",sql.value("account").toString());
                json.insert("userName",sql.value("username").toString());
                json.insert("signature",sql.value("signature").toString());
                json.insert("sex",sql.value("sex").toString());
                json.insert("age",sql.value("age").toInt());
                json.insert("birthday",sql.value("birthday").toString());
                json.insert("location",sql.value("location").toString());
                json.insert("bloodType",sql.value("blood_type").toString());
                json.insert("college",sql.value("college").toString());
                if(table=="student_user_data")
                    json.insert("profession",sql.value("profession").toString());
                json.insert("headPhoto",sql.value("head_photo").toString());
            }
            jsonArray.append(json);
        }
    }
    return jsonArray;
}

QByteArray SqlData::getGroupDataFile(QString account,QString& headPhoto)
{
    //根据账号检索群聊信息
    QString str=QString::asprintf("select all_groups from relation where account = '%1';").arg(account);
    QSqlQuery sql(db);
    QJsonArray jsonArray;
    if(sql.exec(str))
    {
        sql.next();
        QString allGroups=sql.value("all_groups").toString();

        //friends为空直接返回jsonArray，你妈的为什么空串还能分割，什么牛马写的函数
        if(allGroups.isEmpty())
        {
            return QJsonDocument(jsonArray).toJson();
        }
        QStringList list=allGroups.split(";");
        qDebug()<<"friends:"<<allGroups;
        qDebug()<<"list_num:"<<list.size();
        //遍历好友
        for(auto i:list)
        {
            jsonArray.append(askForGroupData(i,headPhoto));
        }
    }
    return QJsonDocument(jsonArray).toJson();
}

QStringList SqlData::getAllFriendHeadPhoto(QString account)
{
    QStringList list;
    //获取好友列表
    QString str=QString::asprintf("select friends from relation where account = '%1';").arg(account);
    QSqlQuery sql(db);
    if(sql.exec(str))
    {
        QStringList temp;
        sql.next();
        temp=sql.value("friends").toString().split(";");
        //根据好友列表获取头像名单
        for(auto i:temp)
        {
            QString table;
            if(i[0]=='T')
            {
                table="teacher_user_data";
            }
            else
            {
                table="student_user_data";
            }
            QString select=QString::asprintf("select head_photo from %1 where account = '%2';").arg(table).arg(i);
            if(sql.exec(select))
            {
                sql.next();
                list.append(sql.value("head_photo").toString());
            }
        }
    }
    //返回头像名单
    return list;
}

QString SqlData::getUserHeadPhoto(QString account)
{
    QString headPhoto;
    QString table;
    if(account[0]=='T')
    {
        table="teacher_user_data";
    }
    else
    {
        table="student_user_data";
    }
    QString str=QString::asprintf("select head_photo from %1 where account= '%2';").arg(table).arg(account);
    QSqlQuery sql(db);
    if(sql.exec(str))
    {
        sql.next();
        headPhoto=sql.value("head_photo").toString();
    }
    return headPhoto;
}

qint64 SqlData::getUserLoginTime(QString account)
{
    QString table;
    if(account[0]=='T')
    {
        table="teacher_user_data";
    }
    else
    {
        table="student_user_data";
    }
    QString str=QString::asprintf("select login_time from %1 where account = '%2';").arg(table).arg(account);
    QSqlQuery sql(db);
    if(sql.exec(str))
    {
        sql.next();
        return sql.value("login_time").toLongLong();
    }
    else
        return -1;
}

QString SqlData::getUserName(QString account)
{
    QString userName;
    QString table;
    if(account[0]=='T')
    {
        table="teacher_user_data";
    }
    else
    {
        table="student_user_data";
    }
    QString str=QString::asprintf("select username from %1 where account= '%2';").arg(table).arg(account);
    QSqlQuery sql(db);
    if(sql.exec(str))
    {
        sql.next();
        userName=sql.value("username").toString();
    }
    return userName;
}

QString SqlData::getGroupName(QString account)
{
    QString groupName;
    QString str=QString::asprintf("select username from groups_member where account= '%1';").arg(account);
    QSqlQuery sql(db);
    if(sql.exec(str))
    {
        sql.next();
        groupName=sql.value("username").toString();
    }
    return groupName;
}

QStringList SqlData::getAllFriend(QString account)
{
    QString str=QString::asprintf("select friends from relation where account ='%1';").arg(account);
    QStringList result;
    QSqlQuery sql(db);
    if(sql.exec(str))
    {
        if(sql.next())
        {
            QString friends=sql.value("friends").toString();
            if(!friends.isEmpty())
            {
                result=friends.split(";");
            }
        }
    }
    return result;
}

QStringList SqlData::getAllMember(QString account)
{
    QString str=QString::asprintf("select host_account, member from groups_member where account ='%1';").arg(account);
    QStringList result;
    QSqlQuery sql(db);
    if(sql.exec(str))
    {
        if(sql.next())
        {
            QString member=sql.value("member").toString();
            if(!member.isEmpty())
            {
                result=member.split(";");
            }
            QString hostAccount=sql.value("host_account").toString();
            if(!hostAccount.isEmpty())
            {
                result.append(hostAccount);
            }
        }
    }
    return result;
}

QString SqlData::getGroupHeadPhoto(QString account)
{
    QString headPhoto;
    QString str=QString::asprintf("select head_photo from groups_member where account= '%1';").arg(account);
    QSqlQuery sql(db);
    if(sql.exec(str))
    {
        sql.next();
        headPhoto=sql.value("head_photo").toString();
    }
    return headPhoto;
}

QString SqlData::getGroupHostAccount(QString account)
{
    QString str=QString::asprintf("select host_account from groups_member where account = '%1';").arg(account);
    QSqlQuery sql(db);
    sql.exec(str);
    sql.next();
    return sql.value("host_account").toString();
}

QByteArray SqlData::searchUser(QString condition,QString& headPhoto)
{
    QString str1="select * from student_user_data where account like '%"+condition+"%' or username like '%"+condition+"%';";
    QString str2="select * from teacher_user_data where account like '%"+condition+"%' or username like '%"+condition+"%';";
    qDebug()<<"str1:"<<str1;
    qDebug()<<"str2:"<<str2;
    QSqlQuery sql(db);
    QJsonArray array;
    sql.exec(str1);
    while(sql.next())
    {
        QJsonObject json;
        json.insert("account",sql.value("account").toString());
        json.insert("userName",sql.value("username").toString());
        json.insert("signature",sql.value("signature").toString());
        json.insert("sex",sql.value("sex").toString());
        json.insert("age",sql.value("age").toInt());
        json.insert("birthday",sql.value("birthday").toString());
        json.insert("location",sql.value("location").toString());
        json.insert("bloodType",sql.value("blood_type").toString());
        json.insert("college",sql.value("college").toString());
        json.insert("profession",sql.value("profession").toString());
        json.insert("headPhoto",sql.value("head_photo").toString());
        array.append(json);

        //添加头像名单(不为空则添加)
        if(!sql.value("head_photo").toString().isEmpty())
        {
            headPhoto+=Protocol::getAllUserPath()+"/"+sql.value("head_photo").toString()+"?";
        }
    }
    sql.exec(str2);
    while(sql.next())
    {
        QJsonObject json;
        json.insert("account",sql.value("account").toString());
        json.insert("userName",sql.value("username").toString());
        json.insert("signature",sql.value("signature").toString());
        json.insert("sex",sql.value("sex").toString());
        json.insert("age",sql.value("age").toInt());
        json.insert("birthday",sql.value("birthday").toString());
        json.insert("location",sql.value("location").toString());
        json.insert("bloodType",sql.value("blood_type").toString());
        json.insert("college",sql.value("college").toString());
        json.insert("headPhoto",sql.value("head_photo").toString());
        array.append(json);

        //添加头像名单(不为空则添加)
        if(!sql.value("head_photo").toString().isEmpty())
        {
            headPhoto+=Protocol::getAllUserPath()+"/"+sql.value("head_photo").toString()+"?";
        }
    }
    return QJsonDocument(array).toJson();
}

QByteArray SqlData::searchGroup(QString condition,QString& headPhoto)
{
    QString str="select * from groups_member where account like '%"+condition+"%' or username like '%"+condition+"%';";
    QSqlQuery sql(db);
    QJsonArray array;
    sql.exec(str);
    while(sql.next())
    {
        QJsonObject json;
        json.insert("account",sql.value("account").toString());
        json.insert("userName",sql.value("username").toString());
        json.insert("hostAccount",sql.value("host_account").toString());
        json.insert("signature",sql.value("signature").toString());
        json.insert("headPhoto",sql.value("head_photo").toString());
        array.append(json);

        //添加头像名单(不为空则添加)
        if(!sql.value("head_photo").toString().isEmpty())
        {
            headPhoto+=Protocol::getAllUserPath()+"/"+sql.value("head_photo").toString()+"?";
        }
    }
    return QJsonDocument(array).toJson();
}

QJsonObject SqlData::askForUserData(QString account,QString& headPhoto)
{
    QString table;
    if(account[0]=='T')
    {
        table="teacher_user_data";
    }
    else
    {
        table="student_user_data";
    }
    QString str=QString::asprintf("select * from %1 where account = '%2';").arg(table).arg(account);
    QSqlQuery sql(db);
    QJsonObject json;
    sql.exec(str);
    if(sql.next())
    {
        json.insert("account",sql.value("account").toString());
        json.insert("userName",sql.value("username").toString());
        json.insert("signature",sql.value("signature").toString());
        json.insert("sex",sql.value("sex").toString());
        json.insert("age",sql.value("age").toInt());
        json.insert("birthday",sql.value("birthday").toString());
        json.insert("location",sql.value("location").toString());
        json.insert("bloodType",sql.value("blood_type").toString());
        json.insert("college",sql.value("college").toString());
        if(table=="student_user_data")
            json.insert("profession",sql.value("profession").toString());
        json.insert("headPhoto",sql.value("head_photo").toString());
        //添加头像名单(不为空则添加)
        if(!sql.value("head_photo").toString().isEmpty())
        {
            headPhoto+=Protocol::getAllUserPath()+"/"+sql.value("head_photo").toString()+"?";
        }
    }
    return json;
}

QJsonObject SqlData::askForGroupData(QString account,QString& headPhoto)
{
    QString str=QString::asprintf("select * from groups_member where account = '%1';").arg(account);
    QSqlQuery sql(db);
    QJsonObject json;
    sql.exec(str);
    if(sql.next())
    {
        json.insert("account",sql.value("account").toString());
        json.insert("userName",sql.value("username").toString());
        json.insert("hostAccount",sql.value("host_account").toString());
        json.insert("signature",sql.value("signature").toString());
        json.insert("headPhoto",sql.value("head_photo").toString());

        //添加头像名单(不为空则添加)
        if(!sql.value("head_photo").toString().isEmpty())
        {
            headPhoto+=Protocol::getAllUserPath()+"/"+sql.value("head_photo").toString()+"?";
        }

        //添加群主个人信息
        QJsonArray array;
        array.append(askForUserData(sql.value("host_account").toString(),headPhoto));
        //循环添加成员信息
        if(!sql.value("member").toString().isEmpty())
        {
            QStringList list=sql.value("member").toString().split(";");
            for(QString string:list)
            {
                array.append(askForUserData(string,headPhoto));
            }
        }
        json.insert("member",array);
    }
    return json;
}

bool SqlData::isUserOnLine(QString account)
{
    QString table;
    if(account[0]=='T')
    {
        table="teacher_user_data";
    }
    else
    {
        table="student_user_data";
    }
    QString str=QString::asprintf("select onlineStatus from %1 where account = '%2';").arg(table).arg(account);
    QSqlQuery sql(db);
    if(sql.exec(str))
    {
        sql.next();
        if(sql.value("onlineStatus").toBool())
            return true;
        else
            return false;
    }
    return false;
}

void SqlData::userUpLine(QString account,qint64 loginTime)
{
    QString table;
    if(account[0]=='T')
    {
        table="teacher_user_data";
    }
    else
    {
        table="student_user_data";
    }
    QString str=QString::asprintf("update %1 set onlineStatus = true, login_time = %2 where account = '%3';").arg(table).arg(loginTime).arg(account);
    QSqlQuery sql(db);
    if(sql.exec(str))
    {
        qDebug()<<"用户："<<account<<"上线";
    }
    else
    {
        qDebug()<<"修改用户状态失败";
    }
}

void SqlData::userDownLine(QString account)
{
    QString table;
    if(account[0]=='T')
    {
        table="teacher_user_data";
    }
    else
    {
        table="student_user_data";
    }
    QString str=QString::asprintf("update %1 set onlineStatus = false where account = '%2';").arg(table).arg(account);
    QSqlQuery sql(db);
    if(sql.exec(str))
    {
        qDebug()<<"用户："<<account<<"下线";
    }
}

void SqlData::closeDataBase()
{
    QString str1="update student_user_data set onlineStatus = false;";
    QString str2="update teacher_user_data set onlineStatus = false;";
    QSqlQuery sql(db);
    if(sql.exec(str1))
    {
        qDebug()<<"关闭所有学生用户";
    }
    else
    {
        qDebug()<<"关闭学生用户失败";
    }
    if(sql.exec(str2))
    {
        qDebug()<<"关闭所有教师用户";
    }
    else
    {
        qDebug()<<"关闭教师用户失败";
    }
}

void SqlData::addFriend(QString account_1,QString account_2,qint64 updateTime)
{
    //确定两人所属的表
//    QString table_1,table_2;
//    if(account_1[0]=='T')
//    {
//        table_1="teacher_user_data";
//    }
//    else
//    {
//        table_1="student_user_data";
//    }
//    if(account_2[0]=='T')
//    {
//        table_2="teacher_user_data";
//    }
//    else
//    {
//        table_2="student_user_data";
//    }
    QSqlQuery sql(db);

    //查找用户1的数据
    QString str="select friends from relation where account = '"+account_1+"' and friends not like '%"+account_2+"%';";
    qDebug()<<str;
    sql.exec(str);
    if(sql.next())
    {
        //不存在则添加
        QString friends=sql.value("friends").toString();
        if(friends.isEmpty())
        {
            friends+=account_2;
        }
        else
        {
            friends+=";"+account_2;
        }
        str="update relation set friends = '"+friends+"' , update_time = "+ QString::number(updateTime) +" where account ='"+account_1+"';";
        sql.exec(str);
    }

    //查找用户2的数据
    str="select friends from relation where account = '"+account_2+"' and friends not like '%"+account_1+"%';";
    sql.exec(str);
    if(sql.next())
    {
        //不存在则添加
        QString friends=sql.value("friends").toString();
        if(friends.isEmpty())
        {
            friends+=account_1;
        }
        else
        {
            friends+=";"+account_1;
        }
        str="update relation set friends = '"+friends+"' , update_time = "+ QString::number(updateTime) +" where account = '"+account_2+"';";
        sql.exec(str);
    }
}

void SqlData::joinGroup(QString account, QString group,qint64 updateTime)
{
//    QString table;
//    if(account[0]=='T')
//    {
//        table="teacher_user_data";
//    }
//    else
//    {
//        table="student_user_data";
//    }

    QSqlQuery sql(db);

    //修改用户的联系人信息
    QString str="select all_groups from relation where account = '"+account+"' and all_groups not like '%"+group+"%';";
    sql.exec(str);
    if(sql.next())
    {
        QString allGroups=sql.value("all_groups").toString();
        if(allGroups.isEmpty())
        {
            allGroups+=group;
        }
        else
        {
            allGroups+=";"+group;
        }
        str="update relation set all_groups = '"+allGroups+"' , update_time = "+ QString::number(updateTime) +" where account = '"+account+"';";
        sql.exec(str);
    }

    //修改群聊信息
    str="select member from groups_member where account = '"+group+"' and member not like '%"+account+"%';";
    sql.exec(str);
    if(sql.next())
    {
        QString member=sql.value("member").toString();
        if(member.isEmpty())
        {
            member+=account;
        }
        else
        {
            member+=";"+account;
        }
        str="update groups_member set member = '"+member+"' , update_time = "+ QString::number(updateTime) +" where account = '"+group+"';";
        sql.exec(str);
    }
}

QJsonObject SqlData::taskToAddFriend(QJsonObject json)
{
    QString account=json.value("sender").toString();
    QJsonObject result;
    result.insert("type",json.value("type").toInt());
    result.insert("account",account);
    result.insert("userName",getUserName(account));
    result.insert("headPhoto",getUserHeadPhoto(account));
    return result;
}

QJsonObject SqlData::taskToJoinGroup(QJsonObject json)
{
    QString account=json.value("sender").toString();
    QString group=json.value("messageType").toString();
    QJsonObject result;
    result.insert("type",json.value("type").toInt());
    result.insert("account",account);
    result.insert("userName",getUserName(account));
    result.insert("headPhoto",getUserHeadPhoto(account));
    result.insert("groupAccount",group);
    result.insert("groupName",getGroupName(group));
    return result;
}

void SqlData::setRelationUpdateTime(QString account, qint64 updateTime)
{
    QString str=QString::asprintf("update relation set update_time = %1 where account ='%2';").arg(updateTime).arg(account);
    QSqlQuery sql(db);
    sql.exec(str);
}

QJsonObject SqlData::taskToSendDynamic(QJsonObject json)
{
    QString account=json.value("sender").toString();
    QJsonObject result;
    result.insert("type",json.value("type").toInt());
    result.insert("account",account);
    result.insert("userName",getUserName(account));
    result.insert("headPhoto",getUserHeadPhoto(account));
    result.insert("message",json.value("message").toString());
    result.insert("time",json.value("time"));
    return result;
}

bool SqlData::isGroupExist(QString account)
{
    QString str=QString::asprintf("select account from groups_member where account = '%1';").arg(account);
    QSqlQuery sql(db);
    sql.exec(str);
    if(sql.next())
        return true;
    else
        return false;
}

QString SqlData::createGroup(QString groupName, QString hostAccount,qint64 time)
{
    QString groupAccount="G";
    qsrand(QDateTime::currentMSecsSinceEpoch());
    int num=qrand()+100000000;
    while(isGroupExist(groupAccount+QString::number(num).left(8)))
    {
        num++;
    }
    groupAccount+=QString::number(num).left(8);
    QString str=QString::asprintf("insert into groups_member (account , username , host_account , update_time) values('%1','%2','%3',%4);")
                      .arg(groupAccount).arg(groupName).arg(hostAccount).arg(time);
    qDebug()<<str;
    QSqlQuery sql(db);
    sql.exec(str);
    str=QString::asprintf("select all_groups from relation where account = '%1';").arg(hostAccount);
    sql.exec(str);
    if(sql.next())
    {
        QString groups=sql.value("all_groups").toString();
        if(groups.isEmpty())
        {
            groups+=groupAccount;
        }
        else
        {
            groups+=";"+groupAccount;
        }
        str=QString::asprintf("update relation set all_groups = '%1' , update_time = %2 where account = '%3';")
                  .arg(groups).arg(time).arg(hostAccount);
        sql.exec(str);
    }
    return groupAccount;
}

void SqlData::modifyUserData(QString account, QJsonObject json)
{
    QString table,str;
    if(account[0]=='T')
    {
        table="teacher_user_data";
    }
    else
    {
        table="student_user_data";
    }
    QString headPhoto=json.value("headPhoto").toString();
    if(headPhoto.isEmpty())
        headPhoto=getUserHeadPhoto(account);
    if(account[0]=='E')
    {
        //数据顺序：用户名-》个性签名-》性别-》年龄-》生日-》血型-》位置-》学院-》专业(只有学生用户有专业信息)
        str=QString::asprintf("update %1 set username = '%2' , signature = '%3' , sex = '%4' , age = %5 , birthday ='%6' , blood_type ='%7' , location = '%8' , college = '%9' , profession ='%10' , head_photo = '%11' ,update_time = %12 where account = '%13';")
                .arg(table).arg(json.value("userName").toString())
                .arg(json.value("signature").toString()).arg(json.value("sex").toString())
                .arg(json.value("age").toInt()).arg(json.value("birthday").toString())
                .arg(json.value("bloodType").toString()).arg(json.value("location").toString())
                .arg(json.value("college").toString()).arg(json.value("profession").toString())
                .arg(headPhoto)
                .arg(json.value("updateTime").toString().toLongLong()).arg(account);
    }
    else
    {
        str=QString::asprintf("update %1 set username = '%2' , signature = '%3' , sex = '%4' , age = %5 , birthday ='%6' , blood_type ='%7' , location = '%8' , college = '%9' , head_photo = '%10' , update_time = %11 where account = '%12';")
                  .arg(table).arg(json.value("userName").toString())
                  .arg(json.value("signature").toString()).arg(json.value("sex").toString())
                  .arg(json.value("age").toInt()).arg(json.value("birthday").toString())
                  .arg(json.value("bloodType").toString()).arg(json.value("location").toString())
                  .arg(json.value("college").toString())
                  .arg(headPhoto)
                  .arg(json.value("updateTime").toString().toLongLong()).arg(account);
    }
    QSqlQuery sql(db);
    sql.exec(str);
}

void SqlData::modifyGroupData(QString account, QJsonObject json)
{
    QString headPhoto=json.value("headPhoto").toString();
    if(headPhoto.isEmpty())
        headPhoto=getGroupHeadPhoto(account);
    QString str=QString::asprintf("update groups_member set username = '%1' , signature = '%2' , head_photo = '%3' , update_time = %4 where account = '%5';")
                      .arg(json.value("userName").toString()).arg(json.value("signature").toString())
                      .arg(headPhoto).arg(json.value("updateTime").toString().toLongLong())
                      .arg(account);
    QSqlQuery sql(db);
    sql.exec(str);
}

bool SqlData::addMessageTask(QJsonObject json)
{
    QString str=QString::asprintf("insert into task (id,sender,receiver,message,message_type,file,type) values ("
                       "%1,'%2','%3','%4','%5','%6',%7 );").arg(json.value("time").toString().toLongLong())
                      .arg(json.value("sender").toString()).arg(json.value("receiver").toString())
                      .arg(json.value("message").toString()).arg(json.value("messageType").toString())
                      .arg(json.value("file").toString()).arg(json.value("type").toInt());
    QSqlQuery sql(db);
    if(sql.exec(str))
    {
        qDebug()<<"添加消息任务成功";
        return true;
    }
    else
        return false;
}

bool SqlData::deleteMessageTask(QString account)
{
    QString str=QString::asprintf("delete from task where receiver = '%1';").arg(account);
    QSqlQuery sql(db);
    if(sql.exec(str))
    {
        qDebug()<<"删除任务成功";
        return true;
    }
    else
        return false;
}

bool SqlData::judgeUserRegistrationMessage(QString account, QString identity, QString message)
{
    QString table;

    //分割message
    //存储顺序：姓名-》密码-》性别-》学院
    //如果是学生末尾会加上专业信息
    QStringList messageList=message.split("\n");
    QString name,password,sex,college,profession="";
    name=messageList.at(0);
    password=messageList.at(1);
    sex=messageList.at(2);
    college=messageList.at(3);

    //根据身份确定信息表和是否有专业信息
    if(identity=="教师")
    {
        table="teacher";
    }
    else
    {
        table="student";
        profession=messageList.at(4);
    }
    //搜索语句
    QString str=QString::asprintf("select * from %1 where id = '%2' and name = '%3' and sex = '%4' "
                      "and college ='%5';").arg(table).arg(account).arg(name).arg(sex)
                      .arg(college);
    qDebug()<<"注册str："<<str;
    QSqlQuery sql(db);
    //执行失败返回false
    if(sql.exec(str))
    {
        //没有匹配项返回false
        if(sql.next())
        {
            //如果身份是教师，则信息匹配
            if(identity=="教师")
                return true;
            else
            {
                //如果是学生还得匹配身份信息
                if(sql.value("profession").toString()==profession)
                    return true;
                else
                    return false;
            }
        }
        else
            return false;
    }
    else
        return false;
}

bool SqlData::userExist(QString account)
{
    //判断去哪个表查询
    QString table;
    if(account[0]=='T')
    {
        table="teacher_user_data";
    }
    else
    {
        table="student_user_data";
    }
    //设置查询语句
    QString str=QString::asprintf("select * from %1 where account = '%2';").arg(table).arg(account);
    QSqlQuery sql(db);
    //执行并返回结果，存在true,不存在false
    if(sql.exec(str))
    {
        if(sql.next())
        {
            return true;
        }
        else
            return false;
    }
    else
        return false;
}

void SqlData::userRegister(QString account, QString password)
{
    //根据账号设置表信息
    QString table_1,table_2;
    if(account[0]=='T')
    {
        table_1="teacher";
        table_2="teacher_user_data";
    }
    else
    {
        table_1="student";
        table_2="student_user_data";
    }
    //存储原始数据表中的信息
    QString name,sex,college,profession;
    QString str=QString::asprintf("select * from %1 where id = '%2';").arg(table_1).arg(account);
    QSqlQuery sql(db);
    if(sql.exec(str))
    {
        sql.next();
        name=sql.value("name").toString();
        sex=sql.value("sex").toString();
        college=sql.value("college").toString();
        //根据account的数据设置插入语句，teacher没有专业信息，将信息插入用户数据表
        if(account[0]!='T')
        {
            profession=sql.value("profession").toString();
            str=QString::asprintf("INSERT INTO %1 (account, pwd, onlinestatus, userName, signature, sex, age,"
                                    "birthday, location, blood_type, college, profession, update_time,"
                                    "login_time, head_photo)VALUES ('%2', '%3', 0, '%4', '', '%5', 18,"
                                    "'2003-09-01', '中国', '未知', '%6', '%7', 0, 0,'');").arg(table_2)
                                    .arg(account).arg(password).arg(name).arg(sex).arg(college)
                                    .arg(profession);
        }
        else
        {
            str=QString::asprintf("INSERT INTO %1 (account, pwd, onlinestatus, userName, signature, sex, age,"
                                    "birthday, location, blood_type, college, update_time,"
                                    "login_time, head_photo)VALUES ('%2', '%3', 0, '%4', '', '%5', 18,"
                                    "'2003-09-01', '中国', '未知', '%6', 0, 0,'');").arg(table_2)
                              .arg(account).arg(password).arg(name).arg(sex).arg(college);
        }
        //执行并输出结果
        sql.exec(str);

        //往联系人表中插入信息
        str=QString::asprintf("insert into relation (account,friends,all_groups,update_time) values ('%1','','',0);")
                  .arg(account);//groups有sql的保留字，会出错，所以只能改成all_groups了
        sql.exec(str);
        qDebug()<<str;
        qDebug()<<"用户："<<account<<"注册成功";
    }
}

QVector<QJsonObject> SqlData::sendMessageTask(QString account)
{
    //以降序存储messageTask，即先收到的消息先发送，后收到的后发送
    QString str=QString::asprintf("select * from task where receiver = '%1' order by id ASC;").arg(account);
    QVector<QJsonObject>jsonVector;
    QSqlQuery sql(db);
    if(sql.exec(str))
    {
        while(sql.next())
        {
            QJsonObject json;
            json.insert("time",sql.value("id").toString());
            json.insert("sender",sql.value("sender").toString());
            json.insert("receiver",sql.value("receiver").toString());
            json.insert("message",sql.value("message").toString());
            json.insert("messageType",sql.value("message_type").toString());
            json.insert("file",sql.value("file").toString());
            json.insert("type",sql.value("type").toInt());
            jsonVector.push_back(json);
        }
    }
    //读取完信息后记得删除任务信息
    deleteMessageTask(account);
    return jsonVector;
}

SqlData::~SqlData()
{
    //qDebug()<<"系统关闭";
    closeDataBase();
}
