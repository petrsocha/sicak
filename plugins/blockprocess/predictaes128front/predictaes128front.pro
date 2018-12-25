!include( ../../../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}


TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
HEADERS        += predictaes128front.h
SOURCES        += predictaes128front.cpp                
TARGET          = $$qtLibraryTarget(sicakpredictaes128front)
DESTDIR         = ./bin

EXAMPLE_FILES = predictaes128front.json

# install
target.path = ../../../INSTALL/plugins/blockprocess
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
 
