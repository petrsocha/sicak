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
* \file main.cpp
*
* \brief SICAK STatistical ANalysis text-based UI
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
#include "stan.h"


Stan::CommandLineParseResult Stan::parseCommandLineParams(QCommandLineParser & parser) {
    
    QTextStream cout(stdout);
    QTextStream cerr(stderr);
    
    // Function options            
    
    const QCommandLineOption idOption({"I", "id"}, "The ID string will be used in output files' filenames. Default value is current datetime.", "string");
    parser.addOption(idOption);
    
    const QCommandLineOption queryOption({"Q", "query"}, "Query available CPA and t-test plug-in modules (-C, -T), platforms (-P) and devices (-D).");
    parser.addOption(queryOption);
    
    const QCommandLineOption cpaModuleOption({"C", "cpa-module"}, "ID of a CPA plug-in module to launch. Select either -C or -T.", "string");
    parser.addOption(cpaModuleOption);    
    
    const QCommandLineOption ttestModuleOption({"T", "ttest-module"}, "ID of a t-test plug-in module to launch. Select either -C or -T.", "string");
    parser.addOption(ttestModuleOption);    
    
    const QCommandLineOption platformOption({"P", "platform"}, "Platform from which to choose a device (-D). Default is 0.", "number", "0");
    parser.addOption(platformOption);
    
    const QCommandLineOption deviceOption({"D", "device"}, "Device from a platform (-P) to run computation on. Default is 0.", "number", "0");
    parser.addOption(deviceOption);
    
    const QCommandLineOption functionOption({"F", "function"}, "Select a function: 'create' a new context from traces/predictions, 'merge' existing contexts A,B or 'finalize' existing context A.", "create|merge|finalize");
    parser.addOption(functionOption);
    
    // Computation options
    
    const QCommandLineOption randTracesOption({"r", "random-traces"}, "File containing -n random data traces, each of which containing -s samples (int16).", "filepath");
    parser.addOption(randTracesOption);
    
    const QCommandLineOption randTracesNOption({"n", "random-traces-count"}, "Number of random data power traces in -r file.", "positive integer");
    parser.addOption(randTracesNOption);    
    
    
    const QCommandLineOption constTracesOption({"c", "constant-traces"}, "File containing -m constant data traces, each of which containing -s samples (int16).", "filepath");
    parser.addOption(constTracesOption);
    
    const QCommandLineOption constTracesMOption({"m", "constant-traces-count"}, "Number of constant data power traces in -c file.", "positive integer");
    parser.addOption(constTracesMOption);    
    
    
    const QCommandLineOption samplesOption({"s", "samples-per-trace"}, "Number of samples per trace.", "positive integer");
    parser.addOption(samplesOption);    
    
    
    const QCommandLineOption predictionsOption({"p", "predictions"}, "File containing -q power prediction sets, each of which containing -k power predictions (uint8) for every random trace in -r file. ", "filepath");
    parser.addOption(predictionsOption);
    
    const QCommandLineOption predictionsQOption({"q", "prediction-sets-count", "contexts-count"}, "Number of power prediction sets/number of contexts. E.g. attacking AES-128 key, this value would be 16.", "positive integer");
    parser.addOption(predictionsQOption);    

    const QCommandLineOption predictionsKOption({"k", "prediction-candidates-count"}, "Number of power predictions for each power trace in -p file. E.g. attacking AES-128 key, this value would be 256.", "positive integer");
    parser.addOption(predictionsKOption);        
    
    
    const QCommandLineOption contextAOption({"a", "context-a"}, "Context file A, for use in Finalize or Merge functions.", "filepath");
    parser.addOption(contextAOption);
    
    const QCommandLineOption contextBOption({"b", "context-b"}, "Context file B, for use in Merge function.", "filepath");
    parser.addOption(contextBOption);
    
    
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
    
    // straight thru params
    m_id = (cfg.isSet(idOption)) ? (cfg.getParam(idOption)) : ((QDateTime::currentDateTime()).toString("ddMMyy-HHmmss"));
    m_platform = (cfg.isSet(platformOption)) ? (cfg.getParam(platformOption)).toInt() : 0;
    m_device = (cfg.isSet(deviceOption)) ? (cfg.getParam(deviceOption)).toInt() : 0;
    m_param = (cfg.isSet(paramOption)) ? (cfg.getParam(paramOption)) : "";
    
    // CPA vs t-test
    if(cfg.isSet(cpaModuleOption) && cfg.isSet(ttestModuleOption)){
        cerr << "Only one of the following options is allowed: -C, -T\n";
        return CommandLineError;
    } else if(cfg.isSet(cpaModuleOption)){
        
        // CPA
        if(!cfg.isSet(functionOption)) {
            cerr << "No function selected: -F\n";
            return CommandLineError;
        }
        
        m_cpaModule = cfg.getParam(cpaModuleOption);
        
        QString function = cfg.getParam(functionOption);
        
        if(!function.compare("create")){
            // CPA create            
            if( !cfg.isSet(predictionsOption) ||
                !cfg.isSet(predictionsKOption) || 
                !cfg.isSet(predictionsQOption) || 
                !cfg.isSet(randTracesOption) ||
                !cfg.isSet(randTracesNOption) || 
                !cfg.isSet(samplesOption) ){
                
                cerr << "Some of CPA create parameters missing: -r, -n, -s, -p, -q, -k are required\n";
                return CommandLineError;
            }
                        
            m_randomTraces = cfg.getParam(randTracesOption);
            m_randomTracesCount = cfg.getParam(randTracesNOption).toLongLong();
            m_samplesPerTrace = cfg.getParam(samplesOption).toLongLong();
            m_predictions = cfg.getParam(predictionsOption);
            m_predictionsSetsCount = cfg.getParam(predictionsQOption).toLongLong();
            m_predictionsCandidatesCount = cfg.getParam(predictionsKOption).toLongLong();
            
            QTimer::singleShot(0, this, SLOT(cpaCreate()));
            return CommandLineTaskPlanned;
                        
        } else if(!function.compare("merge")){
            // CPA merge            
            if( !cfg.isSet(contextAOption) ||
                !cfg.isSet(contextBOption) ||
                !cfg.isSet(predictionsQOption) ) {
                    
                cerr << "Some of CPA merge parameters missing: -a, -b, -q are required\n";
                return CommandLineError;
            }
            
            m_contextA = cfg.getParam(contextAOption);
            m_contextB = cfg.getParam(contextBOption);
            m_predictionsSetsCount = cfg.getParam(predictionsQOption).toLongLong();
            
            QTimer::singleShot(0, this, SLOT(cpaMerge()));
            return CommandLineTaskPlanned;
            
            
        } else if(!function.compare("finalize")){
            // CPA finalize            
            if( !cfg.isSet(contextAOption) ||
                !cfg.isSet(predictionsQOption) ) {
                    
                cerr << "Some of CPA finalize parameters missing: -a, -q are required\n";
                return CommandLineError;
            }
            
            m_contextA = cfg.getParam(contextAOption);
            m_predictionsSetsCount = cfg.getParam(predictionsQOption).toLongLong();
            
            QTimer::singleShot(0, this, SLOT(cpaFinalize()));
            return CommandLineTaskPlanned;
            
            
        } else {
            cerr << "Invalid function selected: -F\n";    
            return CommandLineError;
        }
        
    } else if(cfg.isSet(ttestModuleOption)){
        
        // t-test
        if(!cfg.isSet(functionOption)) {
            cerr << "No function selected: -F\n";
            return CommandLineError;
        }
        
        m_tTestModule = cfg.getParam(ttestModuleOption);
        
        QString function = cfg.getParam(functionOption);
        
        if(!function.compare("create")){
            // t-test create            
            if( !cfg.isSet(randTracesOption) ||
                !cfg.isSet(randTracesNOption) ||
                !cfg.isSet(constTracesOption) ||
                !cfg.isSet(constTracesMOption) ||
                !cfg.isSet(samplesOption) ){
                
                cerr << "Some of t-test create parameters missing: -r, -n, -c, -m, -s are required\n";
                return CommandLineError;
            }
            
            m_randomTraces = cfg.getParam(randTracesOption);
            m_randomTracesCount = cfg.getParam(randTracesNOption).toLongLong();
            m_samplesPerTrace = cfg.getParam(samplesOption).toLongLong();
            m_constantTraces = cfg.getParam(constTracesOption);
            m_constantTracesCount = cfg.getParam(constTracesMOption).toLongLong();
            
            QTimer::singleShot(0, this, SLOT(tTestCreate()));
            return CommandLineTaskPlanned;
            
            
        } else if(!function.compare("merge")){
            // t-test merge
            if( !cfg.isSet(contextAOption) ||
                !cfg.isSet(contextBOption) ) {
                    
                cerr << "Some of t-test merge parameters missing: -a, -b are required\n";
                return CommandLineError;
            }
            
            m_contextA = cfg.getParam(contextAOption);
            m_contextB = cfg.getParam(contextBOption);
            
            QTimer::singleShot(0, this, SLOT(tTestMerge()));
            return CommandLineTaskPlanned;
            
            
        } else if(!function.compare("finalize")){
            // t-test finalize            
            if( !cfg.isSet(contextAOption) ) {
                    
                cerr << "Some of t-test finalize parameters missing: -a is required\n";
                return CommandLineError;
            }
            
            m_contextA = cfg.getParam(contextAOption);
            
            QTimer::singleShot(0, this, SLOT(tTestFinalize()));
            return CommandLineTaskPlanned;
            
            
        } else {
            cerr << "Invalid function selected: -F\n";    
            return CommandLineError;
        }
        
    }        
    
    return CommandLineNOP;
    
}

