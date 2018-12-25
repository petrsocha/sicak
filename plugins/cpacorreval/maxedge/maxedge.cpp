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
* \file maxedge.cpp
*
* \brief SICAK CPA correlation matrix evaluation plugin: maximum edge
*
*
* \author Petr Socha
* \version 1.0
*/

#include "maxedge.h"
#include <QString>
#include <cmath>

MaxEdge::MaxEdge() {
    
}

MaxEdge::~MaxEdge() {
    (*this).deInit();
}

QString MaxEdge::getPluginName() {
    return "Maximum correlation trace derivative (param=\"d;sigma\", e.g. param=\"23;8.0\")";
}

QString MaxEdge::getPluginInfo() {
    return "Finds the maximum edge in the correlation traces. Set gaussian parameters: param='d;sigma', e.g. param='23;8.0'";
}

void MaxEdge::init(const char * param) {    
    
    size_t diameter;
    double sigma;
    
    QStringList params = QString(param).split(";");
    if(params.size() == 2){
        diameter = params.at(0).toLongLong();
        sigma = params.at(1).toDouble();
    } else {
        // default vals
        diameter = 23;
        sigma = 8.0f;
    }
    
    m_kernel = generateDerivativeGaussianKernel(diameter, sigma);
    
}

void MaxEdge::deInit() {
    
    
}

void MaxEdge::evaluateCorrelations(MatrixType<double> & correlationMatrix, size_t & sample, size_t & keyCandidate){
    
    if (correlationMatrix.length() < 1) throw RuntimeException("Empty matrix");
    
    Matrix<double> edges = convolveMatrixRows(correlationMatrix, m_kernel);
    
    double max = fabs(edges(0,0));
    size_t maxCol = 0;
    size_t maxRow = 0;		

    for (size_t col = 0; col < edges.cols(); col++) {

            for (size_t row = 0; row < edges.rows(); row++) {

                    if (fabs(edges(col, row)) > max) {

                            max = fabs(edges(col, row));
                            maxCol = col;
                            maxRow = row;

                    }

            }

    }

    sample = maxCol;
    keyCandidate = maxRow;
    
}


Matrix<double> MaxEdge::convolveMatrixRows(MatrixType<double> & matrix, VectorType<double> & kernel) {

        if (matrix.rows() == 0 || matrix.cols() == 0 || kernel.length() == 0) throw RuntimeException("Nothing to convolve");
        if (matrix.cols() < kernel.length()) throw RuntimeException("Convolutional kernel too large");

        Matrix<double> ret(matrix.cols() - kernel.length() + 1, matrix.rows());

        for (size_t row = 0; row < ret.rows(); row++) {

                for (size_t col = 0; col < ret.cols(); col++) {

                        ret(col, row) = 0.0f;

                        for (size_t k = 0; k < kernel.length(); k++) {

                                ret(col, row) += matrix(col + k, row) * kernel(k);

                        }

                }

        }

        return ret;

}


Vector<double> MaxEdge::generateDerivativeGaussianKernel(size_t diameter, double deviation) {

        int newDiameter = (diameter % 2) ? diameter : diameter + 1;

        Vector<double> kernel(newDiameter);

        for (int i = (-1) * ((newDiameter - 1) / 2); i <= (newDiameter - 1) / 2; i++) {

                kernel(i + ((newDiameter - 1) / 2)) = ((double)(i) / (deviation * deviation)) * exp((double)(-1) * ((double)(i*i) / (deviation*deviation)));

        }

        return kernel;

}

