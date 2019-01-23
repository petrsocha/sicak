/*
*  SICAK - SIde-Channel Analysis toolKit
*  Copyright (C) 2019 Petr Socha, FIT, CTU in Prague
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
* \file smartcard.cpp
*
* \brief SICAK character device plugin: SmartCard
*
*
* \author Petr Socha
* \version 1.0
*/

#include "smartcard.h"

SmartCard::SmartCard(): m_initialized(false), m_recvBuf(65536+2), m_recvBufLen(0) {
    
}

SmartCard::~SmartCard() {
    if (m_initialized) {
        (*this).deInit();
    }
}

QString SmartCard::getPluginName() {
    return "Win32 SmartCard";
}

QString SmartCard::getPluginInfo() {
    return "Open using given Device ID.";
}

void SmartCard::init(const char * filename, int baudrate, int parity, int stopBits) {    
    
    if (m_initialized) {
        throw RuntimeException("The module is already initialized.");
    }
    
    #ifdef _WIN32

    size_t id = atoi(filename);
    DWORD ret;
    LPSTR mszReaders;
    DWORD pcchReaders = SCARD_AUTOALLOCATE;
    DWORD protocol;

    SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &m_context);

    ret = SCardListReadersA(m_context, NULL, (LPSTR)&mszReaders, &pcchReaders);

    if (ret != SCARD_S_SUCCESS) {
        SCardReleaseContext(m_context);
        throw RuntimeException("Failed to list card readers.");
    }

    size_t currId = 0;
    LPSTR mszReadersTmp = mszReaders;

    while (*mszReadersTmp != 0 && currId != id) {

        // skip to next reader
        mszReadersTmp += strlen(mszReadersTmp) + 1;
        currId++;

    }

    if (currId != id) {
        SCardReleaseContext(m_context);
        throw InvalidInputException("Failed to find the specified card reader.");
    }

    ret = SCardConnectA(m_context, mszReadersTmp, SCARD_SHARE_EXCLUSIVE, SCARD_PROTOCOL_T1, &m_card, &protocol);

    if (ret != SCARD_S_SUCCESS) {
        SCardReleaseContext(m_context);
        throw RuntimeException("Failed to connect to the card.");
    }

    SCardFreeMemory(m_context, mszReaders);

    ret = SCardBeginTransaction(m_card);

    if (ret != SCARD_S_SUCCESS) {
        SCardDisconnect(m_card, SCARD_LEAVE_CARD);
        SCardReleaseContext(m_context);
        throw RuntimeException("Failed to begin a card transaction.");
    }

    m_initialized = true;

    // also check status of the card (after this, uve got five seconds till the smartcard connection expires, unless some operation on it is performed which resets the os timer)

    BYTE atr[32];
    DWORD atrLen = 32;
    DWORD state;

    ret = SCardStatus(m_card, NULL, NULL, &state, NULL, atr, &atrLen);

    if (ret != SCARD_S_SUCCESS) {
        throw RuntimeException("Failed to check the card status.");
    }

    switch (state) {
        case SCARD_ABSENT:
            throw RuntimeException("There is no card in the reader.");
            break;
        case SCARD_PRESENT:
            throw RuntimeException("There is a card in the reader, but it has not been moved into position for use.");
            break;
        case SCARD_SWALLOWED:
            throw RuntimeException("There is a card in the reader in position for use. The card is not powered.");
            break;
        case SCARD_POWERED:
            throw RuntimeException("Power is being provided to the card, but the reader driver is unaware of the mode of the card.");
            break;
        case SCARD_NEGOTIABLE:
            throw RuntimeException("The card has been reset and is awaiting PTS negotiation.");
            break;
        case SCARD_SPECIFIC:
            //printf("The card has been reset and specific communication protocols have been established.");
            break;
        default:
            throw RuntimeException("Unknown or unexpected card state.");
            break;
    }

    #else

    throw RuntimeException("Only Win32 platform is supported.");

    #endif
   
}

