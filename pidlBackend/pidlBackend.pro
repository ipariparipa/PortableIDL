include(../global.pri)

TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -L../pidlCore -lpidlCore

INCLUDEPATH += ../pidlCore/include

SOURCES += \
    codegencontext.cpp \
    cppcodegen.cpp \
    cppwriter.cpp \
    cscodegen.cpp \
    cstyledocumentation.cpp \
    cswriter.cpp \
    job_json.cpp \
    json_cscodegen.cpp \
    json_stl_codegen.cpp \
    jsonreader.cpp \
    jsonwriter.cpp \
    language.cpp \
    object.cpp \
    objectfactory_json.cpp \
    operation.cpp \
    reader.cpp \
    writer.cpp \
    xmlreader.cpp

HEADERS += \
    include/pidlBackend/codegencontext.h \
    include/pidlBackend/config.h \
    include/pidlBackend/cppcodegen.h \
    include/pidlBackend/cppcodegenfactory_json.h \
    include/pidlBackend/cppwriter.h \
    include/pidlBackend/cscodegen.h \
    include/pidlBackend/cscodegenfactory_json.h \
    include/pidlBackend/cstyledocumentation.h \
    include/pidlBackend/cstyledocumentationfactory_json.h \
    include/pidlBackend/cswriter.h \
    include/pidlBackend/job_json.h \
    include/pidlBackend/json_cscodegen.h \
    include/pidlBackend/json_stl_codegen.h \
    include/pidlBackend/jsonreader.h \
    include/pidlBackend/jsonwriter.h \
    include/pidlBackend/language.h \
    include/pidlBackend/object.h \
    include/pidlBackend/objectfactory_json.h \
    include/pidlBackend/operation.h \
    include/pidlBackend/operationfactory_json.h \
    include/pidlBackend/reader.h \
    include/pidlBackend/readerfactory_json.h \
    include/pidlBackend/writer.h \
    include/pidlBackend/writerfactory_json.h \
    include/pidlBackend/xmlreader.h


