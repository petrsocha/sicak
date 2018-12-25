!include( ../../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}

TEMPLATE    = subdirs
SUBDIRS     += localttest

#
# OpenCL plugin
#

#!isEmpty(OPENCLINCLUDES){
#    SUBDIRS     += oclcpa    
#}

