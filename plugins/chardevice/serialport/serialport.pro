!include( ../../../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}


TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
HEADERS        += serialport.h
SOURCES        += serialport.cpp                
TARGET          = $$qtLibraryTarget(sicakserialport)
DESTDIR         = ./bin

EXAMPLE_FILES = serialport.json

# install
target.path = ../../../INSTALL/plugins/chardevice
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
 
