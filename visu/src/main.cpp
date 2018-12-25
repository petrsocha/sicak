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
* \brief SICAK VISUalisation text-based UI
*
*
* \author Petr Socha
* \version 1.0
*/

#include <QApplication>
#include <QMainWindow>
#include <QTextStream>
#include <QCommandLineParser>
#include <QtCore>
#include "visu.h"

int main(int argv, char *args[])
{
    QApplication app(argv, args);
    QApplication::setApplicationName("SICAK VISUalisation");
    QApplication::setApplicationVersion("1.0");
    QApplication::setOrganizationName("Faculty of Information Technology, Czech Technical University in Prague");
    QApplication::setOrganizationDomain("fit.cvut.cz");
        
    QTextStream cerr(stderr);    
    QTextStream cout(stdout);    
    QCommandLineParser parser;    
    
    Visu * visu = new Visu(&app);
    
    QObject::connect(visu, SIGNAL(finished()), &app, SLOT(quit()));
         
    switch(visu->parseCommandLineParams(parser)) {
        
        case Visu::CommandLineProcessChart:
            
            cout << qPrintable(QApplication::applicationName()) << " " << qPrintable(QApplication::applicationVersion()) << "\n";
            cout.flush();         
            
            if(!visu->createChart()){
                cerr << "Error creating a chart.\n";
                return 1;
            }
            
            if(visu->shouldSave()){
                if(visu->saveChart()){
                    cout << "File successfully saved.\n";
                    cout.flush();
                } else {  
                    cerr << "Error saving a chart.\n";
                }
            }
            
            if(visu->shouldDisplay()){
                                
                QMainWindow window;
                window.setCentralWidget(visu->getChartView());
                window.resize(visu->getWidth(), visu->getHeight());
                window.show();
                
                return app.exec();
                
            } else {
                return 0;
            }            
            
            
        case Visu::CommandLineError:
            cerr << "Error parsing command line options.\n";
            return 1;
            
        case Visu::CommandLineVersionRequested:
            cout << qPrintable(QApplication::applicationName()) << " " << qPrintable(QApplication::applicationVersion()) << "\n";            
            return 0;
            
        case Visu::CommandLineHelpRequested:
            parser.showHelp();
            return 0;
            
        default: //case Visu::CommandLineNOP:
            cout << "Nothing to do.\n";
            return 0;  
            
    }    
        
}

