!include( ../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}

INCLUDEPATH    += ./include
CONFIG += console
QT -= gui

HEADERS += include/prep.h
SOURCES    = src/main.cpp \
             src/prep.cpp

TARGET     = prep
QMAKE_PROJECT_NAME = prep

DESTDIR    = ./bin

# install
target.path = ../INSTALL
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
