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
* \file serialport.cpp
*
* \brief SICAK character device plugin: serial port
*
*
* \author Petr Socha
* \version 1.0
*/

#include "serialport.h"

SerialPort::SerialPort(): m_opened(false) {
    
}

SerialPort::~SerialPort() {
    
    if(m_opened){
        
        (*this).deInit();
        
    }
    
}

QString SerialPort::getPluginName() {
    return "Win32/POSIX Serial Port";
}

QString SerialPort::getPluginInfo() {
    return "On Win32, open e.g. with \"\\\\.\\COM10\", on POSIX, open e.g. with \"/dev/ttyUSB0\".";
}

void SerialPort::init(const char * filename, int baudrate, int parity, int stopBits) {    
    
    // Opening the port
    #ifdef _WIN32

    m_osHandle = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (m_osHandle == INVALID_HANDLE_VALUE) {
        //CloseHandle(m_osHandle);
        throw InvalidInputException("Could not open the specified serial port");
    }

    #else	

    m_osHandle = open(filename, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);

    if(m_osHandle < 0)
        throw InvalidInputException("Could not open the specified serial port", m_osHandle);

    if(!isatty(m_osHandle)){
        close(m_osHandle);
        throw InvalidInputException("Specified filename is not a serial port");
    }

    if(fcntl(m_osHandle, F_SETFL, 0) < 0){
        close(m_osHandle);
        throw RuntimeException("Could not reset the serial port flags");
    }

    #endif
    
    m_opened = true;
    
    #ifdef _WIN32            

    BOOL status;

    DCB serialParams = { 0 };
    serialParams.DCBlength = sizeof(serialParams);

    status = GetCommState(m_osHandle, &serialParams);
    if(!status) throw RuntimeException("Could not get serial port parameters");				

    switch (baudrate) {

        case 110   : serialParams.BaudRate = CBR_110;    break;
        case 300   : serialParams.BaudRate = CBR_300;    break;
        case 600   : serialParams.BaudRate = CBR_600;    break;
        case 1200  : serialParams.BaudRate = CBR_1200;   break;
        case 2400  : serialParams.BaudRate = CBR_2400;   break;
        case 4800  : serialParams.BaudRate = CBR_4800;   break;
        case 9600  : serialParams.BaudRate = CBR_9600;   break;
        case 19200 : serialParams.BaudRate = CBR_19200;  break;
        case 38400 : serialParams.BaudRate = CBR_38400;  break;
        case 57600 : serialParams.BaudRate = CBR_57600;  break;
        case 115200: serialParams.BaudRate = CBR_115200; break;
        case 128000: serialParams.BaudRate = CBR_128000; break;
        case 256000: serialParams.BaudRate = CBR_256000; break;
        default: 
            if(baudrate > 0) {
                serialParams.BaudRate = (DWORD) baudrate;
            } else {
                throw InvalidInputException("Unsupported baud rate: supported rates are 110, 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 128000, 256000, or any other positive number (not guaranteed to work)"); 
            }
            break;

    }

    if (parity == 0) {

        serialParams.fParity = FALSE;
        serialParams.Parity = NOPARITY;

    } else {

        serialParams.fParity = TRUE;

        if (parity%2 == 0) {

            serialParams.Parity = EVENPARITY;

        } else { // ODD

            serialParams.Parity = ODDPARITY;

        }

    }

    if (stopBits == 2) {

        serialParams.StopBits = TWOSTOPBITS;

    } else { // ONE

        serialParams.StopBits = ONESTOPBIT;

    }
    
    // no flow control
    serialParams.fDtrControl = DTR_CONTROL_DISABLE;
    serialParams.fRtsControl = DTR_CONTROL_DISABLE;
    serialParams.fOutX = FALSE;
    serialParams.fInX = FALSE;
    serialParams.fOutxCtsFlow = FALSE;
    serialParams.fOutxDsrFlow = FALSE;
    serialParams.fDsrSensitivity = FALSE;    

    serialParams.ByteSize = 8; // only support 8-bit word

    serialParams.fBinary = TRUE; // non-binary transfers are not supported on Win anyway, according to msdn
    serialParams.fErrorChar = FALSE; // no corrections
    serialParams.fNull = FALSE; // dont discard null chars
    serialParams.fAbortOnError = FALSE; // dont abort on errors		

    status = SetCommState(m_osHandle, &serialParams);
    if (!status) throw RuntimeException("Could not set serial port parameters");		

    #else	                

    int status;

    struct termios serialParams;

    status = tcgetattr(m_osHandle, &serialParams);
    if(status) throw RuntimeException("Could not get serial port parameters", status);                                

    switch (baudrate) {

        case 110   : status = cfsetispeed(&serialParams, B110);    status |= cfsetospeed(&serialParams, B110);    break;
        case 300   : status = cfsetispeed(&serialParams, B300);    status |= cfsetospeed(&serialParams, B300);    break;
        case 600   : status = cfsetispeed(&serialParams, B600);    status |= cfsetospeed(&serialParams, B600);    break;
        case 1200  : status = cfsetispeed(&serialParams, B1200);   status |= cfsetospeed(&serialParams, B1200);   break;
        case 2400  : status = cfsetispeed(&serialParams, B2400);   status |= cfsetospeed(&serialParams, B2400);   break;
        case 4800  : status = cfsetispeed(&serialParams, B4800);   status |= cfsetospeed(&serialParams, B4800);   break;
        case 9600  : status = cfsetispeed(&serialParams, B9600);   status |= cfsetospeed(&serialParams, B9600);   break;
        case 19200 : status = cfsetispeed(&serialParams, B19200);  status |= cfsetospeed(&serialParams, B19200);  break;
        case 38400 : status = cfsetispeed(&serialParams, B38400);  status |= cfsetospeed(&serialParams, B38400);  break;
        case 57600 : status = cfsetispeed(&serialParams, B57600);  status |= cfsetospeed(&serialParams, B57600);  break;
        case 115200: status = cfsetispeed(&serialParams, B115200); status |= cfsetospeed(&serialParams, B115200); break;
        case 230400: status = cfsetispeed(&serialParams, B230400); status |= cfsetospeed(&serialParams, B230400); break;
        default: throw InvalidInputException("Unsupported baud rate: use one of 110, 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400"); break;

    }

    if(status) throw RuntimeException("Could not set input/output baudrate", status);

    if (parity == 0) {

        serialParams.c_cflag &= ~PARENB;   
        serialParams.c_cflag &= ~PARODD;

    } else {

        serialParams.c_cflag |= PARENB;                        

        if (parity%2 == 0) {

            serialParams.c_cflag &= ~PARODD;

        } else { // ODD

            serialParams.c_cflag |= PARODD;

        }

    }

    if (stopBits == 2) {

        serialParams.c_cflag |= CSTOPB;

    } else { // ONE

        serialParams.c_cflag &= ~CSTOPB;

    }    

    // No flow control
    serialParams.c_cflag &= ~CRTSCTS;
    serialParams.c_iflag &= ~(IXON | IXOFF | IXANY);
    
    serialParams.c_cflag &= ~CSIZE;
    serialParams.c_cflag |= CS8;    // only support 8-bit words
    serialParams.c_cflag |= (CLOCAL | CREAD);  // enable receiver

    serialParams.c_iflag |= (IGNPAR | IGNBRK); // ignore parity errors and break conditions                
    serialParams.c_iflag &= ~(BRKINT | ISTRIP | INLCR | IGNCR | ICRNL | PARMRK | INPCK); // dont process the input
    // UICLC is not in POSIX, so check first
    #ifdef IUCLC
    serialParams.c_iflag &= ~IUCLC;
    #endif

    serialParams.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG); // only raw input                
    #ifdef IEXTEN
    serialParams.c_lflag &= ~IEXTEN;
    #endif                                              

    serialParams.c_oflag &= ~(OCRNL | ONLCR | ONLRET | ONOCR | OFILL); // dont process the output
    #ifdef OLCUC
    serialParams.c_oflag &= ~OLCUC;
    #endif
    #ifdef ONOEOT
    serialParams.c_oflag &= ~ONOEOT;
    #endif
    #ifdef XTABS
    serialParams.c_oflag &= ~XTABS;
    #endif
    #ifdef OXTABS
    serialParams.c_oflag &= ~OXTABS;
    #endif                
    serialParams.c_oflag &= ~OPOST; // only raw output                                 

    status = tcflush(m_osHandle, TCIFLUSH);
    if(status) throw RuntimeException("Could not flush the serial port", status);

    status = tcsetattr(m_osHandle, TCSANOW, &serialParams);
    if(status) throw RuntimeException("Could not set serial port parameters", status);	

    #endif
    
    (*this).setTimeout();    
}

