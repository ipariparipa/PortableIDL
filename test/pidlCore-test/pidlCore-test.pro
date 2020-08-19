
include("../../global.pri")

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
#TARGET =

#QMAKE_CXXFLAGS += -std=c++17

SOURCES += main.cpp \
           datetime_test.cpp

HEADERS += \
           datetime_test.h

LIBS += -L../../pidlCore -lpidlCore
INCLUDEPATH += ../../pidlCore/include

LIBS += -lcppunit -lcrypto -lpthread
