#-------------------------------------------------
#
# Project created by QtCreator 2015-04-10T11:47:02
#
#-------------------------------------------------

QT += core qml

ROOT_DIR = ../..

CONFIG(debug, debug|release): DESTDIR = $${ROOT_DIR}/Output/debug
CONFIG(release, debug|release): DESTDIR = $${ROOT_DIR}/Output/release

TARGET = top_couchdb
TEMPLATE = lib

CONFIG += staticlib c++11

HEADERS += \
    couchdbenums.h \
    couchdb.h \
    couchdbserver.h \
    couchdbresponse.h \
    couchdbquery.h \
    couchdblistener.h

SOURCES += \
    couchdb.cpp \
    couchdbserver.cpp \
    couchdbresponse.cpp \
    couchdbquery.cpp \
    couchdblistener.cpp

