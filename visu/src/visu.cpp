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
* \file visu.cpp
*
* \brief SICAK VISUalisation text-based UI
*
*
* \author Petr Socha
* \version 1.0.1
*/

#include <QtGlobal>
#include <QApplication>
#include <QTextStream>
#include <QCommandLineParser>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QSvgGenerator>

QT_CHARTS_USE_NAMESPACE

#include "configloader.hpp"
#include "global_calls.hpp"
#include "filehandling.hpp"
#include "visu.h"


Visu::CommandLineParseResult Visu::parseCommandLineParams(QCommandLineParser & parser) {
    
    QTextStream cout(stdout);
    QTextStream cerr(stderr);
    
    // Function options            
    const QCommandLineOption displayOption({"D", "display"}, "Display the chart in a graphical window.");
    parser.addOption(displayOption);    
    
    const QCommandLineOption saveOption({"S", "save"}, "Save the chart to file.", "filename");
    parser.addOption(saveOption);    
    
    const QCommandLineOption widthOption({"W", "width"}, "Width of the saved chart.", "positive integer");
    parser.addOption(widthOption);    
    
    const QCommandLineOption heightOption({"H", "height"}, "Height of the saved chart.", "positive integer");
    parser.addOption(heightOption);    
    
    const QCommandLineOption titleOption({"T", "title"}, "Chart title", "string");
    parser.addOption(titleOption);    
    
    const QCommandLineOption tracesOption({"t", "traces"}, "File containing -n traces, each of which containing -s samples (int16).", "filepath");
    parser.addOption(tracesOption);    
    
    const QCommandLineOption tracesNOption({"n", "traces-count"}, "Number of power traces in -t file.", "positive integer");
    parser.addOption(tracesNOption);    
    
    const QCommandLineOption tracesRangeOption({"r", "traces-real-range"}, "Maximum positive value of a power sample in mV, e.g. 2000 for range -2V to +2V.", "positive integer");
    parser.addOption(tracesRangeOption);        
        
    const QCommandLineOption ttestOption({"a", "t-values"}, "File containing -s t-test values (double).", "filepath");
    parser.addOption(ttestOption);    
    
    const QCommandLineOption correlationsOption({"c", "correlations"}, "File containing -q correlation matrices, each of which -s wide and -k tall (double).", "filepath");
    parser.addOption(correlationsOption);    
    
    const QCommandLineOption correlationsQOption({"q", "correlations-sets-count"}, "Number of correlation matrices. E.g. attacking AES-128 key, this value would be 16.", "positive integer");
    parser.addOption(correlationsQOption);    
    
    const QCommandLineOption correlationsKOption({"k", "correlations-candidates-count"}, "Number of key candidates, i.e. rows of correlation matrix. E.g. attacking AES-128 key, this value would be 256.", "positive integer");
    parser.addOption(correlationsKOption);        
    
    const QCommandLineOption samplesOption({"s", "samples-per-trace"}, "Number of samples per trace.", "positive integer");
    parser.addOption(samplesOption);    
    
    const QCommandLineOption sampleRangeOption({"b", "samples-real-range"}, "Time of a single power/correlation trace. Given sampling period T and -s samples, this value would be T*(s-1).", "float number");
    parser.addOption(sampleRangeOption);

    parser.addPositionalArgument("config", "JSON configuration file(s) with Options.");    
                                      
    parser.addPositionalArgument("series", "Time series to plot: e.g. \"t,25,blue\" plots 26th power trace from traces file, \"c,0,255,red\" plots 255th correlation trace from the 1st correlation matrix, \"c,0,all,#bbbbbb\" plots all of them, \"v,pink\" plots t-values from t-values file. Color is optional, otherwise automatically selected.");
    
    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();        
    
    if (!parser.parse(QCoreApplication::arguments())) 
        return CommandLineError;      
    
    if (parser.isSet(versionOption)) return CommandLineVersionRequested;
    if (parser.isSet(helpOption)) return CommandLineHelpRequested;
    
    ConfigLoader cfg(parser);                
    
    m_display = (cfg.isSet(displayOption)) ? true : false;
    m_save = (cfg.isSet(saveOption)) ? true : false;
    
    if(m_save){
        
        m_filepath = cfg.getParam(saveOption);
        
        if(!cfg.isSet(widthOption) || !cfg.isSet(heightOption)){
            cerr << "Width and height must be set when save option is active: -W, -H\n";
            return CommandLineError;
        }
        
        m_width = cfg.getParam(widthOption).toLongLong();
        m_height = cfg.getParam(heightOption).toLongLong();
    }
    
    m_title = (cfg.isSet(titleOption)) ? cfg.getParam(titleOption) : "";        
    
    m_tracesSet = (cfg.isSet(tracesOption)) ? true : false;
    
    if(m_tracesSet){
    
        m_traces = cfg.getParam(tracesOption);
        
        if(!cfg.isSet(tracesNOption) || !cfg.isSet(samplesOption)){
            cerr << "Number of traces and number of samples per trace must be set: -n, -s\n";
            return CommandLineError;
        }
        
        m_tracesN = cfg.getParam(tracesNOption).toLongLong();
        m_samplesPerTrace = cfg.getParam(samplesOption).toLongLong();                                                
            
    }        
    
    m_tValsSet = (cfg.isSet(ttestOption)) ? true : false;
    
    if(m_tValsSet){
     
        m_tValues = cfg.getParam(ttestOption);    
        
        if(!cfg.isSet(samplesOption)){
            cerr << "Number of samples per trace must be set: -s\n";
            return CommandLineError;
        }
            
        m_samplesPerTrace = cfg.getParam(samplesOption).toLongLong();        
        
    }
    
    m_correlationsSet = (cfg.isSet(correlationsOption)) ? true : false;
    
    if(m_correlationsSet){
        
        m_correlations = cfg.getParam(correlationsOption);
        
        if(!cfg.isSet(correlationsQOption) || !cfg.isSet(correlationsKOption) || !cfg.isSet(samplesOption)){
            cerr << "Number of correlation matrices, number of key candidates and number of samples per trace must be set: -q, -k -s\n";
            return CommandLineError;
        }
        
        m_correlationsSetsQ = cfg.getParam(correlationsQOption).toLongLong();
        m_correlationsCandidatesK = cfg.getParam(correlationsKOption).toLongLong();
        m_samplesPerTrace = cfg.getParam(samplesOption).toLongLong();                
        
    }
    
    m_tracesRangeSet = cfg.isSet(tracesRangeOption) ? true : false;
    m_tracesRange = m_tracesRangeSet ? (cfg.getParam(tracesRangeOption).toDouble() / 1000.0) : (double)32768.0;         
    m_samplesRangeSet = cfg.isSet(sampleRangeOption) ? true : false;
    m_samplesRange = m_samplesRangeSet ? cfg.getParam(sampleRangeOption).toDouble() : (double)m_samplesPerTrace;
    
    // parse series arguments
    const QStringList positionalArguments = parser.positionalArguments();
        
    foreach (const QString &argument, positionalArguments) {
        
        QStringList params = QString(argument).split(",");
        
        if(!(params[0].compare("t"))){
            
            // plot a power trace
            
            if(!m_tracesSet){
                cerr << "No power traces file specified: -t\n";
                return CommandLineError;
            }
            
            if(params.size() < 2){
                cerr << "Number of power trace must be specified when plotting a trace: t,0\n";
                return CommandLineError;
            }
            
            PowerTraceSeries traceStr;
            
            traceStr.traceNo = params[1].toLongLong();
            
            if(traceStr.traceNo >= m_tracesN) {
                cerr << "Number of power trace out of range\n";
                return CommandLineError;
            }
                        
            traceStr.color = (params.size() >= 3) ? params[2] : "auto";            
            
            m_powerTracesToPlot.append(traceStr);
            
        } else if(!(params[0].compare("c"))) {
            
            // plot a correlation trace
            
            if(!m_correlationsSet){
                cerr << "No correlation matrices file specified: -c\n";
                return CommandLineError;
            }
            
            if(params.size() < 3){
                cerr << "Number of correlation matrix and number of key candidate must be specified when plotting a correlation trace: c,0,0 or c,0,all,grey\n";
                return CommandLineError;
            }
            
            CorrelationTraceSeries corrTraceStr;
            
            corrTraceStr.matrixNo = params[1].toLongLong();      
            
            if(corrTraceStr.matrixNo >= m_correlationsSetsQ){
                cerr << "Number of correlation matrix out of range\n";
                return CommandLineError;
            }                        
            
            corrTraceStr.color = (params.size() >= 4) ? params[3] : "auto";
            
            if(!(params[2].compare("all"))){
                
                for(size_t candidate = 0; candidate < m_correlationsCandidatesK; candidate++){
                 
                    corrTraceStr.candidateNo = candidate;
                    
                    m_correlationTracesToPlot.append(corrTraceStr);
                    
                }
                
            } else {
                
                corrTraceStr.candidateNo = params[2].toLongLong();
            
                if(corrTraceStr.candidateNo >= m_correlationsCandidatesK){
                    cerr << "Number of key candidate out of range\n";
                    return CommandLineError;
                }
                
                m_correlationTracesToPlot.append(corrTraceStr);
                
            }
            
            
        } else if(!(params[0].compare("v"))) {
            
            // plot a t-test trace
            
            if(!m_tValsSet){
                cerr << "No t-test values specified: -a\n";
                return CommandLineError;
            }
            
            m_plotTVals = true;
            m_tValsColor = (params.size() >= 2) ? params[1] : "auto";
            
            
        }
        
    }
    
    if((!m_plotTVals && m_powerTracesToPlot.size() == 0 && m_correlationTracesToPlot.size() == 0) || (!m_display && !m_save)){
        return CommandLineNOP;
    }
    
    return CommandLineProcessChart;
                    
}

