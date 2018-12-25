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
* \file chardevice.h
*
* \brief Character device plugin interface for use e.g. in meas
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef CHARDEVICE_H
#define CHARDEVICE_H

#include <QString>
#include "types_basic.hpp"

/**
* \class CharDevice
* \ingroup SicakInterface
*
* \brief Character device QT plugin interface
*
*/
class CharDevice {        
    
public:

    virtual ~CharDevice() {}

    /// Plugin name
    virtual QString getPluginName() = 0;
    /// Plugin info
    virtual QString getPluginInfo() = 0;
    
    /// Initialize the plugin with device given by filename, parity: 0=none,1=odd,2=even
    virtual void init(const char * filename, int baudrate = 9600, int parity = 0, int stopBits = 1) = 0;
    /// Deinitialize the plugin
    virtual void deInit() = 0;
    
    /// Query available devices/filenames
    virtual QString queryDevices() = 0;
    
    /// Set communication timeout of the character device, in milliseconds, default 5s
    virtual void setTimeout(int ms = 5000) = 0;
    
    /// Sends out the VectorType
    virtual size_t send(const VectorType<uint8_t> & data) = 0;
    /// Fills the given VectorType
    virtual size_t receive(VectorType<uint8_t> & data) = 0;
    
    /// Sends out the specified amount of bytes from VectorType
    virtual size_t send(const VectorType<uint8_t> & data, size_t len) = 0;    
    /// Receives the specified amount of bytes and saves them in given VectorType
    virtual size_t receive(VectorType<uint8_t> & data, size_t len) = 0;
    
    /// Sends out the specified amount of data from buffer
    virtual size_t send(const uint8_t * buffer, size_t len) = 0;		
    /// Receives the specified amount of data into the buffer
    virtual size_t receive(uint8_t * buffer, size_t len) = 0;
    
};        

#define CharDevice_iid "cz.cvut.fit.Sicak.CharDeviceInterface/1.0"

Q_DECLARE_INTERFACE(CharDevice, CharDevice_iid)


#endif /* CHARDEVICE_H */
