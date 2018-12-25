!include( ../../../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}

#
# OpenCL libraries are a dependency
#

!isEmpty(OPENCLINCLUDES){
    INCLUDEPATH +=  $$OPENCLINCLUDES
} else {
    error( "Could not find the OpenCL includes path" )
}

!isEmpty(OPENCLLIBSDIR){
    !isEmpty(OPENCLLIB){
        win32 {
            LIBS += $${OPENCLLIBSDIR}/$${OPENCLLIB}
        }
        unix {
            LIBS += -L$${OPENCLLIBSDIR} -l:$${OPENCLLIB}
        }        
    } else {
        error( "Could not find the OpenCL lib name" )    
    }
} else {
    error( "Could not find the OpenCL libs path" )  
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


unix {
    QMAKE_CXXFLAGS_RELEASE += -Wno-deprecated-declarations
}

TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
INCLUDEPATH    += ../common \
                  ../../common
HEADERS        += oclcpa.h
SOURCES        += oclcpa.cpp                
TARGET          = $$qtLibraryTarget(sicakoclcpa)
DESTDIR         = ./bin

EXAMPLE_FILES = oclcpa.json

# install
target.path = ../../../INSTALL/plugins/cpaengine
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
