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
* \file ps6000.cpp
*
* \brief SICAK oscilloscope plugin: PicoScope 6000 series. Requires PicoScope 6000 SDK.
*
*
* \author Petr Socha
* \version 1.0
*/

#include "ps6000.h"

Ps6000::Ps6000(): m_preTriggerSamples(0), m_postTriggerSamples(0), m_timebase(0), m_captures(1), m_opened(false) {                       
        
}

Ps6000::~Ps6000() {
    
    if(m_opened){
            
        (*this).deInit();
        
    }
    
};

QString Ps6000::getPluginName() {
    return "PicoScope 6000 series oscilloscope";
}

QString Ps6000::getPluginInfo() {
    return "Open with oscilloscope serial number or leave empty to open first oscilloscope found.";
}

void Ps6000::init(const char * filename) {
    
    int8_t serial = atoi(filename);
    PICO_STATUS status = ps6000OpenUnit(&m_handle, ((serial) ? (&serial) : (NULL)) );
    if (status) throw InvalidInputException("Failed to open PicoScope");
    
    m_opened = true;
    
}

void Ps6000::deInit() {        
            
    if(!m_opened) throw RuntimeException("The oscilloscope needs to be properly initialized first");
    
    PICO_STATUS status = ps6000Stop(m_handle);	
    status = ps6000CloseUnit(m_handle);
    status = status; // prevent warning

    m_opened = false;
    
}

QString Ps6000::queryDevices() {     
    
    return "    * Device ID: 'SERIALNO', where SERIALNO is a serial number of the oscilloscope. Leave empty to let driver automatically select first device found.\n      On Linux, make sure you have permissions to access the device (/dev/usb/...).\n";
    
}

void Ps6000::setChannel(int & channel, bool & enabled, Coupling & coupling, Impedance & impedance, int & rangemV, int & offsetmV, BandwidthLimiter & bwLimit) {                        
    
    if(!m_opened) throw RuntimeException("The oscilloscope needs to be properly initialized first");    
    if(channel < 1 || channel > 4) throw InvalidInputException("Invalid channel");
    
    // channel
    PS6000_CHANNEL tbsChannel;            
    switch(channel){                
        case 2:  tbsChannel = PS6000_CHANNEL_B; break;
        case 3:  tbsChannel = PS6000_CHANNEL_C; break;
        case 4:  tbsChannel = PS6000_CHANNEL_D; break;
        default: tbsChannel = PS6000_CHANNEL_A; channel = 1; break; // case 1
    }
    
    // enabled?
    int16_t tbsEnabled = enabled ? true : false;
    
    // coupling and impedance
    PS6000_COUPLING tbsCoupling;            
    if(coupling == Coupling::AC) {
        if(impedance == Impedance::R50) {
            tbsCoupling = PS6000_AC;
            impedance = Impedance::R1M; // in AC mode, only 1M impedance is available
        } else { // R1M
            tbsCoupling = PS6000_AC;
        }
    } else { // DC
        if(impedance == Impedance::R50) {
            tbsCoupling = PS6000_DC_50R;
        } else { // R1M
            tbsCoupling = PS6000_DC_1M;
        }
    }
    
    // channel range
    const int rangesAvailable[9] = {50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000};
    int selectedRangeIdx = 0;
    while(rangesAvailable[selectedRangeIdx] < rangemV && selectedRangeIdx < 8) selectedRangeIdx++;
    if(impedance == Impedance::R50 && selectedRangeIdx > 6) selectedRangeIdx = 6; // 10V and 20V not available in 50Ohm mode
    rangemV = rangesAvailable[selectedRangeIdx];            
    PS6000_RANGE tbsRange;            
    switch(selectedRangeIdx){
        case 0: tbsRange = PS6000_50MV; break;
        case 1: tbsRange = PS6000_100MV; break;
        case 2: tbsRange = PS6000_200MV; break;
        case 3: tbsRange = PS6000_500MV; break;
        case 4: tbsRange = PS6000_1V; break;
        case 5: tbsRange = PS6000_2V; break;
        case 7: tbsRange = PS6000_10V; break;
        case 8: tbsRange = PS6000_20V; break;
        default: tbsRange = PS6000_5V; break; // case 6
    }
    
    // analog offset
    float tbsOffset = offsetmV;
    tbsOffset /= 1000; // mV conversion to V
    
    // bandwidth limiter settings
    PS6000_BANDWIDTH_LIMITER tbsBwLimit;            
    switch(bwLimit){                
        case BandwidthLimiter::F20MHZ: tbsBwLimit = PS6000_BW_20MHZ; break;                
        case BandwidthLimiter::F25MHZ: tbsBwLimit = PS6000_BW_25MHZ; break;
        default:                       tbsBwLimit = PS6000_BW_FULL;  break; // case BandwidthLimiter::FULL
    }
    
    // finally set-up the picoscope
    PICO_STATUS status = ps6000SetChannel(m_handle, tbsChannel, tbsEnabled, tbsCoupling, tbsRange, tbsOffset, tbsBwLimit);
    if (status) throw RuntimeException("Failed to set channel parameters", status);
        
}

