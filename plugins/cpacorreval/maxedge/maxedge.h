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
* \file maxedge.h
*
* \brief SICAK CPA correlation matrix evaluation plugin: maximum edge
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef MAXEDGE_H
#define MAXEDGE_H 

#include <QObject>
#include <QtPlugin>
#include "cpacorreval.h"
#include "exceptions.hpp"

/**
* \class MaxEdge
* \ingroup CpaCorrEval
*
* \brief CPA correlation matrix evaluation SICAK CpaCorrEval plugin, looking for maximum edge, 
* i.e. largest derivation of correlation trace, approximated by convolving with derivative of gaussian
*
*/
class MaxEdge : public QObject, CpaCorrEval {
    
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cz.cvut.fit.Sicak.CpaCorrEvalInterface/1.0" FILE "maxedge.json")
    Q_INTERFACES(CpaCorrEval)
                
public:
    
    MaxEdge();
    virtual ~MaxEdge() override;
    
    virtual QString getPluginName() override;
    virtual QString getPluginInfo() override;
    
    /// Initialize the plugin, prepare convolutional kernel for further evaluations (first derivative of gaussian) based on parameters: param="d,sigma", e.g. "11,5.0"
    virtual void init(const char * param) override;
    virtual void deInit() override;
    
    virtual void evaluateCorrelations(MatrixType<double> & correlationMatrix, size_t & sample, size_t & keyCandidate) override;    

protected:
    
    /// Convolve rows of matrix
    Matrix<double> convolveMatrixRows(MatrixType<double> & matrix, VectorType<double> & kernel);
    /// Generate derivative of gaussian kernel, which works as edge detector
    Vector<double> generateDerivativeGaussianKernel(size_t diameter, double deviation);
    
    Vector<double> m_kernel;
    
};

#endif /* MAXEDGE_H */
 
