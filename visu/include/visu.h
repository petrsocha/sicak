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
* \file visu.h
*
* \brief SICAK VISUalisation text-based UI
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef VISU_H
#define VISU_H

#include <QObject>
#include <QCommandLineParser>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

QT_CHARTS_USE_NAMESPACE

#include "blockprocess.h"
#include "tracesprocess.h"

/**
* \class Visu
* \ingroup Sicak
*
* \brief Class providing text-based UI allowing to plot power traces, correlation traces or t-value traces
*
*/
class Visu: public QObject {
  
Q_OBJECT

public:
    
    enum CommandLineParseResult {
        CommandLineProcessChart,
        CommandLineNOP,
        CommandLineError,
        CommandLineVersionRequested,
        CommandLineHelpRequested
    };
    
    struct PowerTraceSeries {
        size_t traceNo;
        QString color;
    };
    
    struct CorrelationTraceSeries {
        size_t matrixNo;
        size_t candidateNo;
        QString color;
    };
    
    Visu(QObject *parent = 0) : QObject(parent), m_display(false), m_save(false), m_filepath(""), m_width(800), m_height(400), m_title(""), m_tracesSet(false), m_traces(""), m_tracesN(0), m_tracesRangeSet(false), m_tracesRange(0), m_tValsSet(false), m_tValues(""), m_correlationsSet(false), m_correlations(""), m_correlationsSetsQ(0), m_correlationsCandidatesK(0), m_samplesPerTrace(0), m_samplesRangeSet(false), m_samplesRange(0.0f), m_plotTVals(false), m_tValsColor("auto"), m_chart(nullptr), m_axisX(nullptr), m_axisYtraces(nullptr), m_axisYcorrs(nullptr), m_axisYtvals(nullptr) {}
    
    /// Parse parameters from the command line and configuration files
    CommandLineParseResult parseCommandLineParams(QCommandLineParser & parser);    

protected:
    
    // Command line arguments
    bool m_display;
    bool m_save;
    QString m_filepath;
    size_t m_width;
    size_t m_height;
    QString m_title;        
    
    bool m_tracesSet;
    QString m_traces;
    size_t m_tracesN;
    bool m_tracesRangeSet;
    double m_tracesRange;
    
    bool m_tValsSet;
    QString m_tValues;
    
    bool m_correlationsSet;
    QString m_correlations;
    size_t m_correlationsSetsQ;
    size_t m_correlationsCandidatesK;
    
    size_t m_samplesPerTrace;
    bool m_samplesRangeSet;
    double m_samplesRange;
    
    // Series arguments
    bool m_plotTVals;
    QString m_tValsColor;
    
    QList<PowerTraceSeries> m_powerTracesToPlot;
    QList<CorrelationTraceSeries> m_correlationTracesToPlot;
    
    // Chart
    QChart * m_chart;
    QValueAxis * m_axisX;    
    QValueAxis * m_axisYtraces;
    QValueAxis * m_axisYcorrs;
    QValueAxis * m_axisYtvals;
    
public slots:
    
    /// Creates a chart based on the parameters set by parseCommandLineParams
    bool createChart();
    /// Save created chart to the specified file
    bool saveChart() const;
    size_t getWidth() const { return m_width; }
    size_t getHeight() const { return m_height; }
    bool shouldDisplay() const { return m_display; }
    bool shouldSave() const { return m_save; }
    QChartView * getChartView() const;
    
signals:
    
    void finished();
    
};

#endif /* VISU_H */

