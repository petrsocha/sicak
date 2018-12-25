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
* \file meas.cpp
*
* \brief SICAK MEASurement text-based UI
*
*
* \author Petr Socha
* \version 1.0
*/


#include <QtGlobal>
#include <QCoreApplication>
#include <QTextStream>
#include <QPluginLoader>
#include <QDir>
#include <QCommandLineParser>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QTimer>

#include "measurement.h"
#include "oscilloscope.h"
#include "chardevice.h"
#include "configloader.hpp"
#include "meas.h"


Meas::CommandLineParseResult Meas::parseCommandLineParams(QCommandLineParser & parser) {
    
    QTextStream cout(stdout);
    QTextStream cerr(stderr);
    
    // Function options            
    
    const QCommandLineOption idOption({"I", "id"}, "The ID string will be used in output files' filenames. Default value is current datetime.", "string");
    parser.addOption(idOption);
        
    const QCommandLineOption queryOption({"Q", "query"}, "Query available traces and block data preprocessing plug-in modules (-M, -O, -C).");
    parser.addOption(queryOption);
    
    const QCommandLineOption measurementModuleOption({"M", "measurement-module"}, "ID of measurement scenario plug-in module to use.", "string");
    parser.addOption(measurementModuleOption);    
    
    const QCommandLineOption oscilloscopeModuleOption({"O", "oscilloscope-module"}, "ID of oscilloscope plug-in module to use.", "string");
    parser.addOption(oscilloscopeModuleOption);    
    
    const QCommandLineOption oscilloscopeDeviceOption({"R", "oscilloscope-device"}, "ID of oscilloscope device to use.", "string");
    parser.addOption(oscilloscopeDeviceOption);    
    
    const QCommandLineOption oscilloscopeConfigOption({"S", "oscilloscope-config"}, "Oscilloscope JSON configuration file.", "filepath");
    parser.addOption(oscilloscopeConfigOption);    
    
    const QCommandLineOption chardeviceModuleOption({"C", "chardevice-module"}, "ID of character device plug-in module to use.", "string");
    parser.addOption(chardeviceModuleOption);    
    
    const QCommandLineOption chardeviceDeviceOption({"D", "chardevice-device"}, "ID of character device to use.", "string");
    parser.addOption(chardeviceDeviceOption);
    
    const QCommandLineOption chardeviceConfigOption({"E", "chardevice-config"}, "Character device JSON configuration file.", "filepath");
    parser.addOption(chardeviceConfigOption);    
    
    const QCommandLineOption measurementsNOption({"n", "measurements"}, "Number of measurements to make.", "positive integer");
    parser.addOption(measurementsNOption);    
    
    
    const QCommandLineOption paramOption("param", "Optional measurement plug-in module parameters. Module specific option.", "param");
    parser.addOption(paramOption);
        
    parser.addPositionalArgument("config", "JSON configuration file(s).");
    
    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();        
    
    if (!parser.parse(QCoreApplication::arguments())) 
        return CommandLineError;      
    
    if (parser.isSet(versionOption)) return CommandLineVersionRequested;
    if (parser.isSet(helpOption)) return CommandLineHelpRequested;
    if (parser.isSet(queryOption)) return CommandLineQueryRequested;
    
    ConfigLoader cfg(parser);                
    
    m_id = (cfg.isSet(idOption)) ? (cfg.getParam(idOption)) : ((QDateTime::currentDateTime()).toString("ddMMyy-HHmmss"));
    m_param = (cfg.isSet(paramOption)) ? (cfg.getParam(paramOption)) : "";
    
    // Measurement module set
    if(cfg.isSet(measurementModuleOption)){
                 
        m_measurementModule = cfg.getParam(measurementModuleOption);
        
        if(!cfg.isSet(measurementsNOption)){
            cerr << "Number of measurements must be set: -n\n";
            return CommandLineError;
        }
        
        m_measurementsN = cfg.getParam(measurementsNOption).toLongLong();
        
        // Oscilloscope module set
        if(cfg.isSet(oscilloscopeModuleOption)){
            
            m_oscilloscopeModule = cfg.getParam(oscilloscopeModuleOption);
            
            if(cfg.isSet(oscilloscopeDeviceOption)){
                m_oscilloscopeDevice = cfg.getParam(oscilloscopeDeviceOption);
            }
            
            if(cfg.isSet(oscilloscopeConfigOption)){
                m_oscilloscopeConfig = cfg.getParam(oscilloscopeConfigOption);
            }
            
        }
        
        // Character device module set
        if(cfg.isSet(chardeviceModuleOption)){
            
            m_chardeviceModule = cfg.getParam(chardeviceModuleOption);
            
            if(cfg.isSet(chardeviceDeviceOption)){
                m_chardeviceDevice = cfg.getParam(chardeviceDeviceOption);
            }
            
            if(cfg.isSet(chardeviceConfigOption)){
                m_chardeviceConfig = cfg.getParam(chardeviceConfigOption);
            }
            
        }
        
        QTimer::singleShot(0, this, SLOT(run()));
        return CommandLineTaskPlanned;
    }
    
    
    return CommandLineNOP;
    
}

