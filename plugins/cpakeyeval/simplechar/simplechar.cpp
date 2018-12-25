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
* \file simplechar.cpp
*
* \brief SICAK CPA keyguess evaluation plugin: no transformation, make CPA keyguess the key
*
*
* \author Petr Socha
* \version 1.0
*/

#include "simplechar.h"

SimpleChar::SimpleChar() {
    
}

SimpleChar::~SimpleChar() {
    (*this).deInit();

}

QString SimpleChar::getPluginName() {
    return "Simple key evaluation for byte-based CPA: no transformation after correlation evaluation (e.g. AES first round)";
}

QString SimpleChar::getPluginInfo() {
    return "Simple key evaluation for byte-based CPA: no transformation after correlation evaluation (e.g. AES first round)";
}

void SimpleChar::init(const char * param) {    
    Q_UNUSED(param)
   
}

void SimpleChar::deInit() {
    
 
}

Vector<uint8_t> SimpleChar::evaluateKeyCandidates(const VectorType<size_t> & keyCandidates) {

    Vector<uint8_t> ret(keyCandidates.length());
    
    for(size_t byte = 0; byte < ret.length(); byte++){
     
        ret(byte) = (uint8_t) keyCandidates(byte);
                
    }
    
    return ret;
    
}
