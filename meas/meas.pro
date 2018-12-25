!include( ../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}

INCLUDEPATH    += ./include
CONFIG += console
QT -= gui

HEADERS += include/meas.h
SOURCES    = src/main.cpp \
             src/meas.cpp

TARGET     = meas
QMAKE_PROJECT_NAME = meas

DESTDIR    = ./bin

# install
target.path = ../INSTALL
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
