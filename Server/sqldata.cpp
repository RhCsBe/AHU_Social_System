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

bool SqlData::addMessageTask(QJsonObject json)
{
    QString str=QString::asprintf("insert into task (id,sender,receiver,message,file,type) values ("
                       "%1,'%2','%3','%4','%5',%6);").arg(json.value("time").toString().toLongLong())
                      .arg(json.value("sender").toString()).arg(json.value("receiver").toString())
                      .arg(json.value("message").toString()).arg(json.value("file").toString())
                      .arg(json.value("type").toInt());
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
    QString str=QString::asprintf("select * from task where receiver = '%1' and type = %2 order by id ASC;").arg(account).arg(InfoType::SendMessage);
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
