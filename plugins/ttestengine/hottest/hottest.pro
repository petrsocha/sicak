!include( ../../../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}

#
# OpenMP library is a dependency
#

win32 {
    QMAKE_CXXFLAGS_RELEASE += /openmp
}
unix { 
    QMAKE_CXXFLAGS_RELEASE += -fopenmp
    QMAKE_LFLAGS_RELEASE += -fopenmp
}

TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
INCLUDEPATH    += ../common
HEADERS        += hottest.h
SOURCES        += hottest.cpp                
TARGET          = $$qtLibraryTarget(sicakhottest)
DESTDIR         = ./bin

EXAMPLE_FILES = hottest.json

# install
target.path = ../../../INSTALL/plugins/ttestengine
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
