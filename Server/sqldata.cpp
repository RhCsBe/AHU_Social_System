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
