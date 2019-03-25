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
    
    # enable more agressive optimization    
    QMAKE_CXXFLAGS_RELEASE += -O2
    QMAKE_LFLAGS_RELEASE += -O2
    
    QMAKE_CXXFLAGS_RELEASE += -O3
    QMAKE_LFLAGS_RELEASE += -O3
    
    QMAKE_CXXFLAGS_RELEASE += -ffast-math
    QMAKE_LFLAGS_RELEASE += -ffast-math
}

TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
INCLUDEPATH    += ../common
HEADERS        += hocpa.h
SOURCES        += hocpa.cpp                
TARGET          = $$qtLibraryTarget(sicakhocpa)
DESTDIR         = ./bin

EXAMPLE_FILES = hocpa.json

# install
target.path = ../../../INSTALL/plugins/cpaengine
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
