/*
*  SICAK - SIde-Channel Analysis toolKit
*  Copyright (C) 2018-2019 Petr Socha, FIT, CTU in Prague
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
* \file keysight3000.cpp
*
* \brief SICAK oscilloscope plugin: Keysight 3000 series (formerly agilent), using SCPIDevice, which may require VISA library
*
*
* \author Petr Socha
* \version 1.1
*/

#include "keysight3000.h"

// multi-platform Sleep(ms)
#ifdef _WIN32

#include <windows.h>

#else	

#include <unistd.h>

int Sleep(int ms){
    return usleep(ms * 1000);
}

#endif

Keysight3000::Keysight3000(): m_samples(0), m_triggered(true), m_opened(false) {        // keysight 3000 series oscilloscope is by default triggered    
    
}

Keysight3000::~Keysight3000() {
    
    if(m_opened){
        
        (*this).deInit();
        
    }
                            
}

QString Keysight3000::getPluginName() {
    return "Keysight 3000 series oscilloscope (formerly Agilent)";
}

QString Keysight3000::getPluginInfo() {
    return "On Win32 open e.g. with \"USB0::2391::1031::PROTO03::0::INSTR\" or simply \"USBInstrument1\", on POSIX open e.g. with \"/dev/usbtmc0\".";
}

void Keysight3000::init(const char * filename) {
    
    m_handle.init(filename);
    
    std::string response;        
    m_handle.checkForInstrumentErrors(response); // clear all pending errors on the device
    
    m_opened = true;
    
}

void Keysight3000::deInit() {
    
    if(!m_opened) throw RuntimeException("The oscilloscope needs to be properly initialized first");
    
    m_handle.deInit();
    
    m_opened = false;
                            
}

QString Keysight3000::queryDevices() {     
    
    #ifdef _WIN32
    
    return "    * Device ID: 'VISAADDR', where VISAADDR is a VISA address of the oscilloscope, e.g. \"USBInstrument1\" for 1st USB device or e.g. \"USB0::2391::1031::PROTO03::0::INSTR\"\n      Use oscilloscope software to find out your device's VISA address.\n";
    
    #else	
    
    return "    * Device ID: 'FILEPATH', where FILEPATH is path to a usbtmc device, e.g. \"/dev/usbtmc0\"\n      Make sure you have permissions to access the file, and the usbtmc module loaded.\n";
    
    #endif
    
}

void Keysight3000::setChannel(int & channel, bool & enabled, Coupling & coupling, Impedance & impedance, int & rangemV, int & offsetmV, BandwidthLimiter & bwLimit){
        
    if(!m_opened) throw RuntimeException("The oscilloscope needs to be properly initialized first");
    if(channel < 1 || channel > 4) throw InvalidInputException("Invalid channel");
    
    // channel
    std::string base = ":CHANnel";
    base += std::to_string(channel);

    // enabled?
    std::string command = base;
    command += ":DISPlay ";
    command += enabled ? "1" : "0";            
    m_handle.sendString(command);
    
    // coupling
    command = base;
    command += ":COUPling ";
    if(coupling == Coupling::AC) {
        command += "AC";
    } else { // DC
        command += "DC";
    }
    m_handle.sendString(command);
    
    // impedance
    command = base;
    command += ":IMPedance ";
    if(impedance == Impedance::R50) {
        command += "FIFTy";
    } else { // R1M
        command += "ONEMeg";
    }
    m_handle.sendString(command);
    
    // range
    command = base;
    command += ":RANGe ";
    if(rangemV < 4) rangemV = 4;
    else if (rangemV > 20000) rangemV = 20000;
    command += std::to_string(rangemV * 2); // this api sets channel range as +-X, e.g. for rangemV = 1000, the oscilloscope is set to capture -1V to +1V
    command += "mV";
    m_handle.sendString(command);
    
    // offset
    command = base;
    command += ":OFFSet ";
    command += std::to_string((-1) * offsetmV);
    command += " mV";
    m_handle.sendString(command);
    
    // bwlimit
    command = base;
    command += ":BWLimit ";
    if(bwLimit == BandwidthLimiter::FULL){
        command += "0"; // limiter off
    } else {
        command += "1";
    }
    m_handle.sendString(command);
    
    std::string response;
    int ret = m_handle.checkForInstrumentErrors(response);
    if(ret) throw RuntimeException("Error setting channel parameters", ret);
    
    // read parameters back            
    
    command = base;
    command += ":COUPling?";
    m_handle.queryString(command, response);
    if(response.compare("AC") == 0){
        coupling = Coupling::AC;
    } else if (response.compare("DC") == 0){
        coupling = Coupling::DC;
    } else {
        throw RuntimeException("Invalid oscilloscope answer");
    }
    
    command = base;
    command += ":IMPedance?";
    m_handle.queryString(command, response);
    if(response.compare("ONEM") == 0){
        impedance = Impedance::R1M;
    } else if (response.compare("FIFT") == 0){
        impedance = Impedance::R50;
    } else {
        throw RuntimeException("Invalid oscilloscope answer");
    }
    
    command = base;
    command += ":RANGe?";
    m_handle.queryString(command, response);
    float rdRange = std::strtof(response.c_str(), NULL);
    rdRange /= 2; // convert back to +-X format, defined by API and expected by user
    rdRange *= 1000; // V to mV conversion            
    rangemV = rdRange;
    
    command = base;
    command += ":OFFSet?";
    m_handle.queryString(command, response);
    float rdOffset = std::strtof(response.c_str(), NULL);
    rdOffset *= -1000; // V to mV conversion
    offsetmV = rdOffset;
    
    command = base;
    command += ":BWLimit?";
    m_handle.queryString(command, response);
    if(response.compare("1") == 0){
        bwLimit = BandwidthLimiter::F25MHZ;
    } else if (response.compare("0") == 0){
        bwLimit = BandwidthLimiter::FULL;
    } else {
        throw RuntimeException("Invalid oscilloscope answer");
    }
    
    ret = m_handle.checkForInstrumentErrors(response);
    if(ret) throw RuntimeException("Error reading channel parameters");
}


