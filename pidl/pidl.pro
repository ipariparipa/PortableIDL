#include(../global.pri)

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../pidlBackend/include
INCLUDEPATH += ../pidlCore/include

LIBS += -L../pidlBackend -lpidlBackend
LIBS += -L../pidlCore -lpidlCore
LIBS += -llog4cxx

SOURCES += \
    main.cpp

HEADERS += \
    config.h


