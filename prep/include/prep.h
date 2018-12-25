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
* \file prep.h
*
* \brief SICAK PREProcessing text-based UI
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef PREP_H
#define PREP_H

#include <QObject>
#include <QCommandLineParser>
#include "blockprocess.h"
#include "tracesprocess.h"

/**
* \class Prep
* \ingroup Sicak
*
* \brief Class providing text-based UI front-end to BlockProcess and TracesProcess plug-in modules
*
*/
class Prep: public QObject {
  
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
    
    Prep(QObject *parent = 0) : QObject(parent), m_tracesEngine(nullptr), m_blockEngine(nullptr), m_param(""), m_id(""), m_tracesModule(""), m_blockModule(""), m_traces(""), m_tracesN(0), m_samples(0), m_blocks(""), m_blocksM(0), m_blocksLen(0) {}
    
    /// Parse parameters from the command line and configuration files and plan tasks for event loop accordingly
    CommandLineParseResult parseCommandLineParams(QCommandLineParser & parser);    

protected:
    
    /// Load the specified traces preprocessing module
    bool loadTracesModule();
    /// Load the specified block data preprocessing module
    bool loadBlocksModule();
    
    TracesProcess * m_tracesEngine;
    BlockProcess * m_blockEngine;
    
    QString m_param;
    QString m_id;
    
    QString m_tracesModule;
    QString m_blockModule;
    
    QString m_traces;
    size_t m_tracesN;
    size_t m_samples;
    
    QString m_blocks;
    size_t m_blocksM;
    size_t m_blocksLen;
    
public slots:
    
    /// Query and print available plugins
    void queryPlugins();
    
    /// Load and initialize given modules, load the input data and run the preprocessing plugin
    void preprocessTraces();
    /// Load and initialize given modules, load the input data and run the preprocessing plugin
    void preprocessBlocks();
    
    
signals:
    
    void finished();
    
};

#endif /* PREP_H */

