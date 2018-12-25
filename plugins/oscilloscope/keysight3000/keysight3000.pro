!include( ../../../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}

#
# On Windows, VISA libraries are a dependency od scpidevice
#

win32 {

  !isEmpty(VISAINCLUDES){
    INCLUDEPATH +=  $$VISAINCLUDES
  } else {
    error( "Could not find the VISA includes path" )
  }
  
  !isEmpty(VISALIBSDIR){
    !isEmpty(VISALIB){
        LIBS += $${VISALIBSDIR}/$${VISALIB}
    } else {
        error( "Could not find the VISA lib name" )    
    }
  } else {
    error( "Could not find the VISA libs path" )  
  }
  
}

#
# scpidevice, uses either pure posix calls, or VISA library on win
#

INCLUDEPATH    += ../common
SOURCES += ../common/scpidevice.cpp  
HEADERS += ../common/scpidevice.h

TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
HEADERS        += keysight3000.h
SOURCES        += keysight3000.cpp                
TARGET          = $$qtLibraryTarget(sicakkeysight3000)
DESTDIR         = ./bin

EXAMPLE_FILES = keysight3000.json

# install
target.path = ../../../INSTALL/plugins/oscilloscope
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
 
