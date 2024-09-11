#ifndef SQLDATA_H
#define SQLDATA_H

#include <QObject>
#include <QtSql>
#include <QMessageBox>

class SqlData:public QObject
{
    Q_OBJECT
public:
    SqlData();

private:
    QSqlDatabase db;
};

#endif // SQLDATA_H
