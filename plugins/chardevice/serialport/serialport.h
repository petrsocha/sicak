/*
*  SICAK - SIde-Channel Analysis toolKit
*  Copyright (C) 2018 Petr Socha, FIT, CTU in Prague
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/**
* \file serialport.h
*
* \brief SICAK character device plugin: serial port
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef SERIALPORT_H
#define SERIALPORT_H 

#include <QObject>
#include <QtPlugin>
#include "chardevice.h"
#include "exceptions.hpp"

#ifdef _WIN32

#include <windows.h>

#else

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#endif

/**
* \class SerialPort
* \ingroup CharDevice
*
* \brief Serial port interface (Win32/Posix) SICAK CharDevice plugin
*
*/
class SerialPort : public QObject, CharDevice {
    
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cz.cvut.fit.Sicak.CharDeviceInterface/1.0" FILE "serialport.json")
    Q_INTERFACES(CharDevice)
                
public:
    
    SerialPort();
    virtual ~SerialPort() override;
    
    virtual QString getPluginName() override;
    virtual QString getPluginInfo() override;
    
    /// Initializes the serial port, on Win32 e.g. filename=COM2, on Posix e.g. filename=/dev/ttyUSB0, parity: 0=none, 1=odd, 2=even
    virtual void init(const char * filename, int baudrate = 9600, int parity = 0, int stopBits = 1) override;
    virtual void deInit() override;
    
    virtual QString queryDevices() override;
    
    virtual void setTimeout(int ms = 5000) override;
    
    virtual size_t send(const VectorType<uint8_t> & data) override;
    virtual size_t receive(VectorType<uint8_t> & data) override;
    
    virtual size_t send(const VectorType<uint8_t> & data, size_t len) override;
    virtual size_t receive(VectorType<uint8_t> & data, size_t len) override;

    virtual size_t send(const uint8_t * buffer, size_t len) override;		
    virtual size_t receive(uint8_t * buffer, size_t len) override;
    
protected:

    #ifdef _WIN32

    HANDLE m_osHandle;

    #else	

    int m_osHandle;

    #endif
    
    bool m_opened;
    
};

#endif /* SERIALPORT_H */
 
