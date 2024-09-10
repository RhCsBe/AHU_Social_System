QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chatwindow.cpp \
    frienditem.cpp \
    login.cpp \
    main.cpp \
    maininterface.cpp

HEADERS += \
    chatwindow.h \
    frienditem.h \
    login.h \
    maininterface.h

FORMS += \
    chatwindow.ui \
    frienditem.ui \
    login.ui \
    maininterface.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -LF:/QTMyWidget/QT5/QT-material-widget/ -lcomponents
else:win32:CONFIG(debug, debug|release): LIBS += -LF:/QTMyWidget/QT5/QT-material-widget/ -lcomponentsd

INCLUDEPATH += F:/QTMyWidget/QT5/QT-material-widget/components
DEPENDPATH += F:/QTMyWidget/QT5/QT-material-widget/components

RESOURCES += \
    photo.qrc