void Stan::queryPlugins(){
        
    QTextStream cout(stdout);
    bool found = false;
    
    QDir pluginsDir(QCoreApplication::instance()->applicationDirPath());
    pluginsDir.cd("plugins");           
        
    pluginsDir.cd("cpaengine");
    
    cout << "\nFound following CPA plug-ins, platforms and devices:\n\n";        
    
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();
        if (plugin) {
            CpaEngine * cpaEnginePlugin = qobject_cast<CpaEngine *>(plugin);
            if (cpaEnginePlugin){
                found = true;
                QString pluginId = fileName;                    
                if(pluginId.startsWith("lib")) pluginId.remove(0, 3);
                if(pluginId.startsWith("sicak")) pluginId.remove(0, 5);
                if(pluginId.endsWith(".so")) pluginId.chop(3);
                if(pluginId.endsWith(".dll")) pluginId.chop(4);
                
                cout << "* Plug-in ID: '" << pluginId << "', name: '" << cpaEnginePlugin->getPluginName() << "'\n";
                cout << cpaEnginePlugin->queryDevices() << "\n";
            }
        }
        pluginLoader.unload();
    }
    
    if(!found) cout << "* No CPA plug-in found!\n\n";
                
    pluginsDir.cd(QCoreApplication::instance()->applicationDirPath());
    pluginsDir.cd("plugins");
    pluginsDir.cd("ttestengine");
    found = false;
    cout << "Found following t-test plug-ins, platforms and devices:\n\n";        
    
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();
        if (plugin) {
            TTestEngine * tTestEnginePlugin = qobject_cast<TTestEngine *>(plugin);
            if (tTestEnginePlugin){
                found = true;
                QString pluginId = fileName;                    
                if(pluginId.startsWith("lib")) pluginId.remove(0, 3);
                if(pluginId.startsWith("sicak")) pluginId.remove(0, 5);
                if(pluginId.endsWith(".so")) pluginId.chop(3);
                if(pluginId.endsWith(".dll")) pluginId.chop(4);
                
                cout << "* Plug-in ID: '" << pluginId << "', name: '" << tTestEnginePlugin->getPluginName() << "'\n";
                cout << tTestEnginePlugin->queryDevices() << "\n";
            }
        }
        pluginLoader.unload();
    }
    
    if(!found) cout << "* No t-test plug-in found!\n\n";
    
    emit finished();
}    

