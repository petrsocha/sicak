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
* \file ps6000.h
*
* \brief SICAK oscilloscope plugin: PicoScope 6000 series
*
*
* \author Petr Socha
* \version 1.1
*/

#ifndef PS6000_H
#define PS6000_H

#include <QObject>
#include <QtPlugin>
#include <cmath>
#include <ps6000Api.h>
#include "oscilloscope.h"
#include "exceptions.hpp"

/**
* \class Ps6000
* \ingroup Oscilloscope
*
* \brief Oscilloscope plugin for PicoScope 6000, using official driver for communication
*
*/  
class Ps6000 : public QObject, Oscilloscope {
   
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cz.cvut.fit.Sicak.OscilloscopeInterface/1.0" FILE "ps6000.json")
    Q_INTERFACES(Oscilloscope)        
    
public:        
    
    Ps6000();    
    virtual ~Ps6000() override;
    
    virtual QString getPluginName() override;
    virtual QString getPluginInfo() override;
    
    virtual void init(const char * filename) override;
    virtual void deInit() override;
    
    virtual QString queryDevices() override;
    
    virtual void setChannel(int & channel, bool & enabled, Coupling & coupling, Impedance & impedance, int & rangemV, int & offsetmV, BandwidthLimiter & bwLimit) override;        
    virtual void setTrigger(int & sourceChannel, float & level, TriggerSlope & slope) override;           
    virtual void unsetTrigger() override;
    /// Set the timing settings, real values may be returned by driver to the references
    virtual void setTiming(float & preTriggerRange, float & postTriggerRange, size_t & samples, size_t & captures) override;
    virtual void run() override;
    virtual void stop() override;
    
    virtual size_t getCurrentSetup(size_t & samples, size_t & captures) override;
    virtual size_t getValues(int channel, PowerTraces<int16_t> & traces) override;
    virtual size_t getValues(int channel, int16_t * buffer, size_t len, size_t & samples, size_t & captures) override;

protected:
    int16_t m_handle;
    uint32_t m_preTriggerSamples;
    uint32_t m_postTriggerSamples;
    uint32_t m_timebase;
    float m_timebaseInterval;
    uint32_t m_captures;
    bool m_opened;
    
};

#endif /* PS6000_H */
