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
* \file oscilloscope.h
*
* \brief Oscilloscope plugin interface for use e.g. in meas
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef OSCILLOSCOPE_H
#define OSCILLOSCOPE_H

#include <QString>
#include "types_power.hpp"

/**
* \class Oscilloscope
* \ingroup SicakInterface
*
* \brief Oscilloscope QT plugin interface
*
*/
class Oscilloscope {        
    
public:
    
    /// Coupling of the oscilloscope channel
    enum class Coupling {
        AC,
        DC
    };
    
    /// Impedance of the oscilloscope channel
    enum class Impedance {
        R50,
        R1M
    };        
    
    /// Bandwidth limit of the oscilloscope channel
    enum class BandwidthLimiter {
        FULL,
        F20MHZ,
        F25MHZ
    };
    
    /// Edge slope of the oscilloscope trigger
    enum class TriggerSlope {
        RISING,
        FALLING,
        EITHER
    };
    
    virtual ~Oscilloscope() {}
    
    /// Plugin name
    virtual QString getPluginName() = 0;
    /// Plugin info
    virtual QString getPluginInfo() = 0;
    
    /// Initialize the plugin
    virtual void init(const char * filename) = 0;
    /// Deinitialize the plugin
    virtual void deInit() = 0;
    
    /// Query available devices
    virtual QString queryDevices() = 0;
    
    /// Set the channel settings, real values may be returned by driver to the references
    virtual void setChannel(int & channel, bool & enabled, Coupling & coupling, Impedance & impedance, int & rangemV, int & offsetmV, BandwidthLimiter & bwLimit) = 0;
    /// Set the trigger settings, real values may be returned by driver to the references
    virtual void setTrigger(int & sourceChannel, float & level, TriggerSlope & slope) = 0;
    /// Unset the trigger
    virtual void unsetTrigger() = 0;
    /// Set the timing settings, real values may be returned by driver to the references: e.g. only some oscilloscopes support more captures per run than 1
    virtual void setTiming(float & preTriggerRange, float & postTriggerRange, size_t & samples, size_t & captures) = 0;    
    /// Run the oscilloscope: wait for trigger when triggered, otherwise capture immediately
    virtual void run() = 0; 
    /// Stop the oscilloscope
    virtual void stop() = 0;
    
    /// Returns current samples/captures settings
    virtual size_t getCurrentSetup(size_t & samples, size_t & captures) = 0;
    
    /// Downloads values from the oscilloscope, first waits for the aquisition to complete
    virtual size_t getValues(int channel, PowerTraces<int16_t> & traces) = 0;
    /// Downloads values from the oscilloscope, first waits for the aquisition to complete
    virtual size_t getValues(int channel, int16_t * buffer, size_t len, size_t & samples, size_t & captures) = 0;
    
};        

#define Oscilloscope_iid "cz.cvut.fit.Sicak.OscilloscopeInterface/1.0"

Q_DECLARE_INTERFACE(Oscilloscope, Oscilloscope_iid)

#endif /* OSCILLOSCOPE_H */
