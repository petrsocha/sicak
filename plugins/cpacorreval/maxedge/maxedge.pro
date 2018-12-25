!include( ../../../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}


TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
HEADERS        += maxedge.h
SOURCES        += maxedge.cpp                
TARGET          = $$qtLibraryTarget(sicakmaxedge)
DESTDIR         = ./bin

EXAMPLE_FILES = maxedge.json

# install
target.path = ../../../INSTALL/plugins/cpacorreval
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
 
