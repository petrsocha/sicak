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
* \file scpidevice.cpp
*
* \brief Class providing SCPI device interface, either using USBTMC Linux Module, or using (and requiring) VISA libraries on Win32
*
*
* \author Petr Socha
* \version 1.0
*/

#include "scpidevice.h"

ScpiDevice::ScpiDevice(): m_opened(false) {
    
}

ScpiDevice::~ScpiDevice(){

    if(m_opened) {
        
        (*this).deInit();

    }
}

void ScpiDevice::init(const char * filename){

    #ifdef _WIN32

    ViStatus ret;

    ret = viOpenDefaultRM(&m_defaultRM);
    if(ret < VI_SUCCESS) throw RuntimeException("Failed to open the default VISA device", ret);

    ret = viOpen(m_defaultRM, filename, VI_NULL, VI_NULL, &m_instrument);
    if (ret < VI_SUCCESS) {
        viClose(m_defaultRM);
        throw InvalidInputException("Failed to open the specified VISA device", ret);
    }

    ret = viSetAttribute(m_instrument, VI_ATTR_TMO_VALUE, 5000);
    if (ret < VI_SUCCESS) {
        viClose(m_instrument);
        viClose(m_defaultRM);
        throw RuntimeException("Failed to set VISA device timeout", ret);
    }

    #else

    m_osHandle = open(filename, O_RDWR);                
    if(m_osHandle < 0) throw InvalidInputException("Failed to open the specified usbtmc device", m_osHandle);
    
    #endif
    
    m_opened = true;

}

void ScpiDevice::deInit(){
            
    if(!m_opened) throw RuntimeException("The oscilloscope needs to be properly initialized first");
    
    #ifdef _WIN32

    viClose(m_instrument);
    viClose(m_defaultRM);

    #else

    close(m_osHandle);
    
    #endif

    m_opened = false;
    
}

size_t ScpiDevice::sendString(const std::string & data){
    
    if(!m_opened) throw RuntimeException("The device needs to be properly opened first");
    
    std::string buffer(data);

    // Check for an empty string
    if (buffer.length() == 0) return 0;

    // Check for the newline termination, and if not present, add one
    if (buffer.back() != '\n') buffer.append("\n");

    #ifdef _WIN32

    ViUInt32 retCount;

    // Send the newline-terminated string
    ViStatus ret = viBufWrite(m_instrument, (unsigned char *)buffer.c_str(), buffer.length(), &retCount);
    if(ret < VI_SUCCESS) throw RuntimeException("Could not write the data to the VISA device", ret);        

    #else
        
    // Send the newline-terminated string
    ssize_t retCount = write(m_osHandle, buffer.c_str(), buffer.length());
    if(retCount < 0) throw RuntimeException("Could not write the data to the usbtmc device", retCount);        
    
    #endif
    
    if((size_t)retCount != buffer.length()) throw RuntimeException("Could not send the whole command to the scpi device");
    
    return retCount;

}

size_t ScpiDevice::receiveString(std::string & data){
    
    if(!m_opened) throw RuntimeException("The device needs to be properly opened first");
    
    #define MAXSCPIRECVBUF 1024

    // Plus terminating zero
    char buffer[MAXSCPIRECVBUF + 1];
    buffer[0] = 0; // empty string

    #ifdef _WIN32

    ViStatus ret;
    ViUInt64 maxBufLen = MAXSCPIRECVBUF;

    // Read a string (%t is a string terminated by EOM (usbtmc end of message)) from the oscilloscope
    ret = viScanf(m_instrument, "%#t\n", &maxBufLen, buffer);
    if(ret < VI_SUCCESS) throw RuntimeException("Could not read the data from the usbtmc device", ret);

    size_t receivedBytes = maxBufLen;

    #else	        

    size_t receivedBytes = 0;
    ssize_t ret = 1; // start the loop
    
    // Expecting newline-terminated string, read till i get a newline, or timeout
    while( !(receivedBytes > 0 && buffer[receivedBytes-1] == '\n') && ret > 0) {
        
        ret = read(m_osHandle, buffer + receivedBytes, MAXSCPIRECVBUF - receivedBytes);
        if(ret < 0) throw RuntimeException("Could not read the data from the usbtmc device", ret);        
        receivedBytes += ret;            
        
    }                
    
    #endif		

    // Remove the newline
    if (receivedBytes > 0 && buffer[receivedBytes - 1] == '\n') {
        buffer[--receivedBytes] = 0;
    }
    else if (receivedBytes > 0) {
        throw RuntimeException("Missing the newline at the end of the received string");
    }

    // Store the received string in the output variable
    data = buffer;

    // Return the number of chars received
    return receivedBytes;

}

size_t ScpiDevice::queryString(const std::string & query, std::string & response){
       
    if(!sendString(query)) return 0;
    return receiveString(response);        
    
}

