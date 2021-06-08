
!include("user.pri") { }

CONFIG(PIDL_BACKTRACE) {
    QMAKE_CXXFLAGS += -g1
}
