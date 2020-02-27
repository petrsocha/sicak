!include( config.pri ) {
  error( "Couldn't find the config.pri file!" )
}

INCLUDEPATH    += $$PWD/include

CONFIG += c++11
