!include( ../../../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}


TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
HEADERS        += predictaes128back.h
SOURCES        += predictaes128back.cpp                
TARGET          = $$qtLibraryTarget(sicakpredictaes128back)
DESTDIR         = ./bin

EXAMPLE_FILES = predictaes128back.json

# install
target.path = ../../../INSTALL/plugins/blockprocess
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
 
