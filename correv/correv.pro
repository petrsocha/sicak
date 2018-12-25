!include( ../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}

INCLUDEPATH    += ./include
CONFIG += console
QT -= gui

HEADERS += include/correv.h
SOURCES    = src/main.cpp \
             src/correv.cpp

TARGET     = correv
QMAKE_PROJECT_NAME = correv

DESTDIR    = ./bin

# install
target.path = ../INSTALL
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
