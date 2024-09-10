#include "maininterface.h"

#include <QApplication>
#include "login.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon("://photo/AHU1.png"));
    MainInterface w;
    w.show();
    Login login;
    login.show();
    ChatWindow chat;
    chat.show();
    return a.exec();
}
