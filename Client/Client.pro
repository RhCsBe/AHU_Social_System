QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    addwindow.cpp \
    chatwindow.cpp \
    chatwindowdelegate.cpp \
    creategroupwindow.cpp \
    dynamicdelegate.cpp \
    editdynamicwindow.cpp \
    frienditem.cpp \
    login.cpp \
    main.cpp \
    maininterface.cpp \
    messagedelegate.cpp \
    personal.cpp \
    protocol.cpp \
    register.cpp \
    searchwindow.cpp \
    tcpthread.cpp

HEADERS += \
    addwindow.h \
    chatwindow.h \
    chatwindowdelegate.h \
    creategroupwindow.h \
    dynamicdelegate.h \
    editdynamicwindow.h \
    frienditem.h \
    login.h \
    maininterface.h \
    messagedelegate.h \
    personal.h \
    protocol.h \
    register.h \
    searchwindow.h \
    tcpthread.h

FORMS += \
    addwindow.ui \
    chatwindow.ui \
    creategroupwindow.ui \
    editdynamicwindow.ui \
    frienditem.ui \
    login.ui \
    maininterface.ui \
    personal.ui \
    register.ui \
    searchwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -LF:/QTMyWidget/QT5/QT-material-widget/ -lcomponents
else:win32:CONFIG(debug, debug|release): LIBS += -LF:/QTMyWidget/QT5/QT-material-widget/ -lcomponentsd

INCLUDEPATH += F:/QTMyWidget/QT5/QT-material-widget/components
DEPENDPATH += F:/QTMyWidget/QT5/QT-material-widget/components

RESOURCES += \
    photo.qrc \
    ElaWidgetTools.qrc

win32:CONFIG(release, debug|release): LIBS += -LF:/QTMyWidget/QT5/ElaWidgetTools/ -llibelawidgettools.dll
else:win32:CONFIG(debug, debug|release): LIBS += -LF:/QTMyWidget/QT5/ElaWidgetTools/ -llibelawidgettools.dlld
else:unix: LIBS += -LF:/QTMyWidget/QT5/ElaWidgetTools/ -llibelawidgettools.dll

INCLUDEPATH += F:/QTMyWidget/QT5/ElaWidgetTools/src/include
DEPENDPATH += F:/QTMyWidget/QT5/ElaWidgetTools/src/include

