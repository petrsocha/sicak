!include( ../../../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}

win32 {
    LIBS += Winscard.lib
}

TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
HEADERS        += smartcard.h
SOURCES        += smartcard.cpp                
TARGET          = $$qtLibraryTarget(sicaksmartcard)
DESTDIR         = ./bin

EXAMPLE_FILES = smartcard.json

# install
target.path = ../../../INSTALL/plugins/chardevice
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
 
