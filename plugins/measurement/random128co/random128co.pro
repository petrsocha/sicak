!include( ../../../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}


TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
HEADERS        += random128co.h
SOURCES        += random128co.cpp                
TARGET          = $$qtLibraryTarget(sicakrandom128co)
DESTDIR         = ./bin

EXAMPLE_FILES = random128co.json

# install
target.path = ../../../INSTALL/plugins/measurement
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
 
