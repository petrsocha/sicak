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
* \file configloader.hpp
*
* \brief This header file contains configuration loader and parser used by the sicak utilities
*
*
* \author Petr Socha
* \version 1.0
*/


#ifndef CONFIGLOADER_HPP
#define CONFIGLOADER_HPP

#include <QCommandLineParser>
#include <QJsonObject>
#include <QJsonDocument>

/**
* \class ConfigLoader
*
* \brief A QT based command line options loader/parser
*
*/
class ConfigLoader {
    
public:
    
    /// Constructs a ConfigLoader with given QCommandLineParser
    ConfigLoader(const QCommandLineParser & parser): m_parser(parser) {
        
        const QStringList positionalArguments = m_parser.positionalArguments();
        
        foreach (const QString &str, positionalArguments) {
                        
            QFile file;
            file.setFileName(str);
            if(file.open(QIODevice::ReadOnly | QIODevice::Text)){                                
                QString configStr = file.readAll();
                QJsonDocument doc = QJsonDocument::fromJson(configStr.toUtf8());
                m_config_files.push_back(doc.object());
            }
            
        }
        
    }
    
    /// Returns a string parameter value, command-line has priority over json config file
    QString getParam(const QCommandLineOption & option) const {
        
        if(m_parser.isSet(option))
            return m_parser.value(option);
    
        QStringList names = option.names();
        
        foreach (const QString &name, names) {
            
            if(name.length() == 1) continue;
            
            foreach (const QJsonObject &obj, m_config_files) {
                
                if(obj.contains(name) && obj.value(name).isString())
                    return obj.value(name).toString();
                
            }
            
        }
        
        return "";
        
    }

    /// Returns true when parameter is set, either on command line, on in a json config file
    bool isSet(const QCommandLineOption & option) const {
        
        if(m_parser.isSet(option))
        return true;
            
        QStringList names = option.names();
        
        foreach (const QString &name, names) {
            
            if(name.length() == 1) continue;
            
            foreach (const QJsonObject &obj, m_config_files) {
                if(obj.contains(name) && obj.value(name).isString())
                    return true;
            }
                
        }
        
        return false;
        
    }
    
protected:
    
    const QCommandLineParser & m_parser;
    QList<QJsonObject> m_config_files;
    
};

#endif /* CONFIGLOADER_HPP */
