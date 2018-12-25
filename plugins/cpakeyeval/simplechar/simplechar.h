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
* \file simplechar.h
*
* \brief SICAK CPA keyguess evaluation plugin: no transformation, make CPA keyguess the key
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef SIMPLECHAR_H
#define SIMPLECHAR_H 

#include <QObject>
#include <QtPlugin>
#include "cpakeyeval.h"
#include "exceptions.hpp"

/**
* \class SimpleChar
* \ingroup CpaKeyEval
*
* \brief CPA keyguess evaluation SICAK CpaKeyEval plugin, simply creates cipher key from byte-based CPA keyguess
*
*/
class SimpleChar : public QObject, CpaKeyEval {
    
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cz.cvut.fit.Sicak.CpaKeyEvalInterface/1.0" FILE "simplechar.json")
    Q_INTERFACES(CpaKeyEval)
                
public:
    
    SimpleChar();
    virtual ~SimpleChar() override;
    
    virtual QString getPluginName() override;
    virtual QString getPluginInfo() override;
    
    virtual void init(const char * param) override;
    virtual void deInit() override;
    
    virtual Vector<uint8_t> evaluateKeyCandidates(const VectorType<size_t> & keyCandidates) override;  
    
protected:


    
};

#endif /* SIMPLECHAR_H */
 
