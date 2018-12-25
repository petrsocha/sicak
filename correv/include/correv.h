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
* \file correv.h
*
* \brief SICAK CORRelations EValuation text-based UI
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef CORREV_H
#define CORREV_H

#include <QObject>
#include <QCommandLineParser>
#include "cpacorreval.h"
#include "cpakeyeval.h"

/**
* \class CorrEv
* \ingroup Sicak
*
* \brief Class providing text-based UI front-end to CpaCorrEval and CpaKeyEval plug-in modules
*
*/
class CorrEv: public QObject {
  
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
    
    CorrEv(QObject *parent = 0) : QObject(parent), m_cpaCorrEval(""), m_cpaKeyEval(""), m_cpaCorrEvalPlugin(nullptr), m_cpaKeyEvalPlugin(nullptr), m_correlations(""), m_correlationsQCount(0), m_correlationsKCount(0), m_samplesPerTrace(0), m_param("") {}
    
    /// Parse parameters from the command line and configuration files and plan tasks for event loop accordingly
    CommandLineParseResult parseCommandLineParams(QCommandLineParser & parser);    

protected:
    
    /// Load the specified correlation matrix evaluation module
    bool loadCorrEvalModule();
    /// Load the specified keyguess evaluation module
    bool loadKeyEvalModule();
    
    QString m_cpaCorrEval;
    QString m_cpaKeyEval;
    
    CpaCorrEval * m_cpaCorrEvalPlugin;
    CpaKeyEval * m_cpaKeyEvalPlugin;
    
    QString m_correlations;
    size_t m_correlationsQCount;
    size_t m_correlationsKCount;
    size_t m_samplesPerTrace;
        
    QString m_param;
    
public slots:
    
    /// Query and print available plugins
    void queryPlugins();
    /// Run the correlation matrices evaluation and print the retrieved cipher key to the standard output
    void evaluate();
    
signals:
    
    void finished();
    
};

#endif /* CORREV_H */

