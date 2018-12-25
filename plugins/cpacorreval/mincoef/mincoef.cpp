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
* \file mincoef.cpp
*
* \brief SICAK CPA correlation matrix evaluation plugin: minimum coefficient
*
*
* \author Petr Socha
* \version 1.0
*/

#include "mincoef.h"

MinCoef::MinCoef() {
    
}

MinCoef::~MinCoef() {
    (*this).deInit();
}

QString MinCoef::getPluginName() {
    return "Minimum correlation coefficient";
}

QString MinCoef::getPluginInfo() {
    return "Finds the minimum correlation coefficient in the correlation matrix";
}

void MinCoef::init(const char * param) {    
    
    Q_UNUSED(param)
}

void MinCoef::deInit() {
    
    
}

void MinCoef::evaluateCorrelations(MatrixType<double> & correlationMatrix, size_t & sample, size_t & keyCandidate){
    
    if (correlationMatrix.length() < 1) throw RuntimeException("Empty matrix");

    double min = correlationMatrix(0,0);
    size_t minCol = 0;
    size_t minRow = 0;		

    for (size_t col = 0; col < correlationMatrix.cols(); col++) {

            for (size_t row = 0; row < correlationMatrix.rows(); row++) {

                    if (correlationMatrix(col, row) < min) {

                            min = correlationMatrix(col, row);
                            minCol = col;
                            minRow = row;

                    }

            }

    }

    sample = minCol;
    keyCandidate = minRow;
    
}