void Meas::queryPlugins(){
        
    QTextStream cout(stdout);
    bool found = false;
    
    QDir pluginsDir(QCoreApplication::instance()->applicationDirPath());
    pluginsDir.cd("plugins");                   
    pluginsDir.cd("measurement");
    found = false;
    cout << "\nFound following measurement scenario plug-ins:\n\n";        
    
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();
        if (plugin) {
            Measurement * measurementPlugin = qobject_cast<Measurement *>(plugin);
            if (measurementPlugin){
                found = true;
                QString pluginId = fileName;                    
                if(pluginId.startsWith("lib")) pluginId.remove(0, 3);
                if(pluginId.startsWith("sicak")) pluginId.remove(0, 5);
                if(pluginId.endsWith(".so")) pluginId.chop(3);
                if(pluginId.endsWith(".dll")) pluginId.chop(4);
                
                cout << "* Plug-in ID: '" << pluginId << "', name: '" << measurementPlugin->getPluginName() << "'\n";
                cout << "    Description: '" << measurementPlugin->getPluginInfo() << "'\n\n";
            }
        }
        pluginLoader.unload();
    }
    
    if(!found) cout << "* No measurement scenario plug-in found!\n";
                
    
    pluginsDir.cd(QCoreApplication::instance()->applicationDirPath());
    pluginsDir.cd("plugins");                   
    pluginsDir.cd("oscilloscope");
    found = false;
    cout << "\nFound following oscilloscope plug-ins:\n\n";        
    
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();
        if (plugin) {
            Oscilloscope * oscilloscopePlugin = qobject_cast<Oscilloscope *>(plugin);
            if (oscilloscopePlugin){
                found = true;
                QString pluginId = fileName;                    
                if(pluginId.startsWith("lib")) pluginId.remove(0, 3);
                if(pluginId.startsWith("sicak")) pluginId.remove(0, 5);
                if(pluginId.endsWith(".so")) pluginId.chop(3);
                if(pluginId.endsWith(".dll")) pluginId.chop(4);
                
                cout << "* Plug-in ID: '" << pluginId << "', name: '" << oscilloscopePlugin->getPluginName() << "'\n";
                cout << oscilloscopePlugin->queryDevices() << "\n";
            }
        }
        pluginLoader.unload();
    }
    
    if(!found) cout << "* No oscilloscope plug-in found!\n";
    
    
    pluginsDir.cd(QCoreApplication::instance()->applicationDirPath());
    pluginsDir.cd("plugins");                   
    pluginsDir.cd("chardevice");
    found = false;
    cout << "\nFound following character device plug-ins:\n\n";        
    
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();
        if (plugin) {
            CharDevice * chardevicePlugin = qobject_cast<CharDevice *>(plugin);
            if (chardevicePlugin){
                found = true;
                QString pluginId = fileName;                    
                if(pluginId.startsWith("lib")) pluginId.remove(0, 3);
                if(pluginId.startsWith("sicak")) pluginId.remove(0, 5);
                if(pluginId.endsWith(".so")) pluginId.chop(3);
                if(pluginId.endsWith(".dll")) pluginId.chop(4);
                
                cout << "* Plug-in ID: '" << pluginId << "', name: '" << chardevicePlugin->getPluginName() << "'\n";
                cout << chardevicePlugin->queryDevices() << "\n";
            }
        }
        pluginLoader.unload();
    }
    
    if(!found) cout << "* No character device plug-in found!\n";
    
    cout << "\n";    
    emit finished();
}    

