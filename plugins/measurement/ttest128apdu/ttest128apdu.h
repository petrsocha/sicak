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
* \file ttest128apdu.h
*
* \brief CPA Measurement scenario plugin: sends N times a command APDU: CLA=0x80, INS=0x60, P1=P2=0x00, Lc=0x10, Le=0x10 with 16 bytes of either random or constant data and receives 16 bytes of ciphertexts back. Makes use of multiple captures per oscilloscope run (e.g. Picoscope's rapid block mode).
*
*
* \author Petr Socha
* \version 1.1
*/

#ifndef TTEST128APDU_H
#define TTEST128APDU_H 

#include <QObject>
#include <QtPlugin>
#include <QString>
#include "measurement.h"
#include "exceptions.hpp"


/**
* \class TTest128APDU
* \ingroup Measurement
*
* \brief CPA Measurement scenario plugin: sends N times a command APDU: CLA=0x80, INS=0x60, P1=P2=0x00, Lc=0x10, Le=0x10 with 16 bytes of either random or constant data and receives 16 bytes of ciphertexts back. Makes use of multiple captures per oscilloscope run (e.g. Picoscope's rapid block mode).
*
*/
class TTest128APDU : public QObject, Measurement {
    
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cz.cvut.fit.Sicak.MeasurementInterface/1.0" FILE "ttest128apdu.json")
    Q_INTERFACES(Measurement)
                
public:
    
    TTest128APDU();
    virtual ~TTest128APDU() override;
    
    virtual QString getPluginName() override;
    virtual QString getPluginInfo() override;
    
    virtual void init(const char * param) override;
    virtual void deInit() override;
    
    virtual void run(const char * measurementId, size_t measurements, Oscilloscope * oscilloscope, CharDevice * charDevice) override;
    
protected:
    int m_channel;
    uint8_t m_cla;
    uint8_t m_ins;    
    
};

#endif /* TTEST128APDU_H */
 