size_t ScpiDevice::sendIEEEBlock(const std::string & command, const char * data, size_t len) {
    
    if(!m_opened) throw RuntimeException("The device needs to be properly opened first");

    std::string header(command);

    // Check for an empty string
    if (header.length() == 0) return 0;

    // Check for the space separator, and if not present, add one
    if (header.back() != ' ') header.append(" ");

    // Prepare the IEEE 488 header
    std::string lenStr = std::to_string(len);
    if (lenStr.length() > 8) throw RuntimeException("Could not send this much data to the usbtmc device");
    header.append("#8");
    for (int i = 0; i < 8 - (int64_t)lenStr.length(); i++) header.append("0");
    header.append(lenStr);

    // Prepare the whole IEEE 488.2 data block message        
    int messageLen = header.length() + len + 1; // header + payload + newline 
    std::unique_ptr<char[]> buffer(new char[messageLen]);

    std::memcpy(buffer.get(), header.c_str(), header.length());
    std::memcpy(buffer.get() + header.length(), data, len);
    *(buffer.get() + header.length() + len) = '\n';

    #ifdef _WIN32

    ViUInt32 retCount;

    // Send the message
    ViStatus ret = viBufWrite(m_instrument, (unsigned char *)buffer.get(), messageLen, &retCount);
    if (ret < VI_SUCCESS) throw RuntimeException("Could not write the data to the VISA device", ret);        

    #else	        
    
    // Send the message                        
    ssize_t retCount = write(m_osHandle, buffer.get(), messageLen);
    if(retCount < 0) throw RuntimeException("Could not write the data to the usbtmc device", retCount);
            
    #endif	
    
    if(retCount != messageLen) throw RuntimeException("Could not send the whole message to the scpi device");            
    return retCount - header.length() - 1;

}

size_t ScpiDevice::receiveIEEEBlock(char * data, size_t len){
    
    if(!m_opened) throw RuntimeException("The device needs to be properly opened first");
    
    #ifdef _WIN32
    
    ViUInt32 maxBufLen = len;

    ViStatus ret = viScanf(m_instrument, "%#b\n", &maxBufLen, data);
    if(ret < VI_SUCCESS) throw RuntimeException("Could not read the data from the VISA device", ret);

    return maxBufLen;

    #else	

    char buffer[16];
            
    ssize_t ret;  
    size_t receivedBytes = 0;
    
    // Read the first part of the IEEE 488 data block header, i.e. "#n", where n is a Natural number
    ret = read(m_osHandle, buffer, 2);
    if(ret < 0) throw RuntimeException("Could not read the data from the usbtmc device", ret);  
    if(ret == 0) return 0; // timeout
    if(ret < 2){
        ret = read(m_osHandle, buffer+1, 1);
        if(ret < 0) throw RuntimeException("Could not read the data from the usbtmc device", ret);  
        if(ret == 0) return 0; // timeout
    }
    
    // Parse the header
    if(buffer[0] != '#' || buffer[1] < 48 || buffer[1] > 57) throw RuntimeException("Error parsing the IEEE 488 data block header");
    size_t noOfDigitsHeader = buffer[1] - 48;
    
    if(noOfDigitsHeader == 0) throw RuntimeException("Arbitrary length data block aren't supported");
    
    // Read the number of bytes to be transferred
    while(receivedBytes != noOfDigitsHeader){
        
        ret = read(m_osHandle, buffer+receivedBytes, noOfDigitsHeader-receivedBytes);
        if(ret < 0) throw RuntimeException("Could not read the data from the usbtmc device", ret);  
        if(ret == 0) return 0; // timeout
        receivedBytes += ret;
        
    }
    
    buffer[receivedBytes] = 0;        
    size_t bytesExpected = atol(buffer);                
    
    if(receivedBytes != noOfDigitsHeader || bytesExpected <= 0) throw RuntimeException("Error reading the IEEE 488 data block header");
    
    // Read the announced amount of bytes from the device
    receivedBytes = 0;
    if(bytesExpected > len) throw RuntimeException("Local recv buffer overflow");
    ret = 1; // start the loop
    
    while(receivedBytes != bytesExpected && ret > 0){
        
        ret = read(m_osHandle, data + receivedBytes, bytesExpected - receivedBytes);
        if(ret < 0) throw RuntimeException("Could not read the data from the usbtmc device", ret);  
        if(ret == 0) return 0;
        receivedBytes += ret;
        
    }
    
    if(receivedBytes != bytesExpected) throw RuntimeException("Failed to receive the IEEE 488 data block");
    
    // If everything went down smooth, we should read a newline char now
    ret = read(m_osHandle, buffer, 1);
    if(ret < 0) throw RuntimeException("Could not read the data from the usbtmc device", ret);  
    if(ret == 0) return 0;
    if(buffer[0] != '\n') throw RuntimeException("Something went wrong while receiving the IEEE 488 data block");
    
    return receivedBytes;
    
    #endif	

}

size_t ScpiDevice::queryIEEEBlock(const std::string & query, char * response, size_t responseLen){
        
    if(!sendString(query)) return 0;
    return receiveIEEEBlock(response, responseLen);
    
}

int ScpiDevice::checkForInstrumentErrors(std::string & response) {
        
    std::string errors;
    std::string temp;        
    
    do {
        
        queryString(":SYSTem:ERRor?", temp);        
        errors += temp;
        errors += "; ";
        
    } while (atoi(temp.c_str()) != 0);
    
    response = errors;
    
    return atoi(errors.c_str());
    
}
    
