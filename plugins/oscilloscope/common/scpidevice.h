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
* \file scpidevice.h
*
* \brief Class providing SCPI device interface, either using USBTMC Linux Module, or using VISA libraries
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef SCPIDEVICE_H
#define SCPIDEVICE_H

#include <memory>
#include <string>
#include <cstring>

#ifdef _WIN32

#include <visa.h>

#else

#include <unistd.h>
#include <fcntl.h>

#endif

#include "exceptions.hpp"
    
/**
* \class ScpiDevice
*
* \brief SCPI device interface, using either USBTMC Linux module or VISA library.
*
*/    
class ScpiDevice {
    
protected:
    
    #ifdef _WIN32

    ViSession m_defaultRM;
    ViSession m_instrument;

    #else

    int m_osHandle;

    #endif
    
    bool m_opened;
    
public:
    
    ScpiDevice();        
    
    virtual ~ScpiDevice();
    
    /// Initialize the device, using either USBTMC device filename (e.g. /dev/usbtmc0) on Linux, or using VISA address (e.g. USBInstrument1) on Windows
    virtual void init(const char * filename);
    /// Deinitialize the device
    virtual void deInit();
    
    /// Send string to the device
    virtual size_t sendString(const std::string & data);        
    /// Receive string from the device and store in data
    virtual size_t receiveString(std::string & data);
    
    /// Send query string, wait for the answer and store in response
    virtual size_t queryString(const std::string & query, std::string & response);
    
    /// Send binary block of data to the device, using IEEE-488.2 data format
    virtual size_t sendIEEEBlock(const std::string & command, const char * data, size_t len);
    /// Receive binary block of data from the device, using IEEE-488.2 data format
    virtual size_t receiveIEEEBlock(char * data, size_t len);
    
    /// Send query string, wait for the IEEE-488.2 data block answer
    virtual size_t queryIEEEBlock(const std::string & query, char * response, size_t responseLen);
    
    /// Check for instrument errors
    virtual int checkForInstrumentErrors(std::string & response);
    
};



#endif /* SCPIDEVICE_H */
