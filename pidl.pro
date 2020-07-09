
include("global.pri")

TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    pidlCore \
    pidlBackend \
    pidl \

pidlBackend.depends += pidlCore
pidl.depends += pidlBackend pidlCore
