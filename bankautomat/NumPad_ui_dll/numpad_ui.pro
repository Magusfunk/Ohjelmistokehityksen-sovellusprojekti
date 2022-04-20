QT -= gui
QT += widgets

TEMPLATE = lib
DEFINES += NUMPAD_UI_LIBRARY

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    numpad_ui.cpp

HEADERS += \
    numpad_ui_global.h \
    numpad_ui.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

FORMS += \
    numpad_ui.ui