bool Meas::loadMeasurementModule(){
    
    if(!m_measurementModule.size()) return false;
    
    QDir pluginsDir(QCoreApplication::instance()->applicationDirPath());
    pluginsDir.cd("plugins");           
        
    pluginsDir.cd("measurement");
    
    QString fileName = m_measurementModule;
    fileName.prepend("sicak");
    
    #if defined(Q_OS_WIN)
    fileName.append(".dll");
    #else
    fileName.prepend("lib");
    fileName.append(".so");
    #endif
    
    QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
    QObject *plugin = pluginLoader.instance();    
    if (plugin) {
        m_measurement = qobject_cast<Measurement *>(plugin);
        if (m_measurement){
            return true;
        }
    }
    
    m_measurement = nullptr;
    
    return false;
    
}

bool Meas::loadOscilloscopeModule(){
    
    if(!m_oscilloscopeModule.size()) return false;
    
    QDir pluginsDir(QCoreApplication::instance()->applicationDirPath());
    pluginsDir.cd("plugins");           
        
    pluginsDir.cd("oscilloscope");
    
    QString fileName = m_oscilloscopeModule;
    fileName.prepend("sicak");
    
    #if defined(Q_OS_WIN)
    fileName.append(".dll");
    #else
    fileName.prepend("lib");
    fileName.append(".so");
    #endif
    
    QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
    QObject *plugin = pluginLoader.instance();    
    if (plugin) {
        m_oscilloscope = qobject_cast<Oscilloscope *>(plugin);
        if (m_oscilloscope){
            return true;
        }
    }
    
    m_oscilloscope = nullptr;
    
    return false;
    
}

bool Meas::loadChardeviceModule(){
    
    if(!m_chardeviceModule.size()) return false;
    
    QDir pluginsDir(QCoreApplication::instance()->applicationDirPath());
    pluginsDir.cd("plugins");           
        
    pluginsDir.cd("chardevice");
    
    QString fileName = m_chardeviceModule;
    fileName.prepend("sicak");
    
    #if defined(Q_OS_WIN)
    fileName.append(".dll");
    #else
    fileName.prepend("lib");
    fileName.append(".so");
    #endif
    
    QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
    QObject *plugin = pluginLoader.instance();    
    if (plugin) {
        m_chardevice = qobject_cast<CharDevice *>(plugin);
        if (m_chardevice){
            return true;
        }
    }
    
    m_chardevice = nullptr;
    
    return false;
    
}