void Keysight3000::setTrigger(int & sourceChannel, float & level, TriggerSlope & slope){
        
    if(!m_opened) throw RuntimeException("The oscilloscope needs to be properly initialized first");
    if(sourceChannel < 1 || sourceChannel > 4) throw InvalidInputException("Invalid channel");
    if(level < -0.25f) level = -0.25;
    if(level > 1.25f) level = 1.25f;
    
    // Set trigger mode
    std::string command = ":TRIGger:MODE EDGE";
    m_handle.sendString(command);
    
    // source
    command = ":TRIGger:EDGE:SOURce CHANnel";
    command += std::to_string(sourceChannel);
    m_handle.sendString(command);
    
    // slope
    command = ":TRIGger:EDGE:SLOPe ";
    if(slope == TriggerSlope::RISING){
        command += "POSitive";
    } else if (slope == TriggerSlope::FALLING) { // FALLING
        command += "NEGative";
    } else {
        command += "EITHer";            
    }
    m_handle.sendString(command);
    
    // level                        
    float yRange;
    float yOffset;
    std::string response;
    std::string base = ":CHANnel";
    base += std::to_string(sourceChannel);
    
    command = base;            
    command += ":RANGe?";
    m_handle.queryString(command, response);
    yRange = std::strtof(response.c_str(), NULL);
    
    command = base;
    command += ":OFFSet?";
    m_handle.queryString(command, response);
    yOffset = std::strtof(response.c_str(), NULL);
    
    float tbsLevel = (yOffset - yRange/2) + yRange * level;
    char buffer[255];
    
    command = ":TRIGger:EDGE:LEVel ";
    snprintf(buffer, 255, "%E", tbsLevel);
    command += buffer;
    m_handle.sendString(command);
    
    int ret = m_handle.checkForInstrumentErrors(response);
    if(ret) throw RuntimeException("Error setting the trigger", ret);
    
    m_triggered = true;
    
    // TODO read back parameters
    
}

void Keysight3000::unsetTrigger(){
    
    m_triggered = false;
    
}

void Keysight3000::setTiming(float & preTriggerRange, float & postTriggerRange, size_t & samples, size_t & captures){
                
    if(!m_opened) throw RuntimeException("The oscilloscope needs to be properly initialized first");
    char buffer[255];                
    float range = preTriggerRange + postTriggerRange;
    
    std::string command = ":TIMebase:MODE MAIN";
    m_handle.sendString(command);
    
    command = ":TIMebase:REFerence CENTer";
    m_handle.sendString(command);                        
    
    command = ":TIMebase:RANGe ";        
    snprintf(buffer, 255, "%E", range);
    command += buffer;
    m_handle.sendString(command);                                      
            
    command = ":TIMebase:POSition ";
    snprintf(buffer, 255, "%E", (range/2.0f - preTriggerRange)); // move from center to left (range/2), then proportionally move to satisfy the pre-/post-trigger samples request
    command += buffer;
    m_handle.sendString(command);
    
    std::string response;
    
    int ret = m_handle.checkForInstrumentErrors(response);
    if(ret) throw RuntimeException("Error setting the timebase", ret);
    
    // TODO read parameters back
    
    captures = 1; // only support one capture at a time
    m_samples = dummyMeasurement();
    samples = m_samples;
    
}

