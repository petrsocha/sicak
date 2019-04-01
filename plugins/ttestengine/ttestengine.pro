!include( ../../sicak.pri ) {
  error( "Couldn't find the sicak.pri file!" )
}

TEMPLATE    = subdirs
SUBDIRS     += localttest \
               hottest

#
# OpenCL plugin
#

#!isEmpty(OPENCLINCLUDES){
#    SUBDIRS     += oclcpa    
#}