void Ps6000::setTrigger(int & sourceChannel, float & level, TriggerSlope & slope){
    
    if(!m_opened) throw RuntimeException("The oscilloscope needs to be properly initialized first");
    if(sourceChannel < 1 || sourceChannel > 4) throw InvalidInputException("Invalid channel");   
    
    PS6000_CHANNEL tbsChannel;
    switch(sourceChannel){
        case 2: tbsChannel = PS6000_CHANNEL_B; break;
        case 3: tbsChannel = PS6000_CHANNEL_C; break;
        case 4: tbsChannel = PS6000_CHANNEL_D; break;
        default: tbsChannel = PS6000_CHANNEL_A; sourceChannel = 1; break;
    }
    
    PICO_STATUS status = ps6000SetSimpleTrigger(m_handle, 
                                        true, 
                                        tbsChannel, 
                                        (((float)PS6000_MAX_VALUE) - ((float)PS6000_MIN_VALUE)) * level + ((float)PS6000_MIN_VALUE), 
                                        ( (slope == TriggerSlope::RISING) ? PS6000_RISING : ( (slope == TriggerSlope::FALLING) ? PS6000_FALLING : PS6000_RISING_OR_FALLING ) ), 
                                        0, 
                                        10000 );

    if(status) throw RuntimeException("Failed setting up trigger", status);
    
    return;
    
}

void Ps6000::unsetTrigger() {
    
    if(!m_opened) throw RuntimeException("The oscilloscope needs to be properly initialized first");
    
    PICO_STATUS status = ps6000SetSimpleTrigger(m_handle, 
                                        false,  // false => trigger disabled
                                        PS6000_CHANNEL_A, // dummy values
                                        0, 
                                        PS6000_RISING, 
                                        0, 
                                        0 );

    if(status) throw RuntimeException("Failed setting up trigger", status);
    
    return;
    
}

