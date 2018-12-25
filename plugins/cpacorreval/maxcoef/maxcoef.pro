!include( ../../../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}


TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
HEADERS        += maxcoef.h
SOURCES        += maxcoef.cpp                
TARGET          = $$qtLibraryTarget(sicakmaxcoef)
DESTDIR         = ./bin

EXAMPLE_FILES = maxcoef.json

# install
target.path = ../../../INSTALL/plugins/cpacorreval
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
 
