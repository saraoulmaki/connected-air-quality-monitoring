QT = core
QT += serialport

CONFIG += console
CONFIG -= app_bundle

TARGET = creaderasync
TEMPLATE = app

HEADERS += \
    aes.h \
    aes.hpp \
    serialportreader.h

SOURCES += \
    aes.c \
    main.cpp \
    serialportreader.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/serialport/creaderasync
INSTALLS += target

QT += sql
