!include( ../../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}

TEMPLATE    = subdirs

#
# Keysight 3000 oscilloscope plug-in
#

unix {
    SUBDIRS     += keysight3000    
}

win32 {
    !isEmpty(VISAINCLUDES){
        SUBDIRS     += keysight3000    
    }
}

#
# PicoScope 6000 oscilloscope plug-in
#

!isEmpty(PS6000INCLUDES){
    SUBDIRS     += ps6000    
}

