#ifndef MAININTERFACE_H
#define MAININTERFACE_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainInterface; }
QT_END_NAMESPACE

class MainInterface : public QWidget
{
    Q_OBJECT

public:
    MainInterface(QWidget *parent = nullptr);
    ~MainInterface();

private:
    Ui::MainInterface *ui;
};
#endif // MAININTERFACE_H