void SerialPort::deInit() {
    
    if(!m_opened) throw RuntimeException("The serial port needs to be properly initialized first");
    
    #ifdef _WIN32

    CloseHandle(m_osHandle);

    #else	

    tcflush(m_osHandle, TCIOFLUSH);
    close(m_osHandle);

    #endif
    
    m_opened = false;
    
}

QString SerialPort::queryDevices() {     
    
    #ifdef _WIN32
    
    return "    * Device ID: 'PORTNAME', where PORTNAME is a name of a serial port, e.g. \"COM3\" or \"\\\\.\\COM10\"\n";
    
    #else	
    
    return "    * Device ID: 'FILEPATH', where FILEPATH is path to a terminal device, e.g. \"/dev/ttyUSB0\"\n";
    
    #endif
    
}

void SerialPort::setTimeout(int ms) {
    
    if(!m_opened) throw RuntimeException("The serial port needs to be properly initialized first");
    
    #ifdef _WIN32

    BOOL status;

    COMMTIMEOUTS serialTimeouts = { 0 }; // in milliseconds

    serialTimeouts.ReadIntervalTimeout = 0; // no interval timeout

    serialTimeouts.ReadTotalTimeoutConstant = ms;	// set only total timeouts
    serialTimeouts.ReadTotalTimeoutMultiplier = 0;

    serialTimeouts.WriteTotalTimeoutConstant = ms;
    serialTimeouts.WriteTotalTimeoutMultiplier = 0;

    status = SetCommTimeouts(m_osHandle, &serialTimeouts);
    if (!status) throw RuntimeException("Could not set serial port timeouts");

    #else	

    int status;

    struct termios serialParams;

    status = tcgetattr(m_osHandle, &serialParams);
    if(status) throw RuntimeException("Could not get serial port parameters in order to set a timeout", status);  

    cc_t ds = ms / 100; // POSIX timeouted read takes deciseconds
    if( ( ms % 100 ) >= 50 || (ds == 0 && ms > 0) ) ds++; // round up; or set the lowest timeout possible if asking for less, unless setting no timeout at all                 

    serialParams.c_cc[VTIME] = ds;
    serialParams.c_cc[VMIN] = 0;

    status = tcflush(m_osHandle, TCIFLUSH);
    if(status) throw RuntimeException("Could not flush the serial port while setting the timeout", status);

    status = tcsetattr(m_osHandle, TCSANOW, &serialParams);
    if(status) throw RuntimeException("Could not set serial port timeout", status);

    #endif
        
}

