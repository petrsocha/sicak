!include( config.pri ) {
  error( "Couldn't find the config.pri file!" )
}

INCLUDEPATH    += $$PWD/include

unix {

  QMAKE_CXXFLAGS -= -fstack-protector-strong
  QMAKE_CFLAGS -= -fstack-protector-strong
  QMAKE_CXXFLAGS_RELEASE -= -fstack-protector-strong
  QMAKE_CFLAGS_RELEASE -= -fstack-protector-strong  
  
  QMAKE_CXXFLAGS += -fno-stack-protector
  QMAKE_CFLAGS += -fno-stack-protector  
  QMAKE_CXXFLAGS_RELEASE += -fno-stack-protector
  QMAKE_CFLAGS_RELEASE += -fno-stack-protector
  
}
