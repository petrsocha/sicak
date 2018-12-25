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
* \brief SICAK CORRelations EValuation text-based UI
*
*
* \author Petr Socha
* \version 1.0
*/

#include <QCoreApplication>
#include <QTextStream>
#include <QCommandLineParser>
#include <QtCore>
#include "correv.h"

int main(int argv, char *args[])
{
    QCoreApplication app(argv, args);
    QCoreApplication::setApplicationName("SICAK CORRelations EValuation");
    QCoreApplication::setApplicationVersion("1.0");
    QCoreApplication::setOrganizationName("Faculty of Information Technology, Czech Technical University in Prague");
    QCoreApplication::setOrganizationDomain("fit.cvut.cz");
        
    QTextStream cerr(stderr);    
    QTextStream cout(stdout);    
    QCommandLineParser parser;    
    
    CorrEv * corrEv = new CorrEv(&app);
    
    QObject::connect(corrEv, SIGNAL(finished()), &app, SLOT(quit()));
     
    switch(corrEv->parseCommandLineParams(parser)) {
        case CorrEv::CommandLineTaskPlanned:
            cout << qPrintable(QCoreApplication::applicationName()) << " " << qPrintable(QCoreApplication::applicationVersion()) << "\n";
            cout.flush();                      
            return app.exec();                  
        case CorrEv::CommandLineError:
            cerr << "Error parsing command line options.\n";
            return 1;
        case CorrEv::CommandLineVersionRequested:
            cout << qPrintable(QCoreApplication::applicationName()) << " " << qPrintable(QCoreApplication::applicationVersion()) << "\n";            
            return 0;
        case CorrEv::CommandLineHelpRequested:
            parser.showHelp();
            return 0;
        case CorrEv::CommandLineQueryRequested:
            corrEv->queryPlugins();
            return 0;
        default: //case CorrEv::CommandLineNOP:
            cout << "Nothing to do.\n";
            return 0;  
    }    
        
}

