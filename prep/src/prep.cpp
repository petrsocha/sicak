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
* \file prep.cpp
*
* \brief SICAK PREProcessing text-based UI
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

#include "configloader.hpp"
#include "global_calls.hpp"
#include "filehandling.hpp"
#include "prep.h"


Prep::CommandLineParseResult Prep::parseCommandLineParams(QCommandLineParser & parser) {
    
    QTextStream cout(stdout);
    QTextStream cerr(stderr);
    
    // Function options            
    
    const QCommandLineOption idOption({"I", "id"}, "The ID string will be used in output files' filenames. Default value is current datetime.", "string");
    parser.addOption(idOption);
        
    const QCommandLineOption queryOption({"Q", "query"}, "Query available traces and block data preprocessing plug-in modules (-T, -B).");
    parser.addOption(queryOption);
    
    const QCommandLineOption tracesModuleOption({"T", "traces-preprocess-module"}, "ID of traces preprocessing plug-in module to use. Select either -T or -B.", "string");
    parser.addOption(tracesModuleOption);    
    
    const QCommandLineOption blockModuleOption({"B", "block-preprocess-module"}, "ID of block data preprocessing plug-in module to use. Select either -T or -B.", "string");
    parser.addOption(blockModuleOption);    
    
    
    const QCommandLineOption tracesOption({"t", "traces"}, "File containing -n traces, each of which containing -s samples (int16).", "filepath");
    parser.addOption(tracesOption);    
    
    const QCommandLineOption tracesNOption({"n", "traces-count"}, "Number of power traces in -t file.", "positive integer");
    parser.addOption(tracesNOption);    
    
    const QCommandLineOption samplesOption({"s", "samples-per-trace"}, "Number of samples per trace.", "positive integer");
    parser.addOption(samplesOption);    
    
    
    const QCommandLineOption blocksOption({"b", "blocks"}, "File containing -m blocks of data, each of which -k bytes long.", "filepath");
    parser.addOption(blocksOption);    
    
    const QCommandLineOption blocksMOption({"m", "blocks-count"}, "Number of blocks of data in -b file.", "positive integer");
    parser.addOption(blocksMOption);    
    
    const QCommandLineOption blocksKOption({"k", "blocks-length"}, "Length of data block in -b file, in bytes.", "positive integer");
    parser.addOption(blocksKOption);            
    
    
    const QCommandLineOption paramOption("param", "Optional plug-in module parameters. Module specific option.", "param");
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
            
    if(cfg.isSet(tracesModuleOption) && cfg.isSet(blockModuleOption)){
        cerr << "Only one of the following options is allowed: -T, -B\n";
        return CommandLineError;
        
    } else if(cfg.isSet(tracesModuleOption)) {
        // preprocessing traces
        
        m_tracesModule = cfg.getParam(tracesModuleOption);
        
        if( !cfg.isSet(tracesOption) || 
            !cfg.isSet(tracesNOption) ||
            !cfg.isSet(samplesOption)
        ){
            
                cerr << "Some of traces preprocessing parameters missing: -t, -n, -s are required\n";
                return CommandLineError;
            }
        
        m_traces = cfg.getParam(tracesOption);
        m_tracesN = cfg.getParam(tracesNOption).toLongLong();
        m_samples = cfg.getParam(samplesOption).toLongLong();
        
        QTimer::singleShot(0, this, SLOT(preprocessTraces()));
            return CommandLineTaskPlanned;
            
        
    } else if(cfg.isSet(blockModuleOption)) {
        // preprocessing block data
        
        m_blockModule = cfg.getParam(blockModuleOption);
        
        if( !cfg.isSet(blocksOption) || 
            !cfg.isSet(blocksMOption) ||
            !cfg.isSet(blocksKOption)
        ){
            
                cerr << "Some of block data preprocessing parameters missing: -b, -m, -k are required\n";
                return CommandLineError;
            }
        
        m_blocks = cfg.getParam(blocksOption);
        m_blocksM = cfg.getParam(blocksMOption).toLongLong();
        m_blocksLen = cfg.getParam(blocksKOption).toLongLong();
        
        QTimer::singleShot(0, this, SLOT(preprocessBlocks()));
            return CommandLineTaskPlanned;
        
    }
        
    return CommandLineNOP;
    
}

