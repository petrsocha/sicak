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
* \file stan.h
*
* \brief SICAK STatistical ANalysis text-based UI
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef STAN_H
#define STAN_H

#include <QObject>
#include <QCommandLineParser>
#include "cpaengine.h"
#include "ttestengine.h"

/**
* \class Stan
* \ingroup Sicak
*
* \brief Class providing text-based UI front-end to CpaEngine and TTestEngine plug-in modules
*
*/
class Stan: public QObject {
  
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
    
    Stan(QObject *parent = 0) : QObject(parent), m_id(""), m_platform(0), m_device(0), m_param(""), m_cpaEngine(nullptr), m_tTestEngine(nullptr), m_cpaModule(""), m_tTestModule(""), m_randomTraces(""), m_randomTracesCount(0), m_constantTraces(""), m_constantTracesCount(0), m_samplesPerTrace(0), m_predictions(""), m_predictionsSetsCount(0), m_predictionsCandidatesCount(0), m_contextA(""), m_contextB("") {}
    
    /// Parse parameters from the command line and configuration files and plan tasks for event loop accordingly
    CommandLineParseResult parseCommandLineParams(QCommandLineParser & parser);    

protected:
    
    /// Load the specified CPA computation module
    bool loadCpaModule();
    /// Load the specified t-test computation module
    bool loadTTestModule();
    
    QString m_id;
    int m_platform;
    int m_device;
    QString m_param;
    
    CpaEngine * m_cpaEngine;
    TTestEngine * m_tTestEngine;
    
    QString m_cpaModule;
    QString m_tTestModule;
    
    QString m_randomTraces;
    size_t m_randomTracesCount;
    
    QString m_constantTraces;
    size_t m_constantTracesCount;
    
    size_t m_samplesPerTrace;
    
    QString m_predictions;
    size_t m_predictionsSetsCount;
    size_t m_predictionsCandidatesCount;            
    
    QString m_contextA;
    QString m_contextB;
    
        
public slots:
    
    /// Query and print available plugins
    void queryPlugins();
    
    /// Create a new CPA contexts and save them to file
    void cpaCreate();
    /// Merge the existing CPA contexts and save them to file
    void cpaMerge();
    /// Finalize the existing CPA context and save correlation matrices to file
    void cpaFinalize();
    
    /// Create a new t-test context and save it to file
    void tTestCreate();
    /// Merge the existing t-test contexts and save it to file
    void tTestMerge();
    /// Finalize the existing t-test context and save the t-vals and degrees of freedom to file
    void tTestFinalize();
    
signals:
    
    void finished();
    
};

#endif /* STAN_H */