void Ps6000::setTiming(float & preTriggerRange, float & postTriggerRange, size_t & samples, size_t & captures){

    if(!m_opened) throw RuntimeException("The oscilloscope needs to be properly initialized first");
    
    if(captures < 1) captures = 1;
    PICO_STATUS status;
    
    uint32_t maxMemSamples;
    status = ps6000MemorySegments(m_handle, captures, &maxMemSamples); // if asking for more than one capture, segment the memory
    if(status) throw InvalidInputException("Failed to segment the oscilloscope memory. Maybe asking for too many captures at once?", status);
    
    status = ps6000SetNoOfCaptures(m_handle, captures); // set the number of captures requested at once
    if(status) throw InvalidInputException("Failed to do so many captures at once.", status);

    if(maxMemSamples < samples) throw InvalidInputException("Can't do that many captures with that much samples.");
    
    float requestedSampleInterval = (preTriggerRange + postTriggerRange) / ((float)samples);
    uint32_t foundTimebase;
    
    if(requestedSampleInterval < 6.4e-9) {
        
        foundTimebase = std::log2(requestedSampleInterval * 5000000000.0f);
        foundTimebase = (foundTimebase > 4) ? 0 : foundTimebase; // check for overflow caused by uint(float(log2))
        
    } else {
        
        foundTimebase = requestedSampleInterval * 156250000.0f + 4.0f;
        foundTimebase = (foundTimebase < 5) ? 5 : foundTimebase; // check for numerical errors around the border
        
    }                       
    
    uint32_t realTimebase = foundTimebase;
    float realTimebaseInterval = (realTimebase <= 4) ? std::pow(2, realTimebase) / 5000000000.0f : (realTimebase - 4) / 156250000.0f;                        
    uint32_t realSamples = ceil(preTriggerRange / realTimebaseInterval) + ceil(postTriggerRange / realTimebaseInterval);
    
    float offeredTimeInterval;
    uint32_t offeredSamples;      
    
    bool notSatisfied = true;
    
    do {                         
        
        notSatisfied = false;
        
        status = ps6000GetTimebase2(m_handle, realTimebase, realSamples, &offeredTimeInterval, 0, &offeredSamples, 0);            
        
        if(status || (offeredTimeInterval  / 1000000000.0f) != realTimebaseInterval || offeredSamples < realSamples) {
            
            notSatisfied = true;
            
            realTimebase += 1;
            realTimebaseInterval = (realTimebase <= 4) ? std::pow(2, realTimebase) / 5000000000.0f : (realTimebase - 4) / 156250000.0f;
            realSamples = ceil(preTriggerRange / realTimebaseInterval) + ceil(postTriggerRange / realTimebaseInterval);
            
            if(foundTimebase == realTimebase){
                throw RuntimeException("Failed setting timebase"); // tried all the 2^32 timebases, something is wrong dude
            }
                            
        }
        
        if(status == PICO_INVALID_HANDLE || status == PICO_INVALID_CHANNEL || status == PICO_SEGMENT_OUT_OF_RANGE || status == PICO_DRIVER_FUNCTION) 
            throw RuntimeException("Failed setting timebase", status);                
    
        
    } while (notSatisfied);                
    
    m_captures = captures;
    m_timebase = realTimebase;
    m_preTriggerSamples = ceil(preTriggerRange / realTimebaseInterval);
    m_postTriggerSamples = ceil(postTriggerRange / realTimebaseInterval);
    samples = m_preTriggerSamples + m_postTriggerSamples;
    preTriggerRange = m_preTriggerSamples * realTimebaseInterval;
    postTriggerRange = m_postTriggerSamples * realTimebaseInterval;
    
}

void Ps6000::run() {
    
    if(!m_opened) throw RuntimeException("The oscilloscope needs to be properly initialized first");
    
    PICO_STATUS status = ps6000RunBlock(m_handle, m_preTriggerSamples, m_postTriggerSamples, m_timebase, 0, NULL, 0, NULL, NULL);
    if(status) throw RuntimeException("Failed running the oscilloscope", status);
    
    // sleep 50ms ?
    
}

void Ps6000::stop() {
    
    if(!m_opened) throw RuntimeException("The oscilloscope needs to be properly initialized first");
    
    PICO_STATUS status = ps6000Stop(m_handle);
    if(status) throw RuntimeException("Failed stopping the oscilloscope", status);
    
}

size_t Ps6000::getCurrentSetup(size_t & samples, size_t & captures) {
    samples = m_preTriggerSamples + m_postTriggerSamples;
    captures = m_captures;
    return (m_preTriggerSamples + m_postTriggerSamples) * m_captures;
}

