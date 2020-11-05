include(../global.pri)

TEMPLATE = lib
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++17

TARGET_FILE = $$_PRO_FILE_PWD_/include/pidlCore/platform.h
unix {
    TEMPLATE_FILE = $$_PRO_FILE_PWD_/include/pidlCore/_platform_linux.h_
}

win32 {
    TEMPLATE_FILE = $_PRO_FILE_PWD_/include/pidlCore/_platform_win.h_
}

win64 {
    TEMPLATE_FILE = $_PRO_FILE_PWD_/include/pidlCore/_platform_win.h_
}

platform.input = TEMPLATE_FILE
platform.output = $$TARGET_FILE
platform.variable_out = HEADERS
platform.commands = cp $$TEMPLATE_FILE $$TARGET_FILE
QMAKE_EXTRA_COMPILERS += platform

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
    include/pidlCore/basictypes.h


