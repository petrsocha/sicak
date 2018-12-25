!include( ../../../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}


TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
HEADERS        += simplechar.h
SOURCES        += simplechar.cpp                
TARGET          = $$qtLibraryTarget(sicakplainchar)
DESTDIR         = ./bin

EXAMPLE_FILES = simplechar.json

# install
target.path = ../../../INSTALL/plugins/cpakeyeval
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
 
