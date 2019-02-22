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
* \file keysight3000.h
*
* \brief SICAK oscilloscope plugin: Keysight 3000 series (formerly agilent), using SCPIDevice
*
*
* \author Petr Socha
* \version 1.1
*/

#ifndef KEYSIGHT3000_H
#define KEYSIGHT3000_H 

#include <QObject>
#include <QtPlugin>
#include <string>
#include <cstdlib>
#include "oscilloscope.h"
#include "scpidevice.h"
#include "exceptions.hpp"

/**
* \class Keysight3000
* \ingroup Oscilloscope
*
* \brief Oscilloscope plugin for Keysight 3000 series oscilloscope, using ScpiDevice for communication
*
*/   
class Keysight3000 : public QObject, Oscilloscope {
    
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cz.cvut.fit.Sicak.OscilloscopeInterface/1.0" FILE "keysight3000.json")
    Q_INTERFACES(Oscilloscope)
                
public:        
    
    Keysight3000();    
    virtual ~Keysight3000() override;
    
    virtual QString getPluginName() override;
    virtual QString getPluginInfo() override;
    
    /// Initialize the oscilloscope, use filename=/dev/usbtmc0 on Linux or filename=VISAADDRESS on Windows
    virtual void init(const char * filename) override;
    virtual void deInit() override;
    
    virtual QString queryDevices() override;
    
    virtual void setChannel(int & channel, bool & enabled, Coupling & coupling, Impedance & impedance, int & rangemV, int & offsetmV, BandwidthLimiter & bwLimit) override;        
    virtual void setTrigger(int & sourceChannel, float & level, TriggerSlope & slope) override;        
    virtual void unsetTrigger() override;
    /// Set the timing settings, real values may be returned by driver to the references: e.g. only 1 capture per run is supported by this driver
    virtual void setTiming(float & preTriggerRange, float & postTriggerRange, size_t & samples, size_t & captures) override;
    virtual void run() override;
    virtual void stop() override;
    
    virtual size_t getCurrentSetup(size_t & samples, size_t & captures) override;
    virtual size_t getValues(int channel, PowerTraces<int16_t> & traces) override;
    virtual size_t getValues(int channel, int16_t * buffer, size_t len, size_t & samples, size_t & captures) override;
    
protected:
    ScpiDevice m_handle;
    size_t m_samples;
    bool m_triggered;
    bool m_opened;        
    
    size_t dummyMeasurement();
    
};

#endif /* KEYSIGHT3000_H */
