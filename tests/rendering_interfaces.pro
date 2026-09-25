include(../common.pri)
TEMPLATE = app
TARGET = rendering_interfaces
CONFIG += console testcase
QT += testlib
INCLUDEPATH += ../src/lib
SOURCES += rendering_interfaces.cc ../src/lib/webkitpage.cc ../src/lib/resourceloader.cc
HEADERS += ../src/lib/rendering.hh ../src/lib/resourceloader.hh ../src/lib/webkitpage.hh
