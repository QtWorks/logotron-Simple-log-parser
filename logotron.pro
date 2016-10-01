QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include("../orion-qt/orion.pri")

TARGET = logotron
TEMPLATE = app

SOURCES += main.cpp\
    MainWindow.cpp \
    LogItem.cpp \
    LogProcessor.cpp \
    LogTableWidget.cpp \
    LogFilterPanel.cpp \
    LogItemWidget.cpp \
    OpenFilesDialog.cpp \
    RegexExamWindow.cpp

HEADERS  += \
    MainWindow.h \
    LogItem.h \
    LogProcessor.h \
    LogTableWidget.h \
    LogFilterPanel.h \
    LogItemWidget.h \
    OpenFilesDialog.h \
    RegexExamWindow.h

DESTDIR = $$_PRO_FILE_PWD_/bin

win32 {
    RC_FILE = app.rc
}

RESOURCES += \
    images.qrc