bool Meas::initConfigOscilloscope(){

    QTextStream cout(stdout);
    QTextStream cerr(stderr);
    
    // first initialize the oscilloscope
    try {
        QByteArray ba = m_oscilloscopeDevice.toLocal8Bit();
        m_oscilloscope->init(ba.data());
    } catch( std::exception & e) {
        cerr << "Failed to open and initialize the oscilloscope: " << e.what() << "\n";
        return false;
    }
    
    cout << "* Oscilloscope successfully opened: '" << m_oscilloscopeDevice << "'\n";
    cout.flush();
    
    // then configure the oscilloscope
    QFile file;
    file.setFileName(m_oscilloscopeConfig);
    if(m_oscilloscopeConfig.size() && file.open(QIODevice::ReadOnly | QIODevice::Text)){           
        
        cout << "* Oscilloscope configuration file found: '" << m_oscilloscopeConfig << "'\n";
        cout.flush();
        
        QString configStr = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(configStr.toUtf8());
        QJsonObject docObj = doc.object();
        
        // first set the channels
        foreach(const QString & key, docObj.keys()) {
            
            if(docObj.value(key).isObject() && key.startsWith("channel")){
                
                QString channelStr = key;
                channelStr.remove(0, 7);
                
                int channel = channelStr.toInt();
                
                if(channel <= 0){
                    cerr << "Invalid channel number: must be > 0\n";
                    return false;
                }
                
                bool enabled;
                Oscilloscope::Coupling coupling;
                Oscilloscope::Impedance impedance;
                int rangemV;
                int offsetmV;
                Oscilloscope::BandwidthLimiter bwLimit;
                
                QJsonObject channelSettings = docObj.value(key).toObject();
                
                if(channelSettings.contains("enabled") && channelSettings.value("enabled").isBool()){
                    enabled = channelSettings.value("enabled").toBool();
                } else {
                    cerr << "Channel settings: \"enabled\":bool property is required.\n";
                    return false;
                }

                if(channelSettings.contains("coupling") && channelSettings.value("coupling").isString()){
                    
                    QString couplingStr = channelSettings.value("coupling").toString();
                    if(!couplingStr.compare("AC")){
                        coupling = Oscilloscope::Coupling::AC;
                    } else if(!couplingStr.compare("DC")){
                        coupling = Oscilloscope::Coupling::DC;
                    } else {
                        cerr << "Channel settings: \"coupling\" has invalid value: AC or DC ?.\n";
                        return false;
                    }
                    
                } else {
                    cerr << "Channel settings: \"coupling\":string property is required.\n";
                    return false;
                }
                    
                if(channelSettings.contains("impedance") && channelSettings.value("impedance").isString()){
                    
                    QString impedanceStr = channelSettings.value("impedance").toString();
                    if(!impedanceStr.compare("50")){
                        impedance = Oscilloscope::Impedance::R50;
                    } else if(!impedanceStr.compare("1M")){
                        impedance = Oscilloscope::Impedance::R1M;
                    } else {
                        cerr << "Channel settings: \"impedance\" has invalid value: 50 or 1M ?.\n";
                        return false;
                    }
                    
                } else {
                    cerr << "Channel settings: \"impedance\":string property is required.\n";
                    return false;
                }
                
                if(channelSettings.contains("rangemV")){
                    rangemV = channelSettings.value("rangemV").toInt();
                } else {
                    cerr << "Channel settings: \"rangemV\":int property is required.\n";
                    return false;
                }

                if(channelSettings.contains("offsetmV")){
                    offsetmV = channelSettings.value("offsetmV").toInt();
                } else {
                    cerr << "Channel settings: \"offsetmV\":int property is required.\n";
                    return false;
                }
                
                if(channelSettings.contains("bwLimit") && channelSettings.value("bwLimit").isString()){
                    
                    QString bwLimitStr = channelSettings.value("bwLimit").toString();
                    if(!bwLimitStr.compare("FULL")){
                        bwLimit = Oscilloscope::BandwidthLimiter::FULL;
                    } else if(!bwLimitStr.compare("20MHz")){
                        bwLimit = Oscilloscope::BandwidthLimiter::F20MHZ;
                    } else if(!bwLimitStr.compare("25MHz")){
                        bwLimit = Oscilloscope::BandwidthLimiter::F25MHZ;
                    } else {
                        cerr << "Channel settings: \"bwLimit\" has invalid value: FULL or 20MHz or 25MHz ?.\n";
                        return false;
                    }
                    
                } else {
                    cerr << "Channel settings: \"bwLimit\":string property is required.\n";
                    return false;
                }
                
                cout << "    * Requesting oscilloscope channel settings:\n";
                cout << "        * Channel: '" << channel << "'\n";
                cout << "        * Enabled: '" << (enabled ? "true" : "false") << "'\n";
                cout << "        * Coupling: '" << ((coupling == Oscilloscope::Coupling::AC) ? "AC" : "DC") << "'\n";
                cout << "        * Impedance: '" << ((impedance == Oscilloscope::Impedance::R50) ? "50" : "1M") << "'\n";
                cout << "        * Range: -+'" << rangemV << "mV'\n";
                cout << "        * Offset: '" << offsetmV << "mV'\n";
                cout << "        * Bandwidth Limit: '" << ((bwLimit == Oscilloscope::BandwidthLimiter::FULL) ? "FULL" :
                                                           ((bwLimit == Oscilloscope::BandwidthLimiter::F20MHZ) ? "20MHz" : "25MHz")) << "'\n";
                cout.flush();
                                                           
                                                           
                try {
                    m_oscilloscope->setChannel(channel, enabled, coupling, impedance, rangemV, offsetmV, bwLimit);
                } catch(std::exception & e){
                    cerr << "Failed to set the channel: " << e.what() << "\n";
                    return false;
                }
                                                        
                cout << "    * Real oscilloscope channel settings (after setup):\n";
                cout << "        * Channel: '" << channel << "'\n";
                cout << "        * Enabled: '" << (enabled ? "true" : "false") << "'\n";
                cout << "        * Coupling: '" << ((coupling == Oscilloscope::Coupling::AC) ? "AC" : "DC") << "'\n";
                cout << "        * Impedance: '" << ((impedance == Oscilloscope::Impedance::R50) ? "50" : "1M") << "'\n";
                cout << "        * Range: -+'" << rangemV << "mV'\n";
                cout << "        * Offset: '" << offsetmV << "mV'\n";
                cout << "        * Bandwidth Limit: '" << ((bwLimit == Oscilloscope::BandwidthLimiter::FULL) ? "FULL" :
                                                           ((bwLimit == Oscilloscope::BandwidthLimiter::F20MHZ) ? "20MHz" : "25MHz")) << "'\n";
                cout.flush();
                
            }
            
        }
        
        // then set the trigger
        foreach(const QString & key, docObj.keys()) {
            
            if(docObj.value(key).isObject() && key.startsWith("trigger")){
                                
                QJsonObject triggerSettings = docObj.value(key).toObject();
                
                bool enabled;
                int channel;
                float level;
                Oscilloscope::TriggerSlope slope;
                
                if(triggerSettings.contains("enabled") && triggerSettings.value("enabled").isBool()){
                    
                    enabled = triggerSettings.value("enabled").toBool();
                    
                } else {
                    
                    cerr << "Trigger settings: \"enabled\":bool property is required.\n";
                    return false;
                }
                
                if(triggerSettings.contains("channel")){
                    
                    channel = triggerSettings.value("channel").toInt();
                    
                } else {
                    
                    cerr << "Trigger settings: \"channel\":int property is required.\n";
                    return false;
                }
                
                if(triggerSettings.contains("level") && triggerSettings.value("level").isDouble()){
                    
                    level = triggerSettings.value("level").toDouble();
                    
                } else {
                    
                    cerr << "Trigger settings: \"level\":double property is required.\n";
                    return false;
                }
                
                if(triggerSettings.contains("slope") && triggerSettings.value("slope").isString()){
                    
                    QString slopeStr = triggerSettings.value("slope").toString();
                    if(!slopeStr.compare("rising")){
                        slope = Oscilloscope::TriggerSlope::RISING;
                    } else if(!slopeStr.compare("falling")){
                        slope = Oscilloscope::TriggerSlope::FALLING;
                    } else if(!slopeStr.compare("either")){
                        slope = Oscilloscope::TriggerSlope::EITHER;
                    } else {
                        cerr << "Trigger settings: \"slope\" has invalid value: rising or falling or either ?\n";
                        return false;
                    }
                    
                } else {
                    cerr << "Trigger settings: \"slope\":string property is required.\n";
                    return false;
                }
                
                if(enabled) {                
                    
                    cout << "    * Requesting oscilloscope trigger settings:\n";
                    cout << "        * Enabled: '" << (enabled ? "true" : "false") << "'\n";
                    cout << "        * Source channel: '" << channel << "'\n";
                    cout << "        * Trigger level: '" << level << "' for channel range 0..1\n";
                    cout << "        * Edge slope: '" << ((slope == Oscilloscope::TriggerSlope::RISING) ? "rising" :
                                                            ((slope == Oscilloscope::TriggerSlope::FALLING) ? "falling" : "either")) << "'\n";
                    cout.flush();
                    
                    try {
                        
                        m_oscilloscope->setTrigger(channel, level, slope);
                        
                    } catch(std::exception & e) {
                        cerr << "Failed to set the trigger: " << e.what() << "\n";
                        return false;
                    }
                    
                    cout << "    * Real oscilloscope trigger settings (after setup):\n";
                    cout << "        * Enabled: '" << (enabled ? "true" : "false") << "'\n";
                    cout << "        * Source channel: '" << channel << "'\n";
                    cout << "        * Trigger level: '" << level << "' for channel range 0..1\n";
                    cout << "        * Edge slope: '" << ((slope == Oscilloscope::TriggerSlope::RISING) ? "rising" :
                                                            ((slope == Oscilloscope::TriggerSlope::FALLING) ? "falling" : "either")) << "'\n";
                    cout.flush();
                    
                
                } else {
                    
                    cout << "    * Requesting oscilloscope trigger settings:\n";
                    cout << "        * Enabled: '" << (enabled ? "true" : "false") << "'\n";
                    cout.flush();
                    
                    try {
                        
                        m_oscilloscope->unsetTrigger();
                        
                    } catch (std::exception & e){
                        cerr << "Failed to unset the trigger: " << e.what() << "\n";
                        return false;
                    }
                    
                    cout << "    * Real oscilloscope trigger settings (after setup):\n";
                    cout << "        * Enabled: '" << (enabled ? "true" : "false") << "'\n";
                    cout.flush();
                                        
                }
                
            }
            
        }
        
        // third pass finally sets the timing
        foreach(const QString & key, docObj.keys()) {
            
            if(docObj.value(key).isObject() && key.startsWith("timing")){
                
                QJsonObject timingSettings = docObj.value(key).toObject();
                
                float preTriggerRange;
                float postTriggerRange;
                size_t samples; 
                size_t captures;
                
                if(timingSettings.contains("preTriggerRange") && timingSettings.value("preTriggerRange").isDouble()){
                    
                    preTriggerRange = timingSettings.value("preTriggerRange").toDouble();
                    
                } else {
                    
                    cerr << "Timing settings: \"preTriggerRange\":double property is required.\n";
                    return false;
                }
                
                if(timingSettings.contains("postTriggerRange") && timingSettings.value("postTriggerRange").isDouble()){
                    
                    postTriggerRange = timingSettings.value("postTriggerRange").toDouble();
                    
                } else {
                    
                    cerr << "Timing settings: \"postTriggerRange\":double property is required.\n";
                    return false;
                }
                
                if(timingSettings.contains("samples")){
                    
                    samples = timingSettings.value("samples").toInt();
                    
                } else {
                    
                    cerr << "Timing settings: \"samples\":int property is required.\n";
                    return false;
                }
                
                if(timingSettings.contains("captures")){
                    
                    captures = timingSettings.value("captures").toInt();
                    
                } else {
                    
                    cerr << "Timing settings: \"captures\":int property is required.\n";
                    return false;
                }
                
                cout << "    * Requesting oscilloscope timing settings:\n";
                cout << "        * Pre-trigger time range: '" << preTriggerRange << "s'\n";
                cout << "        * Post-trigger time range: '" << postTriggerRange << "s'\n";
                cout << "        * Samples: '" << samples << "'\n";
                cout << "        * Captures per run: '" << captures << "'\n";
                cout.flush();
                
                try {
                        
                    m_oscilloscope->setTiming(preTriggerRange, postTriggerRange, samples, captures);
                    
                } catch (std::exception & e){
                    cerr << "Failed to set the timing: " << e.what() << "\n";
                    return false;
                }
                
                cout << "    * Real oscilloscope timing settings (after setup):\n";
                cout << "        * Pre-trigger time range: '" << preTriggerRange << "s'\n";
                cout << "        * Post-trigger time range: '" << postTriggerRange << "s'\n";
                cout << "        * Samples: '" << samples << "'\n";
                cout << "        * Captures per run: '" << captures << "'\n";
                cout.flush();
                
            }
            
        }
        
    } else if(m_oscilloscopeConfig.size()) {
        cerr << "Failed to open specified oscilloscope configuration file.\n";
        return false;
    } else {
        cout << "* No oscilloscope configuration file found.\n";
    }
        
    return true;
}