void Keysight3000::run() {
    
    if(!m_opened) throw RuntimeException("The oscilloscope needs to be properly initialized first");        
    
    std::string response;
    m_handle.queryString(":STOP;*OPC?", response); 
    
    int ret = m_handle.checkForInstrumentErrors(response);
    if(ret) throw RuntimeException("Error before run", ret);
    
    Sleep(100);
    
    m_handle.sendString(":SINGle");
        
    // wait until armed
    int respInt;
    do {
        Sleep(100);
        m_handle.queryString(":AER?", response);
        respInt = atoi(response.c_str());
    } while (!respInt);
        
    // if the trigger is unset, force the oscilloscope to acquire data right now
    if(!m_triggered){
        m_handle.sendString(":TRIGger:FORCe");
    }
    
}

void Keysight3000::stop() {
    
    if(!m_opened) throw RuntimeException("The oscilloscope needs to be properly initialized first");
    
    std::string response;
    m_handle.queryString(":STOP;*OPC?", response);
    
    int ret = m_handle.checkForInstrumentErrors(response);
    if(ret) throw RuntimeException("Error stopping the oscilloscope", ret);
    
}

size_t Keysight3000::getCurrentSetup(size_t & samples, size_t & captures) {
       
    // do a dummy measurement to obtain a number of samples per trace                            
    captures = 1; // only support one capture at a time
    m_samples = dummyMeasurement();
    samples = m_samples;        
        
    return m_samples;
    
}

size_t Keysight3000::getValues(int channel, int16_t * buffer, size_t len, size_t & samples, size_t & captures) {
    
    if(!m_opened) throw RuntimeException("The oscilloscope needs to be properly initialized first");
    if(channel < 1 || channel > 4) throw Exception("Invalid channel");
    
    std::string response;
    std::string command;
    
    // wait for the aquisition to complete
    int respInt;
    do {
        Sleep(100);
        m_handle.queryString(":OPERegister:CONDition?", response);
        respInt = atoi(response.c_str()) & (1 << 3);
    } while (respInt);
    
    command = ":WAVeform:SOURce CHANnel";
    command += std::to_string(channel);
    m_handle.sendString(command);
    
    int ret = m_handle.checkForInstrumentErrors(response);
    if(ret) throw Exception("Error setting the source channel", ret);
    
    m_handle.queryString(":WAVeform:POINts?", response); 
    samples = atoi(response.c_str());
    captures = 1;
    
    if(samples * captures > len) throw Exception("Receiving buffer too small");
    
    Sleep(100);
    
    size_t recvRet = m_handle.queryIEEEBlock(":WAVeform:DATA?", reinterpret_cast<char *>(buffer), len*2) / 2;
    
    if(recvRet != samples) throw RuntimeException("Failed to download the power trace from oscilloscope: not enough samples");

    Sleep(100);
    
    ret = m_handle.queryString("*OPC?", response); // makes sure that everything got downloaded, otherwise causes -410 error
    
    ret = m_handle.checkForInstrumentErrors(response);
    if(ret) throw Exception("Error while downloading data", ret);
    
    return recvRet; 
    
}

size_t Keysight3000::getValues(int channel, PowerTraces<int16_t> & traces) {
    
    if(!m_opened) throw RuntimeException("The oscilloscope needs to be properly initialized first");
    
    if(channel < 1 || channel > 4) throw InvalidInputException("Invalid channel");        
    
    traces.init(m_samples, 1); //< alloc memory for aquisition
    
    size_t samples, captures;
    
    return (*this).getValues(channel, traces.data(), traces.size(), samples, captures);
} 

size_t Keysight3000::dummyMeasurement() {
    
    std::string response;
    m_handle.sendString(":ACQuire:COMPlete 100");
    m_handle.sendString(":ACQuire:TYPE NORMal");
    m_handle.sendString(":WAVeform:POINts:MODE RAW");
    m_handle.sendString(":WAVeform:FORMat WORD");
    m_handle.sendString(":WAVeform:UNSigned 0");                
    m_handle.sendString(":WAVeform:BYTeorder LSBFirst");
    m_handle.queryString(":STOP;*OPC?", response);        
    
    Sleep(100);
    
    m_handle.sendString(":SINGle");
    int respInt;
    do {
        Sleep(100);
        m_handle.queryString(":AER?", response);
        respInt = atoi(response.c_str());
    } while (!respInt);
    
    m_handle.sendString(":TRIGger:FORCe");  
    
    do {
        Sleep(100);
        m_handle.queryString(":OPERegister:CONDition?", response);
        respInt = atoi(response.c_str()) & (1 << 3);
    } while (respInt);
    
    m_handle.queryString(":WAVeform:POINts?", response);                      
    
    size_t samples = atoi(response.c_str());
    
    int ret = m_handle.checkForInstrumentErrors(response);
    if(ret) throw RuntimeException("Error doing a dummy measurement", ret);
        
    return samples;
    
}
