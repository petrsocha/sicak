!include( ../../../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}


TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
HEADERS        += mincoef.h
SOURCES        += mincoef.cpp                
TARGET          = $$qtLibraryTarget(sicakmincoef)
DESTDIR         = ./bin

EXAMPLE_FILES = mincoef.json

# install
target.path = ../../../INSTALL/plugins/cpacorreval
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
 
