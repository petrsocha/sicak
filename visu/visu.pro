!include( ../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}

INCLUDEPATH    += ./include
QT += gui charts widgets svg
CONFIG += console


HEADERS += include/visu.h
SOURCES    = src/main.cpp \
             src/visu.cpp

TARGET     = visu
QMAKE_PROJECT_NAME = visu

DESTDIR    = ./bin

# install
target.path = ../INSTALL
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
