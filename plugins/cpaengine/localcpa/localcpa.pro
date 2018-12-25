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
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_LFLAGS_RELEASE -= -O2
    
    QMAKE_CXXFLAGS_RELEASE += -O3
    QMAKE_LFLAGS_RELEASE += -O3
}

TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
INCLUDEPATH    += ../common
HEADERS        += localcpa.h
SOURCES        += localcpa.cpp                
TARGET          = $$qtLibraryTarget(sicakcpa)
DESTDIR         = ./bin

EXAMPLE_FILES = localcpa.json

# install
target.path = ../../../INSTALL/plugins/cpaengine
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
