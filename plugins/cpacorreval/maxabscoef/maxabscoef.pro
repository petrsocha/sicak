!include( ../../../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}


TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
HEADERS        += maxabscoef.h
SOURCES        += maxabscoef.cpp                
TARGET          = $$qtLibraryTarget(sicakmaxabscoef)
DESTDIR         = ./bin

EXAMPLE_FILES = maxabscoef.json

# install
target.path = ../../../INSTALL/plugins/cpacorreval
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
 
