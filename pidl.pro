
include("global.pri")

TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    pidlCore \
    pidlBackend \
    pidl \
    test \

pidlBackend.depends += pidlCore
pidl.depends += pidlBackend pidlCore
test.depends += pidlCore
