!include( ../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}

INCLUDEPATH    += ./include
CONFIG += console
QT -= gui

HEADERS += include/stan.h
SOURCES    = src/main.cpp \
             src/stan.cpp

TARGET     = stan
QMAKE_PROJECT_NAME = stan

DESTDIR    = ./bin

# install
target.path = ../INSTALL
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