size_t Ps6000::getValues(int channel, int16_t * buffer, size_t len, size_t & samples, size_t & captures) {        
    
    if(!m_opened) throw RuntimeException("The oscilloscope needs to be properly initialized first");
    
    if(channel < 1 || channel > 4) throw InvalidInputException("Invalid channel");        
    
    int16_t ready = 0;
    while (!ready) {
        ps6000IsReady(m_handle, &ready);
    }
    
    if((m_preTriggerSamples+m_postTriggerSamples) * m_captures > len) throw RuntimeException("Receiving buffer too small");        
    
    PS6000_CHANNEL tbsChannel;
    switch(channel){
        case 2: tbsChannel = PS6000_CHANNEL_B; break;
        case 3: tbsChannel = PS6000_CHANNEL_C; break;
        case 4: tbsChannel = PS6000_CHANNEL_D; break;
        default: tbsChannel = PS6000_CHANNEL_A; break;
    }
    
    uint32_t samples_tmp = (m_preTriggerSamples + m_postTriggerSamples);
    
    if(m_captures > 1){
        
        PICO_STATUS status;
        int16_t over = 0;            
        
        for (uint32_t i = 0; i < m_captures; i++) {                
            status = ps6000SetDataBufferBulk(m_handle, tbsChannel, reinterpret_cast<short *>(buffer) + i * samples_tmp, samples_tmp, i, PS6000_RATIO_MODE_NONE);
            if (status) throw RuntimeException("Failed to set up receiving buffer");
        }
        
        samples = samples_tmp;
        samples_tmp *= m_captures;
        
        status = ps6000GetValuesBulk(m_handle, &samples_tmp, 0, m_captures - 1, 0, PS6000_RATIO_MODE_NONE, &over);          
        if (status) throw RuntimeException("Failed to receive the data");            
        
        captures = m_captures;
        
        return samples_tmp;            
        
    } else {
        
        PICO_STATUS status = ps6000SetDataBuffer(m_handle, tbsChannel, reinterpret_cast<short *>(buffer), m_preTriggerSamples + m_postTriggerSamples, PS6000_RATIO_MODE_NONE);
        if (status) throw RuntimeException("Failed to set up receiving buffer");
        
        int16_t over = 0;
        
        status = ps6000GetValues(m_handle, 0, &samples_tmp, 1, PS6000_RATIO_MODE_NONE, 0, &over);
        if (status || samples_tmp != (m_preTriggerSamples + m_postTriggerSamples)) throw RuntimeException("Failed to receive the data");
        
        samples = samples_tmp;
        captures = 1;
        
        return samples;
        
    }
    
    return 0;
            
}

size_t Ps6000::getValues(int channel, PowerTraces<int16_t> & traces) {        
    
    if(!m_opened) throw RuntimeException("The oscilloscope needs to be properly initialized first");
    
    if(channel < 1 || channel > 4) throw InvalidInputException("Invalid channel");        
    
    traces.init(m_preTriggerSamples + m_postTriggerSamples, m_captures); //< alloc memory for aquisition
    
    size_t samples, captures;
    
    return (*this).getValues(channel, traces.data(), traces.size(), samples, captures);
    
    /*
    // wait for the aquisition to complete
    int16_t ready = 0;
    while (!ready) {
        ps6000IsReady(m_handle, &ready);
    }
        
    traces.init(m_preTriggerSamples + m_postTriggerSamples, m_captures); //< alloc memory for aquisition
    
    PS6000_CHANNEL tbsChannel;
    switch(channel){
        case 2: tbsChannel = PS6000_CHANNEL_B; break;
        case 3: tbsChannel = PS6000_CHANNEL_C; break;
        case 4: tbsChannel = PS6000_CHANNEL_D; break;
        default: tbsChannel = PS6000_CHANNEL_A; break;
    }
    
    uint32_t samplesTotal = (m_preTriggerSamples + m_postTriggerSamples) * m_captures;
    
    if(m_captures > 1){
        
        PICO_STATUS status;
        int16_t over = 0;            
        
        for (uint32_t i = 0; i < m_captures; i++) {                
            status = ps6000SetDataBufferBulk(m_handle, tbsChannel, &(traces(0, i)), traces.samplesPerTrace(), i, PS6000_RATIO_MODE_NONE);
            if (status) throw RuntimeException("Failed to set up receiving buffer", status);
        }        
        
        status = ps6000GetValuesBulk(m_handle, &samplesTotal, 0, m_captures - 1, 0, PS6000_RATIO_MODE_NONE, &over);          
        if (status) throw RuntimeException("Failed to receive the data", status);                    
        // TODO assert samplesTotal == traces.length() ??? podle manualu ne, ale podle vojty ano
        
        return samplesTotal;            
        
    } else {
        
        PICO_STATUS status = ps6000SetDataBuffer(m_handle, tbsChannel, traces.data(), traces.samplesPerTrace(), PS6000_RATIO_MODE_NONE);
        if (status) throw RuntimeException("Failed to set up receiving buffer", status);
        
        int16_t over = 0;
        
        status = ps6000GetValues(m_handle, 0, &samplesTotal, 1, PS6000_RATIO_MODE_NONE, 0, &over);
        if (status || samplesTotal != (m_preTriggerSamples + m_postTriggerSamples)) throw RuntimeException("Failed to receive the data");        
        
        return samplesTotal;
        
    }
    
    return 0;
    */        
}
