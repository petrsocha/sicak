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
* \file aes128back.cpp
*
* \brief SICAK CPA keyguess evaluation plugin: AES-128 keyguess, which should be last round key, gets reversed to the cipher key
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef AES128BACK_H
#define AES128BACK_H 

#include <QObject>
#include <QtPlugin>
#include "cpakeyeval.h"
#include "exceptions.hpp"

/**
* \class Aes128Back
* \ingroup CpaKeyEval
*
* \brief CPA keyguess evaluation SICAK CpaKeyEval plugin, reverses the AES-128 last round key to the cipher key
*
*/
class Aes128Back : public QObject, CpaKeyEval {
    
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cz.cvut.fit.Sicak.CpaKeyEvalInterface/1.0" FILE "aes128back.json")
    Q_INTERFACES(CpaKeyEval)
                
public:
    
    Aes128Back();
    virtual ~Aes128Back() override;
    
    virtual QString getPluginName() override;
    virtual QString getPluginInfo() override;
    
    virtual void init(const char * param) override;
    virtual void deInit() override;
    
    virtual Vector<uint8_t> evaluateKeyCandidates(const VectorType<size_t> & keyCandidates) override;  
    
protected:

    void invKey(unsigned char * key, int round = 0);
    void invKeyRound(unsigned char * key, unsigned char rcon);

    
};

#endif /* AES128BACK_H */
 
