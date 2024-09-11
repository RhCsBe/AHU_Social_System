#include "sqldata.h"

SqlData::SqlData()
{
    db=QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setDatabaseName("ahu_social_system");
    db.setUserName("root");
    db.setPassword("LJH030825");
    if(db.open())
    {
        QMessageBox::information(nullptr,"提示","连接数据库成功");
    }
    else
    {
        QMessageBox::warning(nullptr,"警告","连接数据库失败");
    }
}
