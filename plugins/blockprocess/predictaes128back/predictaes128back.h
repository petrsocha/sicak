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
* \file predictaes128back.h
*
* \brief SICAK block data processing plugin: power predictions for AES-128 based on ciphertext and last round working register Hamming distance
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef PREDICTAES128BACK_H
#define PREDICTAES128BACK_H 

#include <QObject>
#include <QtPlugin>
#include "blockprocess.h"
#include "exceptions.hpp"

/**
* \class PredictAES128Back
* \ingroup BlockProcess
*
* \brief AES-128 last round working register Hamming distance power predictions SICAK BlockProcess plugin
*
*/
class PredictAES128Back : public QObject, BlockProcess {
    
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cz.cvut.fit.Sicak.BlockProcessInterface/1.0" FILE "predictaes128back.json")
    Q_INTERFACES(BlockProcess)
                
public:
    
    PredictAES128Back();
    virtual ~PredictAES128Back() override;
    
    virtual QString getPluginName() override;
    virtual QString getPluginInfo() override;
    
    virtual void init(const char * param) override;
    virtual void deInit() override;
    
    /// Creates new file containing power predictions based on data matrix, 16 cols wide and N rows tall
    virtual void processBlockData(MatrixType<uint8_t> & data, const char * id) override;    
    
protected:


    
};

#endif /* PREDICTAES128BACK_H */
 