size_t SerialPort::send(const uint8_t * buffer, size_t len) {
    
    if(!m_opened) throw RuntimeException("The serial port needs to be properly initialized first");
    
    #ifdef _WIN32

    DWORD bytesWritten = 0;
    BOOL status;

    status = WriteFile(m_osHandle, buffer, len, &bytesWritten, NULL);		
    if (!status) throw RuntimeException("Write to the serial port failed");
    
    if(len != bytesWritten) throw RuntimeException("Serial port write timeout.");

    return (size_t) bytesWritten;

    #else	

    ssize_t bytesWritten = write(m_osHandle, (const void *) buffer, len);                
    if(bytesWritten < 0) throw RuntimeException("Write to the serial port failed", bytesWritten);

    if(len != bytesWritten) throw RuntimeException("Serial port write timeout.");
    
    return (size_t) bytesWritten;

    #endif
    
}

size_t SerialPort::send(const VectorType<uint8_t> & data, size_t len) {
    
    if(len > data.size()) throw InvalidInputException("Not enough data to send");    
    
    return (*this).send(data.data(), len);
    
}

size_t SerialPort::send(const VectorType<uint8_t> & data) {        
    
    return (*this).send(data.data(), data.size());
    
}

size_t SerialPort::receive(uint8_t * buffer, size_t len) {
    
    if(!m_opened) throw RuntimeException("The serial port needs to be properly initialized first");
    
    #ifdef _WIN32

    DWORD bytesRead = 0;
    BOOL status;

    // ReadFile returns either when all the requested bytes have been read, or the timeout expires
    // (wont return with less data than required, unless the timeout expires)

    status = ReadFile(m_osHandle, buffer, len, &bytesRead, NULL);
    if (!status) throw RuntimeException("Read from the serial port failed");

    if(len != bytesRead) throw RuntimeException("Serial port read timeout.");
    
    return (size_t) bytesRead;

    #else	

    size_t bytesRead = 0;
    ssize_t readRet = 1; // set to allow the loop to start

    while(bytesRead < len && readRet > 0){

        // read returns either when any data is available, or when the timeout expires
        // (read may return immediately with a single byte of data available, wont wait for the rest of the requested data; hence the loop)
        // (actually, it might wait, it is really implementation dependent; prefer a posix compliant way)

        readRet = read(m_osHandle, (void *)(buffer + bytesRead), len - bytesRead);

        if(readRet < 0) {

            throw RuntimeException("Read from the serial port failed", readRet);

        } else {

            bytesRead += readRet;

        }                    

    }

    if(len != bytesRead) throw RuntimeException("Serial port read timeout.");
    
    return bytesRead;                

    #endif
    
}

size_t SerialPort::receive(VectorType<uint8_t> & data, size_t len) {
    
    data.init(len); //< Make sure we have enough space to receive len chars
    
    return (*this).receive(data.data(), len);
    
}

size_t SerialPort::receive(VectorType<uint8_t> & data) {    
    
    return (*this).receive(data.data(), data.size());
    
}

