!include( ../../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}

TEMPLATE    = subdirs
SUBDIRS     += localcpa

#
# OpenCL plugin
#

!isEmpty(OPENCLINCLUDES){
    SUBDIRS     += oclcpa    
}