void Prep::queryPlugins(){
        
    QTextStream cout(stdout);
    bool found = false;
    
    QDir pluginsDir(QCoreApplication::instance()->applicationDirPath());
    pluginsDir.cd("plugins");           
        
    pluginsDir.cd("tracesprocess");
    
    cout << "\nFound following traces preprocessing plug-ins:\n\n";        
    
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();
        if (plugin) {
            TracesProcess * tracesProcessPlugin = qobject_cast<TracesProcess *>(plugin);
            if (tracesProcessPlugin){
                found = true;
                QString pluginId = fileName;                    
                if(pluginId.startsWith("lib")) pluginId.remove(0, 3);
                if(pluginId.startsWith("sicak")) pluginId.remove(0, 5);
                if(pluginId.endsWith(".so")) pluginId.chop(3);
                if(pluginId.endsWith(".dll")) pluginId.chop(4);
                
                cout << "* Plug-in ID: '" << pluginId << "', name: '" << tracesProcessPlugin->getPluginName() << "'\n";
            }
        }
        pluginLoader.unload();
    }
    
    if(!found) cout << "* No traces preprocessing plug-in found!\n";
                
    pluginsDir.cd(QCoreApplication::instance()->applicationDirPath());
    pluginsDir.cd("plugins");
    pluginsDir.cd("blockprocess");
    found = false;
    cout << "\nFound following block data preprocessing plug-ins:\n\n";        
    
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();
        if (plugin) {
            BlockProcess * blockProcessPlugin = qobject_cast<BlockProcess *>(plugin);
            if (blockProcessPlugin){                
                found = true;
                QString pluginId = fileName;                    
                if(pluginId.startsWith("lib")) pluginId.remove(0, 3);
                if(pluginId.startsWith("sicak")) pluginId.remove(0, 5);
                if(pluginId.endsWith(".so")) pluginId.chop(3);
                if(pluginId.endsWith(".dll")) pluginId.chop(4);
                
                cout << "* Plug-in ID: '" << pluginId << "', name: '" << blockProcessPlugin->getPluginName() << "'\n";
            }
        }
        pluginLoader.unload();
    }
    
    if(!found) cout << "* No block data preprocessing plug-in found!\n";
    cout << "\n";
    
    emit finished();
}    

bool Prep::loadTracesModule() {
    
    QDir pluginsDir(QCoreApplication::instance()->applicationDirPath());
    pluginsDir.cd("plugins");           
        
    pluginsDir.cd("tracesprocess");
    
    QString fileName = m_tracesModule;
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
        m_tracesEngine = qobject_cast<TracesProcess *>(plugin);
        if (m_tracesEngine){
            return true;
        }
    }
    
    return false;
    
}

bool Prep::loadBlocksModule() {
    
    QDir pluginsDir(QCoreApplication::instance()->applicationDirPath());
    pluginsDir.cd("plugins");           
        
    pluginsDir.cd("blockprocess");
    
    QString fileName = m_blockModule;
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
        m_blockEngine = qobject_cast<BlockProcess *>(plugin);
        if (m_blockEngine){
            return true;
        }
    }
    
    return false;
    
}

void Prep::preprocessTraces(){
    
    QTextStream cout(stdout);
    QTextStream cerr(stderr);
    cout << "Preprocessing power traces...\n";
    cout.flush();
    
    if(!loadTracesModule()){
        cerr << "Failed to load the specified plug-in module\n";
        emit finished();
        return;
    }
    
    // Init
    try {
                
        QByteArray ba = m_param.toLocal8Bit();
        m_tracesEngine->init(ba.data());
        
    } catch(std::exception & e){
        cerr << "Failed to initialize the plug-in module: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    PowerTraces<int16_t> powerTraces;
    
    // Alloc memory
    try {
            
        powerTraces.init(m_samples, m_tracesN);
        
    } catch (std::exception & e) {
        cerr << "Failed to allocate power traces memory: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    std::fstream tracesFile;
    QByteArray ba;
    
    // Open file
    try {
        
        ba = m_traces.toLocal8Bit();    
        tracesFile = openInFile(ba.data());
        
    } catch (std::exception & e) {
        cerr << "Failed to open power traces file: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Load data
    try {
        
        fillArrayFromFile(tracesFile, powerTraces);                
        closeFile(tracesFile);        
        
    } catch (std::exception & e) {
        cerr << "Failed to read power traces from file: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Run the processing
    try {
        
        ba = m_id.toLocal8Bit();
        m_tracesEngine->processTraces(powerTraces, ba.data());
        
    } catch (std::exception & e) {
        cerr << "Failed to process the power traces: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // deInit
    try {

        m_tracesEngine->deInit();
        
    } catch(std::exception & e){
        cerr << "Failed to properly deinitialize the plug-in module: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    emit finished();
}

void Prep::preprocessBlocks(){
    
    QTextStream cout(stdout);
    QTextStream cerr(stderr);
    cout << "Preprocessing block data...\n";
    cout.flush();
    
    if(!loadBlocksModule()){
        cerr << "Failed to load the specified plug-in module\n";
        emit finished();
        return;
    }
    
    // Init
    try {
                
        QByteArray ba = m_param.toLocal8Bit();
        m_blockEngine->init(ba.data());
        
    } catch(std::exception & e){
        cerr << "Failed to initialize the plug-in module: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    Matrix<uint8_t> blockData;
    
    // Alloc  memory
    try {
            
        blockData.init(m_blocksLen, m_blocksM);
        
    } catch (std::exception & e) {
        cerr << "Failed to allocate block data memory: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    std::fstream dataFile;
    QByteArray ba;
    
    // Open  file
    try {
        
        ba = m_blocks.toLocal8Bit();    
        dataFile = openInFile(ba.data());
        
    } catch (std::exception & e) {
        cerr << "Failed to open block data file: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Load data
    try {
        
        fillArrayFromFile(dataFile, blockData);                
        closeFile(dataFile);        
        
    } catch (std::exception & e) {
        cerr << "Failed to read block data from file: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Run the processing
    try {
        
        ba = m_id.toLocal8Bit();
        m_blockEngine->processBlockData(blockData, ba.data());
        
    } catch (std::exception & e) {
        cerr << "Failed to process the power traces: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // deInit
    try {

        m_blockEngine->deInit();
        
    } catch(std::exception & e){
        cerr << "Failed to properly close the files or deinitialize the plug-in module: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    emit finished();
}

