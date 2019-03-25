/*
*  SICAK - SIde-Channel Analysis toolKit
*  Copyright (C) 2018-2019 Petr Socha, FIT, CTU in Prague
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
* \file maxabscoef.cpp
*
* \brief SICAK CPA correlation matrix evaluation plugin: maximum absolute coefficient
*
*
* \author Petr Socha
* \version 1.0
*/

#include "maxabscoef.h"
#include <cmath>

MaxAbsCoef::MaxAbsCoef() {
    
}

MaxAbsCoef::~MaxAbsCoef() {
    (*this).deInit();
}

QString MaxAbsCoef::getPluginName() {
    return "Maximum absolute value correlation coefficient";
}

QString MaxAbsCoef::getPluginInfo() {
    return "Finds the maximum absolute value correlation coefficient in the correlation matrix";
}

void MaxAbsCoef::init(const char * param) {    
    
    Q_UNUSED(param)
}

void MaxAbsCoef::deInit() {
    
    
}

void MaxAbsCoef::evaluateCorrelations(MatrixType<double> & correlationMatrix, size_t & sample, size_t & keyCandidate){
    
    if (correlationMatrix.length() < 1) throw RuntimeException("Empty matrix");

    double max = fabs(correlationMatrix(0,0));
    size_t maxCol = 0;
    size_t maxRow = 0;		

    for (size_t col = 0; col < correlationMatrix.cols(); col++) {

            for (size_t row = 0; row < correlationMatrix.rows(); row++) {

                    if (fabs(correlationMatrix(col, row)) > max) {

                            max = fabs(correlationMatrix(col, row));
                            maxCol = col;
                            maxRow = row;

                    }

            }

    }

    sample = maxCol;
    keyCandidate = maxRow;
    
}