bool Visu::createChart(){
    
    QTextStream cerr(stderr);            
    
    // Prepare chart
    m_chart = new QChart();
    m_chart->legend()->hide();
    if(m_title.size()){
        m_chart->setTitle(m_title);   
    }
    
    // Samples axis
    m_axisX = new QValueAxis();
    if(m_samplesRangeSet){
        m_axisX->setTitleText("Time [s]");        
    } else {
        m_axisX->setTitleText("Samples");        
    }    
    m_axisX->setLabelFormat("%g");
    m_axisX->setRange(0, m_samplesRange);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);    
 
    // Voltage/ADV axis
    if(m_powerTracesToPlot.size()) {
        
        m_axisYtraces = new QValueAxis(); 
        if(m_tracesRangeSet){
            m_axisYtraces->setTitleText("Voltage [V]");
        } else {
            m_axisYtraces->setTitleText("ADC Values");
        }        
        m_axisYtraces->setLabelFormat("%g");
        
        m_chart->addAxis(m_axisYtraces, Qt::AlignLeft);
        
    }
    
    // Correlations axis
    if(m_correlationTracesToPlot.size()){
        
        m_axisYcorrs = new QValueAxis();
        m_axisYcorrs->setTitleText("Pearson correlation coefficient");
        m_axisYcorrs->setLabelFormat("%g");
        
        if(m_powerTracesToPlot.size()){ // axis on the left already exists
            m_chart->addAxis(m_axisYcorrs, Qt::AlignRight);
        } else { // no axis exists yet
            m_chart->addAxis(m_axisYcorrs, Qt::AlignLeft);
        }
        
    }
    
    // t-values axis
    if(m_plotTVals){
        
        if(m_powerTracesToPlot.size() && m_correlationTracesToPlot.size()) { // use correlation traces axis
            
            m_axisYtraces->setTitleText("Pearson correlation coefficient / t-value");
            m_axisYtvals = m_axisYtraces;
            
        } else { // create new axis
            
            m_axisYtvals = new QValueAxis();
            m_axisYtvals->setTitleText("t-value");
            m_axisYtvals->setLabelFormat("%g");
            
            if(m_powerTracesToPlot.size() || m_correlationTracesToPlot.size()){ // axis on the left already exists
                m_chart->addAxis(m_axisYtvals, Qt::AlignRight);
            } else { // no axis exists yet
                m_chart->addAxis(m_axisYtvals, Qt::AlignLeft);
            }
            
        }
        
    }
    
    double sampleInterval = m_samplesRange / (double)m_samplesPerTrace;
        
    // Power traces to plot
    if(m_powerTracesToPlot.size()) {
     
        std::fstream tracesFile;        
        
        try{
            // open power traces file            
            QByteArray ba = m_traces.toLocal8Bit();
            tracesFile = openInFile(ba.data());
        
        } catch (std::exception & e) {
            cerr << "Failed to open power traces file: " << e.what() << "\n";
            return false;
        }
        
        Vector<int16_t> powerTrace;        
        double max = -m_tracesRange;
        double min = m_tracesRange;
        
        foreach (const PowerTraceSeries & serie, m_powerTracesToPlot) {
                    
            try {
                powerTrace = loadPowerTraceFromFile<int16_t>(tracesFile, m_samplesPerTrace, serie.traceNo);
            } catch (std::exception & e) {
                cerr << "Failed to read the power trace: " << e.what() << "\n";
                return false;
            }                        
            
            QLineSeries * series = new QLineSeries();
            
            for(size_t i = 0; i < m_samplesPerTrace; i++) {                
                // normalize samples to 0..1, recompute for set range
                double val = ((powerTrace(i)+32768.0f)/(2.0f*32768.0f))* (2.0*m_tracesRange) - (m_tracesRange);
                if(val > max) max = val;
                if(val < min) min = val;
                series->append(i*sampleInterval, val); 
            }
                
            if(serie.color.compare("auto") != 0) {
                series->setColor(serie.color);
            }                
            
            m_chart->addSeries(series);   
            series->attachAxis(m_axisX);
            series->attachAxis(m_axisYtraces);            
            
        }                
        
        m_axisYtraces->setRange(min, max);
        m_axisYtraces->applyNiceNumbers();
        
        closeFile(tracesFile);
    
    }
    
    if(m_correlationTracesToPlot.size()){
        
        std::fstream corrsFile;        
        
        try{
            // open power traces file            
            QByteArray ba = m_correlations.toLocal8Bit();
            corrsFile = openInFile(ba.data());
        
        } catch (std::exception & e) {
            cerr << "Failed to open correlations file: " << e.what() << "\n";
            return false;
        }
        
        Vector<double> correlationTrace;        
        double max = -1.0;
        double min = 1.0;
    
        foreach (const CorrelationTraceSeries & serie, m_correlationTracesToPlot) {
            
            try {
                correlationTrace = loadCorrelationTraceFromFile<double>(corrsFile, m_samplesPerTrace, m_correlationsCandidatesK, serie.matrixNo, serie.candidateNo);
            } catch (std::exception & e) {
                cerr << "Failed to read the correlation trace: " << e.what() << "\n";
                return false;
            }
            
            QLineSeries * series = new QLineSeries();
            
            for(size_t i = 0; i < m_samplesPerTrace; i++) {  
                if(correlationTrace(i) > max) max = correlationTrace(i);
                if(correlationTrace(i) < min) min = correlationTrace(i);
                series->append(i*sampleInterval, correlationTrace(i)); 
            }
                
            if(serie.color.compare("auto") != 0) {
                series->setColor(serie.color);
            }
                
            m_chart->addSeries(series);
            series->attachAxis(m_axisX);
            series->attachAxis(m_axisYcorrs);       
            
        }
        
        m_axisYcorrs->setRange(min, max);
        m_axisYcorrs->applyNiceNumbers();
    
        closeFile(corrsFile);
    
    }
    
    if(m_plotTVals){
     
        std::fstream tValsFile;
        
        try{
            // open power traces file            
            QByteArray ba = m_tValues.toLocal8Bit();
            tValsFile = openInFile(ba.data());
        
        } catch (std::exception & e) {
            cerr << "Failed to open correlations file: " << e.what() << "\n";
            return false;
        }
        
        Vector<double> tValsTrace;
        
        try {
            tValsTrace = loadTValuesFromFile<double>(tValsFile, m_samplesPerTrace);
        } catch (std::exception & e) {
            cerr << "Failed to read the t-values trace: " << e.what() << "\n";
            return false;
        }
        
        QLineSeries * series = new QLineSeries();
        
        for(size_t i = 0; i < m_samplesPerTrace; i++) {                
            series->append(i*sampleInterval, tValsTrace(i)); 
        }
            
        if(m_tValsColor.compare("auto") != 0) {
            series->setColor(m_tValsColor);
        }
            
        m_chart->addSeries(series);
        series->attachAxis(m_axisX);
        series->attachAxis(m_axisYtvals);
        
        m_axisYtvals->applyNiceNumbers();
        
        closeFile(tValsFile);
    }        
    
    return true;
    
}

bool Visu::saveChart() const {
    
    if(!m_save) return true;
    
    QChartView * chartView = new QChartView(m_chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->resize(m_width, m_height);
    
    if(m_filepath.endsWith(".svg")){
        
        // vector
        
        QSvgGenerator generator;
        generator.setFileName(m_filepath);
        generator.setSize(chartView->size());
        generator.setViewBox(chartView->rect());
        generator.setTitle(m_title);
        dynamic_cast<QWidget *>(chartView)->render( &generator );
        
        return true;
        
    } else {
        
        // bitmap
        
        QPixmap p(chartView->size());
        dynamic_cast<QWidget *>(chartView)->render( &p );
        
        return p.save(m_filepath);
    
    }
    
}

QChartView * Visu::getChartView() const {
 
    QChartView * chartView = new QChartView(m_chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    
    return chartView;
    
}
