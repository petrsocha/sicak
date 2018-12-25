!include( ../../../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}


TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
HEADERS        += ttest128co.h
SOURCES        += ttest128co.cpp                
TARGET          = $$qtLibraryTarget(sicakttest128co)
DESTDIR         = ./bin

EXAMPLE_FILES = ttest128co.json

# install
target.path = ../../../INSTALL/plugins/measurement
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
 
