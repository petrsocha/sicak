!include( ../../../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}

#
# PicoScope libraries are a dependency
#

!isEmpty(PS6000INCLUDES){
    INCLUDEPATH +=  $$PS6000INCLUDES
} else {
    error( "Could not find the PicoScope includes path" )
}

!isEmpty(PS6000LIBSDIR){
    !isEmpty(PS6000LIB){
        win32 {
            LIBS += $${PS6000LIBSDIR}/$${PS6000LIB}
        }
        unix {
            LIBS += -L$${PS6000LIBSDIR} -l:$${PS6000LIB}
        }        
    } else {
        error( "Could not find the PicoScope lib name" )    
    }
} else {
    error( "Could not find the PicoScope libs path" )  
}


TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
HEADERS        += ps6000.h
SOURCES        += ps6000.cpp                
TARGET          = $$qtLibraryTarget(sicakps6000)
DESTDIR         = ./bin

EXAMPLE_FILES = ps6000.json

# install
target.path = ../../../INSTALL/plugins/oscilloscope
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
