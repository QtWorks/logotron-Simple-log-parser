QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include($$_PRO_FILE_PWD_/orion/orion.pri)

TARGET = logotron
TEMPLATE = app

SOURCES += main.cpp\
    Appearance.cpp \
    MainWindow.cpp \
    LogItem.cpp \
    LogProcessor.cpp \
    LogTableWidget.cpp \
    LogFilterPanel.cpp \
    LogItemWidget.cpp \
    OpenFilesDialog.cpp \
    RegexExamWindow.cpp

HEADERS  += \
    Appearance.h \
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


