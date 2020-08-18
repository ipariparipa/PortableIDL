include(../global.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -llog4cxx

SOURCES += \
    datetime.cpp \
    errorcollector.cpp \
    exception.cpp \
    jsontools.cpp

HEADERS += \
    include/pidlCore/config.h \
    include/pidlCore/datetime.h \
    include/pidlCore/errorcollector.h \
    include/pidlCore/exception.h \
    include/pidlCore/jsontools.h \
    include/pidlCore/nullable.h \
    include/pidlCore/platform.h \
    include/pidlCore/basictypes.h


