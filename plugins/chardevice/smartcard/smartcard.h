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
* \file smartcard.h
*
* \brief SICAK character device plugin: SmartCard
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef SMARTCARD_H
#define SMARTCARD_H 

#include <QObject>
#include <QtPlugin>
#include "chardevice.h"
#include "exceptions.hpp"

#ifdef _WIN32

#include <winscard.h>

#else


#endif

/**
* \class SmartCard
* \ingroup CharDevice
*
* \brief SmartCard Win32 SICAK CharDevice plugin
*
*/
class SmartCard : public QObject, CharDevice {
    
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cz.cvut.fit.Sicak.CharDeviceInterface/1.0" FILE "smartcard.json")
    Q_INTERFACES(CharDevice)
                
public:
    
    SmartCard();
    virtual ~SmartCard() override;
    
    virtual QString getPluginName() override;
    virtual QString getPluginInfo() override;
    
    /// filename is card reader ID, other params are ignored
    virtual void init(const char * filename, int baudrate = 9600, int parity = 0, int stopBits = 1) override;
    virtual void deInit() override;
    
    virtual QString queryDevices() override;
    
    /// setting timeout is not supported, no-op
    virtual void setTimeout(int ms = 5000) override;
    
    /// The Vector/buffer must contain a valid APDU message. Given the SmartCard nature, the function blocks until a response is received back
    virtual size_t send(const VectorType<uint8_t> & data) override;
    /// Response to the previously sent APDU message is stored in data, including status word
    virtual size_t receive(VectorType<uint8_t> & data) override;
    
    /// The Vector/buffer must contain a valid APDU message. Given the SmartCard nature, the function blocks until a response is received back
    virtual size_t send(const VectorType<uint8_t> & data, size_t len) override;
    /// Response to the previously sent APDU message is stored in data, including status word
    virtual size_t receive(VectorType<uint8_t> & data, size_t len) override;

    /// The Vector/buffer must contain a valid APDU message. Given the SmartCard nature, the function blocks until a response is received back
    virtual size_t send(const uint8_t * buffer, size_t len) override;		
    /// Response to the previously sent APDU message is stored in data, including status word
    virtual size_t receive(uint8_t * buffer, size_t len) override;
    
protected:
    
    bool m_initialized;

    #ifdef _WIN32

    SCARDCONTEXT m_context;
    SCARDHANDLE m_card;    
    
    #else	

    
    #endif
    
    Vector<uint8_t> m_recvBuf;
    size_t m_recvBufLen;
    
};

#endif /* SMARTCARD_H */
 
