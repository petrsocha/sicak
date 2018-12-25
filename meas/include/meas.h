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
* \file meas.h
*
* \brief SICAK MEASurement text-based UI
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef MEAS_H
#define MEAS_H

#include <QObject>
#include <QCommandLineParser>
#include "measurement.h"
#include "oscilloscope.h"
#include "chardevice.h"

/**
* \class Meas
* \ingroup Sicak
*
* \brief Class providing text-based UI front-end to Measurement, Oscilloscope and CharDevice plug-in modules
*
*/
class Meas: public QObject {
  
Q_OBJECT

public:
    
    enum CommandLineParseResult {
        CommandLineTaskPlanned,
        CommandLineNOP,
        CommandLineError,
        CommandLineVersionRequested,
        CommandLineHelpRequested,
        CommandLineQueryRequested
    };
    
    Meas(QObject *parent = 0) : QObject(parent), m_id(""), m_param(""), m_measurementModule(""), m_measurementsN(0), m_oscilloscopeModule(""), m_oscilloscopeDevice(""), m_oscilloscopeConfig(""), m_chardeviceModule(""), m_chardeviceDevice(""), m_chardeviceConfig(""), m_measurement(nullptr), m_oscilloscope(nullptr), m_chardevice(nullptr) {}
    
    /// Parse parameters from the command line and configuration files and plan tasks for event loop accordingly
    CommandLineParseResult parseCommandLineParams(QCommandLineParser & parser);    

protected:
    
    /// Load the specified measurement scenario module
    bool loadMeasurementModule();
    /// Load the specified oscilloscope module
    bool loadOscilloscopeModule();
    /// Load the specified character device module
    bool loadChardeviceModule();
    
    /// Initialize and configure loaded oscilloscope from given config file
    bool initConfigOscilloscope();
    /// Initialize and configure loaded character device module from given config file
    bool initConfigChardevice();
    
    // parameters from command line
    QString m_id;
    QString m_param;
    
    QString m_measurementModule;
    size_t m_measurementsN;
    
    QString m_oscilloscopeModule;        
    QString m_oscilloscopeDevice;
    QString m_oscilloscopeConfig;
    
    QString m_chardeviceModule;
    QString m_chardeviceDevice;
    QString m_chardeviceConfig;
    
    // Modules
    Measurement * m_measurement;
    Oscilloscope * m_oscilloscope;
    CharDevice * m_chardevice;
    
public slots:
    
    /// Query and print available plugins
    void queryPlugins();
    /// Load and configure all the modules and run the measurement scenario
    void run();
    
    
signals:
    
    void finished();
    
};

#endif /* MEAS_H */

