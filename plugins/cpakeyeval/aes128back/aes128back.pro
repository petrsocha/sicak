!include( ../../../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}


TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
HEADERS        += aes128back.h
SOURCES        += aes128back.cpp                
TARGET          = $$qtLibraryTarget(sicakaes128back)
DESTDIR         = ./bin

EXAMPLE_FILES = aes128back.json

# install
target.path = ../../../INSTALL/plugins/cpakeyeval
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
 