bool Stan::loadCpaModule(){
    
    QDir pluginsDir(QCoreApplication::instance()->applicationDirPath());
    pluginsDir.cd("plugins");           
        
    pluginsDir.cd("cpaengine");
    
    QString fileName = m_cpaModule;
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
        m_cpaEngine = qobject_cast<CpaEngine *>(plugin);
        if (m_cpaEngine){
            return true;
        }
    }
    
    return false;
}

bool Stan::loadTTestModule(){
    
    QDir pluginsDir(QCoreApplication::instance()->applicationDirPath());
    pluginsDir.cd("plugins");           
        
    pluginsDir.cd("ttestengine");
    
    QString fileName = m_tTestModule;
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
        m_tTestEngine = qobject_cast<TTestEngine *>(plugin);
        if (m_tTestEngine){
            return true;
        }
    }
    
    return false;
}

void Stan::cpaCreate() {
    
    QTextStream cout(stdout);
    QTextStream cerr(stderr);
    cout << "Creating new CPA contexts...\n";
    cout.flush();
    
    if(!loadCpaModule()){
        cerr << "Failed to load the specified plug-in module\n";
        emit finished();
        return;
    }
    
    // Init
    try {
                
        QByteArray ba = m_param.toLocal8Bit();
        m_cpaEngine->init(m_platform, m_device, m_randomTracesCount, m_samplesPerTrace, m_predictionsCandidatesCount, ba.data());
        
    } catch(std::exception & e){
        cerr << "Failed to initialize the plug-in module: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    QString contextsFileName = "cpa-";
    contextsFileName.append(m_id);
    contextsFileName.append(".");
    if(m_predictionsSetsCount > 1) contextsFileName.append(QString("%1").arg(m_predictionsSetsCount));
    contextsFileName.append("ctx");
    
    QByteArray ba;
    std::fstream contextsFile;
    std::fstream powerTracesFile;
    std::fstream powerPredictionsFile;
        
    PowerTraces<int16_t> powerTraces;        
    PowerPredictions<uint8_t> powerPredictions;    
            
    // Open random traces file
    try {
        
        ba = m_randomTraces.toLocal8Bit();    
        powerTracesFile = openInFile(ba.data());
        
    } catch (std::exception & e) {
        cerr << "Failed to open random power traces file: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Open power predictions file
    try {
        
        ba = m_predictions.toLocal8Bit();
        powerPredictionsFile = openInFile(ba.data());
        
    } catch (std::exception & e) {
        cerr << "Failed to open power predictions file: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Alloc traces and predictions memory
    try {
            
        powerTraces.init(m_samplesPerTrace, m_randomTracesCount);
        powerPredictions.init(m_predictionsCandidatesCount, m_randomTracesCount);
        
    } catch (std::exception & e) {
        cerr << "Failed to allocate power traces and power predictions memory: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Load power traces
    try {
        
        fillArrayFromFile(powerTracesFile, powerTraces);                
        closeFile(powerTracesFile);        
        
    } catch (std::exception & e) {
        cerr << "Failed to read random power traces from file: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Open output file
    try{
        
        ba = contextsFileName.toLocal8Bit();    
        contextsFile = openOutFile(ba.data());
        
    } catch (std::exception & e) {
        cerr << "Failed to open output contexts file: " << e.what() << "\n";
        emit finished();
        return;
    }      
    
    UnivariateContext<double> context;
    
    // Traces are the same for the whole computation
    m_cpaEngine->setConstTraces(true);
    
    CoutProgress::get().start(m_predictionsSetsCount);
    
    for(size_t i = 0; i < m_predictionsSetsCount; i++){
        
        // Load power predictions
        try {
        
            fillArrayFromFile(powerPredictionsFile, powerPredictions);                            
            
        } catch (std::exception & e) {
            cerr << "Failed to read power predictions from file: " << e.what() << "\n";
            emit finished();
            return;
        }
        
        // Run the computation
        try {
            
            context = m_cpaEngine->createContext(powerTraces, powerPredictions);
            
        } catch(std::exception & e){
            cerr << "Failed to create CPA context: " << e.what() << "\n";
            emit finished();
            return;
        }
                
        // Save the context to the file
        try {
        
            writeContextToFile(contextsFile, context);
            
        } catch (std::exception & e) {
            cerr << "Failed to write a CPA context to file: " << e.what() << "\n";
            emit finished();
            return;
        }
        
        CoutProgress::get().update(i);
    }        
    
    CoutProgress::get().finish();
            
    
    // deInit
    try {
                
        closeFile(contextsFile);
        closeFile(powerPredictionsFile);
        m_cpaEngine->deInit();
        
    } catch(std::exception & e){
        cerr << "Failed to properly close the files or deinitialize the plug-in module: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Flush config to json file
    QJsonObject contextConf;
    contextConf["context-a"] = contextsFileName;
    contextConf["prediction-sets-count"] = QString::number(m_predictionsSetsCount);
    contextConf["contexts-count"] = QString::number(m_predictionsSetsCount);    
    QJsonDocument contextDoc(contextConf);
    QString contextDocFilename = m_id;
    contextDocFilename.append(".json");
    QFile contextDocFile(contextDocFilename);
    if(contextDocFile.open(QIODevice::WriteOnly)){
        contextDocFile.write(contextDoc.toJson());
    } else {
        cerr << "Failed to save a config JSON file.\n";
    }
    
    
    cout << QString("Created %1 new CPA contexts using\n * %2 power traces with %3 samples per trace, from '%4',\n * %1 prediction sets containing %5 power predictions for each of these power traces, from '%6'\nand saved to '%7'.\n").arg(m_predictionsSetsCount).arg(m_randomTracesCount).arg(m_samplesPerTrace).arg(m_randomTraces).arg(m_predictionsCandidatesCount).arg(m_predictions).arg(contextsFileName);
    
    emit finished();
}

void Stan::cpaMerge() {
    
    QTextStream cout(stdout);
    QTextStream cerr(stderr);
    cout << "Merging CPA contexts...\n";
    cout.flush();
    
    if(!loadCpaModule()){
        cerr << "Failed to load the specified plug-in module.\n";
        emit finished();
        return;
    }
    
    // Init
    try {
                
        QByteArray ba = m_param.toLocal8Bit();
        m_cpaEngine->init(m_platform, m_device, m_randomTracesCount, m_samplesPerTrace, m_predictionsCandidatesCount, ba.data());
        
    } catch(std::exception & e){
        cerr << "Failed to initialize the plug-in module: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    QString contextsFileName = "cpa-";
    contextsFileName.append(m_id);
    contextsFileName.append("-merged.");
    if(m_predictionsSetsCount > 1) contextsFileName.append(QString("%1").arg(m_predictionsSetsCount));
    contextsFileName.append("ctx");
    
    QByteArray ba;
    std::fstream outputFile;
    std::fstream firstCtxFile;
    std::fstream secondCtxFile;
    
    // Open first context file
    try {
        
        ba = m_contextA.toLocal8Bit();    
        firstCtxFile = openInFile(ba.data());
        
    } catch (std::exception & e) {
        cerr << "Failed to open context-A file: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Open second context file
    try {
        
        ba = m_contextB.toLocal8Bit();    
        secondCtxFile = openInFile(ba.data());
        
    } catch (std::exception & e) {
        cerr << "Failed to open context-B file: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Open output context file
    try {
        
        ba = contextsFileName.toLocal8Bit();    
        outputFile = openOutFile(ba.data());
        
    } catch (std::exception & e) {
        cerr << "Failed to open output file: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    UnivariateContext<double> firstContext;
    UnivariateContext<double> secondContext;        
    
    CoutProgress::get().start(m_predictionsSetsCount);
    
    for(size_t i = 0; i < m_predictionsSetsCount; i++){
        
        // Read context from first file
        try {
            firstContext = readContextFromFile<double>(firstCtxFile);
        } catch(std::exception & e) {
            cerr << "Failed to read from context-A file: " << e.what() << "\n";
            emit finished();
            return;
        }
        
        // Read context from second file
        try {
            secondContext = readContextFromFile<double>(secondCtxFile);
        } catch(std::exception & e) {
            cerr << "Failed to read from context-B file: " << e.what() << "\n";
            emit finished();
            return;
        }                
        
        // Merge them
        try {            
            m_cpaEngine->mergeContexts(firstContext, secondContext);
            
        } catch(std::exception & e){
            cerr << "Failed to merge CPA contexts: " << e.what() << "\n";
            emit finished();
            return;
        }
        
        // Save the merged context to file
        try {
            writeContextToFile(outputFile, firstContext);
        } catch(std::exception & e) {
            cerr << "Failed to save a merged context to file: " << e.what() << "\n";
            emit finished();
            return;
        }  
        
        CoutProgress::get().update(i);
    }        
    
    CoutProgress::get().finish();
    
    // deInit
    try {
        closeFile(outputFile);
        closeFile(firstCtxFile);
        closeFile(secondCtxFile);
        m_cpaEngine->deInit();
        
    } catch(std::exception & e){
        cerr << "Failed to properly close files or to deinitialize the plug-in module: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Flush config to json file
    QJsonObject contextConf;
    contextConf["context-a"] = contextsFileName;
    contextConf["prediction-sets-count"] = QString::number(m_predictionsSetsCount);
    contextConf["contexts-count"] = QString::number(m_predictionsSetsCount);    
    QJsonDocument contextDoc(contextConf);
    QString contextDocFilename = m_id;
    contextDocFilename.append(".json");
    QFile contextDocFile(contextDocFilename);
    if(contextDocFile.open(QIODevice::WriteOnly)){
        contextDocFile.write(contextDoc.toJson());
    } else {
        cerr << "Failed to save a config JSON file.\n";
    }    
    
    // Assuming contexts in one file are all of the same cardinality!!!
    cout << QString("Created %1 merged CPA contexts using\n * %1 contexts based on %5 traces from '%2'\n * %1 contexts based on %6 traces from '%3'\nand saved to '%4'.\n").arg(m_predictionsSetsCount).arg(m_contextA).arg(m_contextB).arg(contextsFileName).arg(firstContext.p1Card() - secondContext.p1Card()).arg(secondContext.p1Card());
    
    emit finished();
}

void Stan::cpaFinalize() {
    
    QTextStream cout(stdout);
    QTextStream cerr(stderr);
    cout << "Finalizing CPA context...\n";
    cout.flush();    
    
    if(!loadCpaModule()){
        cerr << "Failed to load the specified plug-in module.\n";
        emit finished();
        return;
    }
    
    // Init
    try {
                
        QByteArray ba = m_param.toLocal8Bit();
        m_cpaEngine->init(m_platform, m_device, m_randomTracesCount, m_samplesPerTrace, m_predictionsCandidatesCount, ba.data());
        
    } catch(std::exception & e){
        cerr << "Failed to initialize the plug-in module: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    QString correlationsFileName = "cpa-";
    correlationsFileName.append(m_id);
    correlationsFileName.append(".");
    if(m_predictionsSetsCount > 1) correlationsFileName.append(QString("%1").arg(m_predictionsSetsCount));
    correlationsFileName.append("cor");
    
    QByteArray ba;
    std::fstream outputFile;
    std::fstream ctxFile;
    
    // Open context file
    try {
        
        ba = m_contextA.toLocal8Bit();    
        ctxFile = openInFile(ba.data());
        
    } catch (std::exception & e) {
        cerr << "Failed to open context-A file: " << e.what() << "\n";
        emit finished();
        return;
    }    
    
    // Open output correlations file
    try {
        
        ba = correlationsFileName.toLocal8Bit();    
        outputFile = openOutFile(ba.data());
        
    } catch (std::exception & e) {
        cerr << "Failed to open output file: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    UnivariateContext<double> context;    
    Matrix<double> correlations;
    
    CoutProgress::get().start(m_predictionsSetsCount);
    
    for(size_t i = 0; i < m_predictionsSetsCount; i++){
        
        // Read the context from file
        try {
            
            context = readContextFromFile<double>(ctxFile);
            
        } catch(std::exception & e) {
            cerr << "Failed to read from context-A file: " << e.what() << "\n";
            emit finished();
            return;
        }
        
        // Compute correlations
        try {
            
            correlations = m_cpaEngine->finalizeContext(context);
            
        } catch(std::exception & e){
            cerr << "Failed to finalize CPA context: " << e.what() << "\n";
            emit finished();
            return;
        }
        
        // Save the correlations to file
        try {
            writeArrayToFile(outputFile, correlations);
        } catch(std::exception & e) {
            cerr << "Failed to save a merged context to file: " << e.what() << "\n";
            emit finished();
            return;
        }       
        
        CoutProgress::get().update(i);
    }
    
    CoutProgress::get().finish();
    
    // deInit
    try {
        closeFile(outputFile);
        closeFile(ctxFile);        
        m_cpaEngine->deInit();
        
    } catch(std::exception & e){
        cerr << "Failed to deinitialize the plug-in module: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Flush config to json file
    QJsonObject correlConf;
    correlConf["correlations"] = correlationsFileName;
    correlConf["correlations-sets-count"] = QString::number(m_predictionsSetsCount);
    correlConf["prediction-sets-count"] = QString::number(m_predictionsSetsCount);
    correlConf["contexts-count"] = QString::number(m_predictionsSetsCount);    
    correlConf["prediction-candidates-count"] = QString::number(correlations.rows());
    correlConf["correlations-candidates-count"] = QString::number(correlations.rows());
    correlConf["samples-per-trace"] = QString::number(correlations.cols());
    QJsonDocument correlDoc(correlConf);
    QString contextDocFilename = m_id;
    contextDocFilename.append(".json");
    QFile contextDocFile(contextDocFilename);
    if(contextDocFile.open(QIODevice::WriteOnly)){
        contextDocFile.write(correlDoc.toJson());
    } else {
        cerr << "Failed to save a config JSON file.\n";
    }
    
    
    cout << QString("Created %1 correlation matrices (%4x%5) using\n * %1 contexts based on %6 from '%2'\nand saved to '%3'.\n").arg(m_predictionsSetsCount).arg(m_contextA).arg(correlationsFileName).arg(correlations.cols()).arg(correlations.rows()).arg(context.p1Card());
    
    emit finished();
}

void Stan::tTestCreate() {
    
    QTextStream cout(stdout);
    QTextStream cerr(stderr);
    cout << "Creating new t-test context...\n";
    cout.flush();
        
    if(!loadTTestModule()){
        cerr << "Failed to load the specified plug-in module.\n";
        emit finished();
        return;
    }
    
    // Init
    try {
                
        QByteArray ba = m_param.toLocal8Bit();
        m_tTestEngine->init(m_platform, m_device, m_randomTracesCount, m_constantTracesCount, m_samplesPerTrace, ba.data());
        
    } catch(std::exception & e){
        cerr << "Failed to initialize the plug-in module: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    QString contextsFileName = "ttest-";
    contextsFileName.append(m_id);
    contextsFileName.append(".");
    contextsFileName.append("ctx");
    
    QByteArray ba;
    std::fstream contextsFile;
    std::fstream randomTracesFile;
    std::fstream constTracesFile;
        
    PowerTraces<int16_t> randomTraces;        
    PowerTraces<int16_t> constTraces;    
            
    // Open random traces file
    try {
        
        ba = m_randomTraces.toLocal8Bit();    
        randomTracesFile = openInFile(ba.data());
        
    } catch (std::exception & e) {
        cerr << "Failed to open random power traces file: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Open constant traces file
    try {
        
        ba = m_constantTraces.toLocal8Bit();
        constTracesFile = openInFile(ba.data());
        
    } catch (std::exception & e) {
        cerr << "Failed to open power predictions file: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Allocate random/constant traces memory
    try {
            
        randomTraces.init(m_samplesPerTrace, m_randomTracesCount);
        constTraces.init(m_samplesPerTrace, m_constantTracesCount);
        
    } catch (std::exception & e) {
        cerr << "Failed to allocate random and constant power traces memory: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Load random traces
    try {
        
        fillArrayFromFile(randomTracesFile, randomTraces);                
        closeFile(randomTracesFile);        
        
    } catch (std::exception & e) {
        cerr << "Failed to read random power traces from file: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Load constant traces
    try {
        
        fillArrayFromFile(constTracesFile, constTraces);                
        closeFile(constTracesFile);        
        
    } catch (std::exception & e) {
        cerr << "Failed to read constant power traces from file: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Open output file
    try{
        
        ba = contextsFileName.toLocal8Bit();    
        contextsFile = openOutFile(ba.data());
        
    } catch (std::exception & e) {
        cerr << "Failed to open output contexts file: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    UnivariateContext<double> context;
    
    CoutProgress::get().start(100);
    
    // Run the computation
    try {
            
        context = m_tTestEngine->createContext(randomTraces, constTraces);
        
    } catch(std::exception & e){
        cerr << "Failed to compute t-test context: " << e.what() << "\n";
        emit finished();
        return;
    }    
    
    CoutProgress::get().finish();
        
    // Save context to file
    try {
        
        writeContextToFile(contextsFile, context);
        closeFile(contextsFile);
        
    } catch (std::exception & e) {
        cerr << "Failed to write t-test context to file: " << e.what() << "\n";
        emit finished();
        return;
    }    
        
    // deInit
    try {
                
        m_tTestEngine->deInit();
        
    } catch(std::exception & e){
        cerr << "Failed to deinitialize the plug-in module: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Flush config to json file
    QJsonObject contextConf;
    contextConf["context-a"] = contextsFileName; 
    QJsonDocument contextDoc(contextConf);
    QString contextDocFilename = m_id;
    contextDocFilename.append(".json");
    QFile contextDocFile(contextDocFilename);
    if(contextDocFile.open(QIODevice::WriteOnly)){
        contextDocFile.write(contextDoc.toJson());
    } else {
        cerr << "Failed to save a config JSON file.\n";
    }    
    
    cout << QString("Created new t-test context using\n * %1 random power traces with %2 samples per trace, from '%3',\n * %4 constant power traces with %2 samples per trace, from '%5'\nand saved to '%6'.\n").arg(m_randomTracesCount).arg(m_samplesPerTrace).arg(m_randomTraces).arg(m_constantTracesCount).arg(m_constantTraces).arg(contextsFileName);
    
    emit finished();
}

void Stan::tTestMerge() {
    
    QTextStream cout(stdout);
    QTextStream cerr(stderr);
    cout << "Merging t-test contexts...\n";
    cout.flush();
    
    if(!loadTTestModule()){
        cerr << "Failed to load the specified plug-in module.\n";
        emit finished();
        return;
    }
    
    // Init
    try {
                
        QByteArray ba = m_param.toLocal8Bit();
        m_tTestEngine->init(m_platform, m_device, m_randomTracesCount, m_constantTracesCount, m_samplesPerTrace, ba.data());
        
    } catch(std::exception & e){
        cerr << "Failed to initialize the plug-in module: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    QString contextsFileName = "ttest-";
    contextsFileName.append(m_id);
    contextsFileName.append("-merged.");
    contextsFileName.append("ctx");
    
    QByteArray ba;
    std::fstream outputFile;
    std::fstream firstCtxFile;
    std::fstream secondCtxFile;
    
    // Open first context file
    try {
        
        ba = m_contextA.toLocal8Bit();    
        firstCtxFile = openInFile(ba.data());
        
    } catch (std::exception & e) {
        cerr << "Failed to open context-A file: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Open second context file
    try {
        
        ba = m_contextB.toLocal8Bit();    
        secondCtxFile = openInFile(ba.data());
        
    } catch (std::exception & e) {
        cerr << "Failed to open context-B file: " << e.what() << "\n";
        emit finished();
        return;
    }        
    
    UnivariateContext<double> firstContext;
    UnivariateContext<double> secondContext;     
    
    // Read context from first file
    try {
        firstContext = readContextFromFile<double>(firstCtxFile);
        closeFile(firstCtxFile);
    } catch(std::exception & e) {
        cerr << "Failed to read from context-A file: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Read context from second file
    try {
        secondContext = readContextFromFile<double>(secondCtxFile);
        closeFile(secondCtxFile);
    } catch(std::exception & e) {
        cerr << "Failed to read from context-B file: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Open output context file
    try {
        
        ba = contextsFileName.toLocal8Bit();    
        outputFile = openOutFile(ba.data());
        
    } catch (std::exception & e) {
        cerr << "Failed to open output file: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    CoutProgress::get().start(100);
    // Merge contexts
    try {
        
        m_tTestEngine->mergeContexts(firstContext, secondContext);
        
    } catch(std::exception & e){
        cerr << "Failed to merge t-test contexts: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    CoutProgress::get().finish();
    
    // Save the merged context to file
    try {
        writeContextToFile(outputFile, firstContext);
        closeFile(outputFile);
    } catch(std::exception & e) {
        cerr << "Failed to save a merged context to file: " << e.what() << "\n";
        emit finished();
        return;
    }        
    
    // deInit
    try {
                
        m_tTestEngine->deInit();
        
    } catch(std::exception & e){
        cerr << "Failed to deinitialize the plug-in module: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Flush config to json file
    QJsonObject contextConf;
    contextConf["context-a"] = contextsFileName; 
    QJsonDocument contextDoc(contextConf);
    QString contextDocFilename = m_id;
    contextDocFilename.append(".json");
    QFile contextDocFile(contextDocFilename);
    if(contextDocFile.open(QIODevice::WriteOnly)){
        contextDocFile.write(contextDoc.toJson());
    } else {
        cerr << "Failed to save a config JSON file.\n";
    }    
    
    cout << QString("Created a merged t-test context using\n * a context with %4 random and %5 constant power traces from '%1'\n * a context with %6 random and %7 constant power traces from '%2'\nand saved to '%3'.\n").arg(m_contextA).arg(m_contextB).arg(contextsFileName).arg(firstContext.p1Card() - secondContext.p1Card()).arg(firstContext.p2Card() - secondContext.p2Card()).arg(secondContext.p1Card()).arg(secondContext.p2Card());
    
    emit finished();
}

void Stan::tTestFinalize() {
    
    QTextStream cout(stdout);
    QTextStream cerr(stderr);
    cout << "Finalizing t-test context...\n";
    cout.flush();
    
    if(!loadTTestModule()){
        cerr << "Failed to load the specified plug-in module.\n";
        emit finished();
        return;
    }
    
    // Init
    try {
                
        QByteArray ba = m_param.toLocal8Bit();
        m_tTestEngine->init(m_platform, m_device, m_randomTracesCount, m_constantTracesCount, m_samplesPerTrace, ba.data());
        
    } catch(std::exception & e){
        cerr << "Failed to initialize the plug-in module: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    QString tValsFileName = "ttest-";
    tValsFileName.append(m_id);
    tValsFileName.append(".");
    tValsFileName.append("tvals");
    
    QByteArray ba;
    std::fstream outputFile;
    std::fstream ctxFile;
    
    UnivariateContext<double> context;    
    Matrix<double> tVals;
    
    // Open context file
    try {
        
        ba = m_contextA.toLocal8Bit();    
        ctxFile = openInFile(ba.data());
        
    } catch (std::exception & e) {
        cerr << "Failed to open context-A file: " << e.what() << "\n";
        emit finished();
        return;
    }    
    
    // Read the context from file
    try {
        
        context = readContextFromFile<double>(ctxFile);
        closeFile(ctxFile);
        
    } catch(std::exception & e) {
        cerr << "Failed to read from context-A file: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Open output tvals file
    try {
        
        ba = tValsFileName.toLocal8Bit();    
        outputFile = openOutFile(ba.data());
        
    } catch (std::exception & e) {
        cerr << "Failed to open output file: " << e.what() << "\n";
        emit finished();
        return;
    }
                     
    CoutProgress::get().start(100);
    // Compute tvals
    try {
        
        tVals = m_tTestEngine->finalizeContext(context);
        
    } catch(std::exception & e){
        cerr << "Failed to finalize CPA context: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    CoutProgress::get().finish();
    
    // Save the tvals to file
    try {
        writeArrayToFile(outputFile, tVals);
        closeFile(outputFile);
    } catch(std::exception & e) {
        cerr << "Failed to save a merged context to file: " << e.what() << "\n";
        emit finished();
        return;
    }       
    
    // deInit
    try {
                
        m_tTestEngine->deInit();
        
    } catch(std::exception & e){
        cerr << "Failed to deinitialize the plug-in module: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    // Flush config to json file
    QJsonObject tvalsConf;
    tvalsConf["t-values"] = tValsFileName; 
    tvalsConf["samples-per-trace"] = QString::number(tVals.cols());
    QJsonDocument tvalsDoc(tvalsConf);
    QString tvalsDocFilename = m_id;
    tvalsDocFilename.append(".json");
    QFile contextDocFile(tvalsDocFilename);
    if(contextDocFile.open(QIODevice::WriteOnly)){
        contextDocFile.write(tvalsDoc.toJson());
    } else {
        cerr << "Failed to save a config JSON file.\n";
    }    
    
    cout << QString("Created 2 vectors containing %3 t-values and %3 degrees of freedom using\n * a context with %4 random and %5 constant power traces from '%1'\nand saved to '%2'.\n").arg(m_contextA).arg(tValsFileName).arg(tVals.cols()).arg(context.p1Card()).arg(context.p2Card());
    
    emit finished();
}
    