void SmartCard::deInit() {
    
    if (!m_initialized) {
        throw RuntimeException("The module needs to be initialized first.");
    }
    
    #ifdef _WIN32    

    SCardEndTransaction(m_card, SCARD_LEAVE_CARD);
    SCardDisconnect(m_card, SCARD_LEAVE_CARD);
    SCardReleaseContext(m_context);    

    #else

    throw RuntimeException("Only Win32 platform is supported.");

    #endif
    
    m_initialized = false;
    
}

QString SmartCard::queryDevices() {     
    
    #ifdef _WIN32

    QString retDevices = "";
    DWORD ret;
    LPSTR mszReaders;
    DWORD pcchReaders = SCARD_AUTOALLOCATE;

    SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &m_context);	
    ret = SCardListReadersA(m_context, NULL, (LPSTR)&mszReaders, &pcchReaders);

    if (ret == SCARD_S_SUCCESS) {

        size_t id = 0;
        LPSTR mszReadersTmp = mszReaders;
        
        // mszReaders is a Multi-$tring, e.g. "abc\0def\0\0"
        while (*mszReadersTmp != 0) {

            retDevices += "    * Device ID: '";
            retDevices += QString::number(id);
            retDevices += "': '";
            retDevices += mszReadersTmp;
            retDevices += "'\n";

            // move to the next string
            mszReadersTmp += strlen(mszReadersTmp) + 1;
            id++;

        }

        if (id == 0) {
            retDevices += "    * No SmartCard reader found!\n";
        }

    } else if(ret == SCARD_E_NO_READERS_AVAILABLE) {
        retDevices = "    * No SmartCard reader found!\n";

    } else {
        retDevices = "    * Error querying SmartCard readers!\n";

    }

    SCardFreeMemory(m_context, mszReaders);
    SCardReleaseContext(m_context);

    return retDevices;

    #else

    return "    * Only Win32 platform is currently supported by this module!\n";

    #endif
    
}

void SmartCard::setTimeout(int ms) {
            
    #ifdef _WIN32
    
    return;

    #else

    throw RuntimeException("Only Win32 platform is supported.");

    #endif
        
}

size_t SmartCard::send(const uint8_t * buffer, size_t len) {
    
    if (!m_initialized) {
        throw RuntimeException("The module needs to be initialized first.");
    }
    
    #ifdef _WIN32

    DWORD ret;
    DWORD recvLen = (DWORD)m_recvBuf.size();

    ret = SCardTransmit(m_card, SCARD_PCI_T1, buffer, len, NULL, m_recvBuf.data(), &recvLen);

    if(ret != SCARD_S_SUCCESS){
        throw RuntimeException("Smart card data command-response transmission failed.");
    }
    
    m_recvBufLen = recvLen;

    return len;

    #else

    throw RuntimeException("Only Win32 platform is supported.");

    #endif
    
    return 0;
    
}

size_t SmartCard::send(const VectorType<uint8_t> & data, size_t len) {
    
    if(len >= data.size()) throw InvalidInputException("Not enough data to send");    
    
    return (*this).send(data.data(), len);
    
}

size_t SmartCard::send(const VectorType<uint8_t> & data) {        
    
    return (*this).send(data.data(), data.size());
    
}

size_t SmartCard::receive(uint8_t * buffer, size_t len) {
    
    if (!m_initialized) {
        throw RuntimeException("The module needs to be initialized first.");
    }
    
    #ifdef _WIN32

    size_t lenData = (len < m_recvBufLen) ? len : m_recvBufLen;

    for (size_t i = 0; i < lenData; i++) {
        buffer[i] = m_recvBuf(i);
    }

    return lenData;

    #else

    throw RuntimeException("Only Win32 platform is supported.");

    #endif
    
    return 0;
    
}

size_t SmartCard::receive(VectorType<uint8_t> & data, size_t len) {
    
    data.init(len); //< Make sure we have enough space to receive len chars
    
    return (*this).receive(data.data(), len);
    
}

size_t SmartCard::receive(VectorType<uint8_t> & data) {    
    
    return (*this).receive(data.data(), data.size());
    
}

