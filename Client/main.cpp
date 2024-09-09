#include "maininterface.h"

#include <QApplication>
#include "login.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainInterface w;
    w.show();
    Login login;
    login.show();
    return a.exec();
}
