#include "frienditem.h"
#include "ui_frienditem.h"

FriendItem::FriendItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FriendItem)
{
    ui->setupUi(this);
}

FriendItem::~FriendItem()
{
    delete ui;
}
