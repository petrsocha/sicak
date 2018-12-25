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
* \file maxcoef.cpp
*
* \brief SICAK CPA correlation matrix evaluation plugin: maximum coefficient
*
*
* \author Petr Socha
* \version 1.0
*/

#include "maxcoef.h"

MaxCoef::MaxCoef() {
    
}

MaxCoef::~MaxCoef() {
    (*this).deInit();
}

QString MaxCoef::getPluginName() {
    return "Maximum correlation coefficient";
}

QString MaxCoef::getPluginInfo() {
    return "Finds the maximum correlation coefficient in the correlation matrix";
}

void MaxCoef::init(const char * param) {    
    
    Q_UNUSED(param)
}

void MaxCoef::deInit() {
    
    
}

void MaxCoef::evaluateCorrelations(MatrixType<double> & correlationMatrix, size_t & sample, size_t & keyCandidate){
    
    if (correlationMatrix.length() < 1) throw RuntimeException("Empty matrix");

    double max = correlationMatrix(0,0);
    size_t maxCol = 0;
    size_t maxRow = 0;		

    for (size_t col = 0; col < correlationMatrix.cols(); col++) {

            for (size_t row = 0; row < correlationMatrix.rows(); row++) {

                    if (correlationMatrix(col, row) > max) {

                            max = correlationMatrix(col, row);
                            maxCol = col;
                            maxRow = row;

                    }

            }

    }

    sample = maxCol;
    keyCandidate = maxRow;
    
}


