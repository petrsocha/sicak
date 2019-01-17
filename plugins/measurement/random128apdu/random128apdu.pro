!include( ../../../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}


TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
HEADERS        += random128apdu.h
SOURCES        += random128apdu.cpp                
TARGET          = $$qtLibraryTarget(sicakrandom128apdu)
DESTDIR         = ./bin

EXAMPLE_FILES = random128apdu.json

# install
target.path = ../../../INSTALL/plugins/measurement
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
 
