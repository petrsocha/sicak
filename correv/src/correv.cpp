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
* \file correv.cpp
*
* \brief SICAK CORRelations EValuation text-based UI
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
#include "correv.h"


CorrEv::CommandLineParseResult CorrEv::parseCommandLineParams(QCommandLineParser & parser) {
    
    QTextStream cout(stdout);
    QTextStream cerr(stderr);
    
    // Function options            
        
    const QCommandLineOption queryOption({"Q", "query"}, "Query available CPA correlation matrix evaluation and keyguess evaluation plug-in modules (-E, -K).");
    parser.addOption(queryOption);
    
    const QCommandLineOption corrModuleOption({"E", "correlations-eval-module"}, "ID of a CPA correlation matrix evaluation plug-in module to use.", "string");
    parser.addOption(corrModuleOption);    
    
    const QCommandLineOption keyguessModuleOption({"K", "keyguess-eval-module"}, "ID of a CPA keyguess evaluation plug-in module to use.", "string");
    parser.addOption(keyguessModuleOption);    
    
    const QCommandLineOption correlationsOption({"c", "correlations"}, "File containing -q correlation matrices, each of which -s wide and -k tall (double).", "filepath");
    parser.addOption(correlationsOption);    
    
    const QCommandLineOption correlationsQOption({"q", "prediction-sets-count", "contexts-count"}, "Number of correlation matrices. E.g. attacking AES-128 key, this value would be 16.", "positive integer");
    parser.addOption(correlationsQOption);    
    
    const QCommandLineOption correlationsKOption({"k", "prediction-candidates-count"}, "Number of key candidates, i.e. rows of correlation matrix. E.g. attacking AES-128 key, this value would be 256.", "positive integer");
    parser.addOption(correlationsKOption);        
    
    const QCommandLineOption samplesOption({"s", "samples-per-trace"}, "Number of samples per trace, i.e. cols of correlation matrix.", "positive integer");
    parser.addOption(samplesOption);    
    
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
    
    m_param = (cfg.isSet(paramOption)) ? (cfg.getParam(paramOption)) : "";
    
    if(cfg.isSet(corrModuleOption) != cfg.isSet(keyguessModuleOption)){
        cerr << "Both evaluation modules must be set: -E, -K\n";
        return CommandLineError;
    } else if(cfg.isSet(corrModuleOption) && cfg.isSet(keyguessModuleOption)) {
        
        m_cpaCorrEval = cfg.getParam(corrModuleOption);
        m_cpaKeyEval = cfg.getParam(keyguessModuleOption);
        
        if( !cfg.isSet(correlationsOption) || 
            !cfg.isSet(correlationsQOption) ||
            !cfg.isSet(correlationsKOption) ||
            !cfg.isSet(samplesOption)
        ){
            
                cerr << "Some of CPA correlations evaluation parameters missing: -c, -q, -k, -s, are required\n";
                return CommandLineError;
            }
        
        m_correlations = cfg.getParam(correlationsOption);
        m_correlationsQCount = cfg.getParam(correlationsQOption).toLongLong();
        m_correlationsKCount = cfg.getParam(correlationsKOption).toLongLong();
        m_samplesPerTrace = cfg.getParam(samplesOption).toLongLong();
        
        QTimer::singleShot(0, this, SLOT(evaluate()));
        return CommandLineTaskPlanned;
    }                
    
    return CommandLineNOP;
    
}

void CorrEv::queryPlugins(){
        
    QTextStream cout(stdout);
    bool found = false;
    
    QDir pluginsDir(QCoreApplication::instance()->applicationDirPath());
    pluginsDir.cd("plugins");           
        
    pluginsDir.cd("cpacorreval");
    
    cout << "\nFound following CPA correlation matrix evaluation plug-ins:\n\n";        
    
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();
        if (plugin) {
            CpaCorrEval * cpaCorrEvalPlugin = qobject_cast<CpaCorrEval *>(plugin);
            if (cpaCorrEvalPlugin){
                found = true;
                QString pluginId = fileName;                    
                if(pluginId.startsWith("lib")) pluginId.remove(0, 3);
                if(pluginId.startsWith("sicak")) pluginId.remove(0, 5);
                if(pluginId.endsWith(".so")) pluginId.chop(3);
                if(pluginId.endsWith(".dll")) pluginId.chop(4);
                
                cout << "* Plug-in ID: '" << pluginId << "', name: '" << cpaCorrEvalPlugin->getPluginName() << "'\n";
            }
        }
        pluginLoader.unload();
    }
    
    if(!found) cout << "* No correlation matrix evaluation plug-in found!\n";
                
    pluginsDir.cd(QCoreApplication::instance()->applicationDirPath());
    pluginsDir.cd("plugins");
    pluginsDir.cd("cpakeyeval");
    found = false;
    cout << "\nFound following CPA keyguess evaluation plug-ins:\n\n";        
    
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();
        if (plugin) {
            CpaKeyEval * cpaKeyEvalPlugin = qobject_cast<CpaKeyEval *>(plugin);
            if (cpaKeyEvalPlugin){
                found = true;
                QString pluginId = fileName;                    
                if(pluginId.startsWith("lib")) pluginId.remove(0, 3);
                if(pluginId.startsWith("sicak")) pluginId.remove(0, 5);
                if(pluginId.endsWith(".so")) pluginId.chop(3);
                if(pluginId.endsWith(".dll")) pluginId.chop(4);
                
                cout << "* Plug-in ID: '" << pluginId << "', name: '" << cpaKeyEvalPlugin->getPluginName() << "'\n";
            }
        }
        pluginLoader.unload();
    }
    
    if(!found) cout << "* No CPA keyguess evaluation plug-in found!\n";
    cout << "\n";
    
    emit finished();
}    