bool Meas::initConfigChardevice(){
    
    QTextStream cout(stdout);
    QTextStream cerr(stderr);
    
    int baudrate = 9600;
    int parity = 0;
    int stopBits = 1;
    int timeoutms = 5000;
    
    QFile file;
    file.setFileName(m_chardeviceConfig);
    if(m_chardeviceConfig.size() && file.open(QIODevice::ReadOnly | QIODevice::Text)){
         
        cout << "* Character device configuration file found: " << m_chardeviceConfig << "\n";
        cout.flush();
        
        QString configStr = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(configStr.toUtf8());
        QJsonObject docObj = doc.object();
        
        if(docObj.contains("baudrate")){
            baudrate = docObj.value("baudrate").toInt();
        }

        if(docObj.contains("parity")){
            parity = docObj.value("parity").toInt();
        }
        
        if(docObj.contains("stopbits")){
            stopBits = docObj.value("stopbits").toInt();
        }
        
        if(docObj.contains("timeoutms")){
            stopBits = docObj.value("timeoutms").toInt();
        }
         
    } else if(m_chardeviceConfig.size()){
        cerr << "Failed to open specified character device configuration file.\n";
        return false;
    } else {
        cout << "* No character device configuration file found.\n";
    }
    
    cout.flush();
    
    try {
    
        QByteArray ba = m_chardeviceDevice.toLocal8Bit();
        m_chardevice->init(ba.data(), baudrate, parity, stopBits);
        
    } catch(std::exception & e){
        cerr << "Failed to open the specified character device: " << e.what() << "\n";        
        return false;
    }
    
    cout << "* Character device successfully opened: '" << m_chardeviceDevice << "'\n";
    cout << "    * Using following settings:\n";
    cout << "        * Baudrate: '" << baudrate << "'\n";
    cout << "        * Parity: '" << ( (!parity) ? "no parity" : ( (parity%2) ? "even" : "odd" ) ) << "'\n";
    cout << "        * Stop bits: '" << ( (stopBits == 2) ? "two" : "one" ) << "'\n";
    cout.flush();
    
    try {
    
        m_chardevice->setTimeout(timeoutms);
        
    } catch(std::exception & e){
        cerr << "Failed to set the character device timeout: " << e.what() << "\n";        
        return false;
    }
    
    cout << "* Character device timeout set: '" << timeoutms << "ms'\n";
    cout.flush();
    
    return true;
}

