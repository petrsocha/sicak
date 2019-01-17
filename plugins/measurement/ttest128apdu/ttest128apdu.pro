!include( ../../../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}


TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
HEADERS        += ttest128apdu.h
SOURCES        += ttest128apdu.cpp                
TARGET          = $$qtLibraryTarget(sicakttest128apdu)
DESTDIR         = ./bin

EXAMPLE_FILES = ttest128apdu.json

# install
target.path = ../../../INSTALL/plugins/measurement
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
 