bool CorrEv::loadCorrEvalModule() {
    
    QDir pluginsDir(QCoreApplication::instance()->applicationDirPath());
    pluginsDir.cd("plugins");           
        
    pluginsDir.cd("cpacorreval");
    
    QString fileName = m_cpaCorrEval;
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
        m_cpaCorrEvalPlugin = qobject_cast<CpaCorrEval *>(plugin);
        if (m_cpaCorrEvalPlugin){
            return true;
        }
    }
    
    return false;
    
}

bool CorrEv::loadKeyEvalModule() {
    
    QDir pluginsDir(QCoreApplication::instance()->applicationDirPath());
    pluginsDir.cd("plugins");           
        
    pluginsDir.cd("cpakeyeval");
    
    QString fileName = m_cpaKeyEval;
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
        m_cpaKeyEvalPlugin = qobject_cast<CpaKeyEval *>(plugin);
        if (m_cpaKeyEvalPlugin){
            return true;
        }
    }
    
    return false;
    
}

void CorrEv::evaluate() {
    
    QTextStream cout(stdout);
    QTextStream cerr(stderr);
    cout << "Evaluating CPA correlation matrices...\n";
    cout.flush();
    
    if(!loadCorrEvalModule()){
        cerr << "Failed to load the specified correlations matrix evaluation plug-in module\n";
        emit finished();
        return;
    }
    
    if(!loadKeyEvalModule()){
        cerr << "Failed to load the specified keyguess evaluation plug-in module\n";
        emit finished();
        return;
    }
    
    // Init
    QByteArray ba = m_param.toLocal8Bit();
    try {
                        
        m_cpaCorrEvalPlugin->init(ba.data());
        
    } catch(std::exception & e){
        cerr << "Failed to initialize the correlations matrix evaluation plug-in module: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    try {
                
        m_cpaKeyEvalPlugin->init(ba.data());
        
    } catch(std::exception & e){
        cerr << "Failed to initialize the keyguess evaluation plug-in module: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    std::fstream correlationsFile;
    
    // Open correlations file
    try {
        
        ba = m_correlations.toLocal8Bit();
        correlationsFile = openInFile(ba.data());
        
    } catch (std::exception & e) {
        cerr << "Failed to open power predictions file: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    Vector<size_t> keyGuess(m_correlationsQCount);
    Matrix<double> correlationMatrix(m_samplesPerTrace, m_correlationsKCount);
    
    CoutProgress::get().start(m_correlationsQCount);
    // Evaluate each correlation matrix
    for(size_t i = 0; i < m_correlationsQCount; i++) {
     
        // Load correlation matrix
        try {
        
            fillArrayFromFile(correlationsFile, correlationMatrix);                            
            
        } catch (std::exception & e) {
            cerr << "Failed to read correlation matrix from file: " << e.what() << "\n";
            emit finished();
            return;
        }
        
        size_t sample;
        
        try {
            
            m_cpaCorrEvalPlugin->evaluateCorrelations(correlationMatrix, sample, keyGuess(i));
            
        } catch (std::exception & e) {            
            cerr << "Failed to evaluate correlation matrix: " << e.what() << "\n";
            emit finished();
            return;            
        }
        
        CoutProgress::get().update(i);
    }
    
    CoutProgress::get().finish();
    
    Vector<uint8_t> cipherKey;
    
    // When the full keyguess is obtained, evaluate it to obtain a cipher key
    try {
        
        cipherKey = m_cpaKeyEvalPlugin->evaluateKeyCandidates(keyGuess);
        
    } catch (std::exception & e){
        cerr << "Failed to evaluate the keyguess: " << e.what() << "\n";
        emit finished();
        return;            
    }
    
    QByteArray key((char *)cipherKey.data(), cipherKey.size());
    cout << "Obtained key (hex): '" << QString(key.toHex()) << "'\n";
        
    // deInit
    try {
                        
        m_cpaCorrEvalPlugin->deInit();
        m_cpaKeyEvalPlugin->deInit();
        
    } catch(std::exception & e){
        cerr << "Failed to properly close the files or deinitialize the plug-in modules: " << e.what() << "\n";
        emit finished();
        return;
    }
    
    emit finished();
}