void Meas::run(){

    QTextStream cout(stdout);
    QTextStream cerr(stderr);
    QByteArray ba;
    cout << "* " << QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss") << " Starting...\n";
    cout.flush();
    //cout << "* Running the measurement...\n";
    //cout.flush();
 
    if(!loadMeasurementModule()){
        cerr << "Failed to load the measurement scenario module plug-in.\n";
        emit finished();
        return;
    }
    
    try {
        
        ba = m_param.toLocal8Bit();
        m_measurement->init(ba.data());
        
    } catch (std::exception & e){
        cerr << "Failed to initialize the measurement scenario module plug-in.\n";
        emit finished();
        return;
    }
    
    cout << "* Measurement scenario module loaded: '" << m_measurement->getPluginName() << "'\n";
    cout.flush();
    
    if(loadOscilloscopeModule()){
    
        cout << "* Oscilloscope module loaded: '" << m_oscilloscope->getPluginName() << "'\n";
        cout.flush();
        
        if(!initConfigOscilloscope()){
            cerr << "Failed to initialize and configure the oscilloscope.\n";
            emit finished();
            return;
        }
        
    } else if(m_oscilloscopeModule.size()) {
        cerr << "Failed to load the specified oscilloscope module.\n";
        emit finished();
        return;
    } else {
        cout << "* No oscilloscope module specified/loaded.\n";
    }
    
    cout.flush();
    
    if(loadChardeviceModule()){
        
        cout << "* Character device module loaded: '" << m_chardevice->getPluginName() << "'\n";
        cout.flush();
        
        if(!initConfigChardevice()){
            cerr << "Failed to initialize and configure the character device.\n";
            emit finished();
            return;
        }
        
    } else if(m_chardeviceModule.size()) {
        cerr << "Failed to load the specified character device module!\n";
        emit finished();
        return;
    } else {
        cout << "* No character device module specified/loaded.\n";
    }
        
    // Oscilloscope, if set, is now initialized and configured,
    // character device too,
    // ...
    // so run the measurement scenario now
    cout << "* Launching " << m_measurementsN << " measurements...\n";
    cout.flush();
    try {
        
        QByteArray ba = m_id.toLocal8Bit();
        m_measurement->run(ba.data(), m_measurementsN, m_oscilloscope, m_chardevice);
        
    } catch(std::exception & e) {
        cerr << "\nFailed to run the measurement scenario: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    
    // deInit modules
    try {
        
        if(m_oscilloscope != nullptr) m_oscilloscope->deInit();
        if(m_chardevice != nullptr) m_chardevice->deInit();
        m_measurement->deInit();        
        
    } catch(std::exception & e){
        cerr << "Failed to properly deinitialize the plug-in modules: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    cout << "* " << QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss") << " Finished.\n";    
    
    emit finished();
    return;
}
