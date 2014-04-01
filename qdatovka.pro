#-------------------------------------------------
#
# Project created by QtCreator 2014-03-24T10:01:11
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qdatovka
TEMPLATE = app

QMAKE_CXXFLAGS = \
	-g -O0 -std=c++11 \
	-Wall -Wextra -pedantic

SOURCES += src/main.cpp\
    src/gui/datovka.cpp \
    src/gui/dlg_preferences.cpp \
    src/gui/dlg_proxysets.cpp \
    src/models/accounts_model.cpp \
    src/models/messages_remote_models.cpp

HEADERS += src/common.h \
    src/gui/datovka.h \
    src/gui/dlg_preferences.h \
    src/gui/dlg_proxysets.h \
    src/models/accounts_model.h \
    src/models/messages_remote_models.h

FORMS += src/gui/ui/datovka.ui \
    src/gui/ui/dlg_preferences.ui \
    src/gui/ui/dlg_proxysets.ui

RESOURCES += \
    res/resources.qrc

TRANSLATIONS += locale/datovka_en.ts \
    locale/datovka_cs.ts

OTHER_FILES +=
