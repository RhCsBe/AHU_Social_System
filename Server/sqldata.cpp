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

bool SqlData::searchUser(QString account, QString password)
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
        QStringList list=sql.value("friends").toString().split(";");
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
                if(table=="teacher_user_data")
                    json.insert("profession",sql.value("profession").toString());
                //json不提供长整形的存储和转换，所以要将长整形转成字符串存储在json中
                json.insert("headPhoto",sql.value("head_photo").toString());
            }
            jsonArray.append(json);
        }
    }
    return jsonArray;
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
