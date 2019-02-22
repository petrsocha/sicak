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
* \file ttest128apdu.cpp
*
* \brief SICAK CPA measurement scenario plugin: sends key command (0x01), followed by the cipher key, followed by N times {encrypt command (0x02) followed by 16 bytes of either random or constant data}. Makes use of multiple captures per oscilloscope run (e.g. Picoscope's rapid block mode).
*
*
* \author Petr Socha
* \version 1.1
*/

#include <QString>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <random>
#include "ttest128apdu.h"
#include "filehandling.hpp"
#include "global_calls.hpp"

TTest128APDU::TTest128APDU(): m_channel(1), m_cla(0x80), m_ins(0x60) {
    
}

TTest128APDU::~TTest128APDU(){
    
}

QString TTest128APDU::getPluginName() {
    return "AES-128 t-test (APDU oriented)";
}

QString TTest128APDU::getPluginInfo() {
    return "Sends N times a command APDU: CLA=0x80, INS=0x60, P1=P2=0x00, Lc=0x10, Le=0x10 with 16 bytes of either random or constant data, receives Response APDUs with 16 bytes of ciphertext back, and captures the power consumption.";
}

void TTest128APDU::init(const char * param){
    
    QStringList params = QString(param).split(";");
    QString paramVal;
    
    for (int i = 0; i < params.size(); ++i){ // iterate thru all parameters
        
         if(params.at(i).startsWith("ch=")){
             
             paramVal = params.at(i);
             paramVal.remove(0,3);
             
             m_channel = paramVal.toInt();
             if(m_channel < 0) throw RuntimeException("Invalid measurement channel param");                          
             
         } else if(params.at(i).startsWith("cla=")){
             
             paramVal = params.at(i);
             paramVal.remove(0,4);
             
             bool ok;
             m_cla = paramVal.toInt(&ok, 16);
             
             if(!ok) throw RuntimeException("Failed parsing hex CLA param");
             
         } else if(params.at(i).startsWith("ins=")){
             
             paramVal = params.at(i);
             paramVal.remove(0,4);
             
             bool ok;
             m_ins = paramVal.toInt(&ok, 16);
             
             if(!ok) throw RuntimeException("Failed parsing hex INS param");
             
         }
         
    }
    
}

void TTest128APDU::deInit(){
    
}

void TTest128APDU::run(const char * measurementId, size_t measurements, Oscilloscope * oscilloscope, CharDevice * charDevice){
    
    if(oscilloscope == nullptr || charDevice == nullptr){
        throw RuntimeException("Oscilloscope and character device are needed to run this measurement.");
    }
    
    size_t samplesPerTrace;
    size_t capturesPerRun;
    
    oscilloscope->getCurrentSetup(samplesPerTrace, capturesPerRun); //< Retrieve oscilloscope setup    
    
    // Check the input params
    if(measurements < capturesPerRun){
        throw InvalidInputException("Oscilloscope and measurement parameter mismatch: number of measurements must be greater or equal to number of oscilloscope captures");
    } else if(measurements % capturesPerRun){                
        throw InvalidInputException("Oscilloscope and measurement parameter mismatch: number of measurements must be divisible by the number of oscilloscope captures without remainder");
    }        
    
    // Print intro info
    QTextStream cout(stdout);
    
    cout << QString("Downloading power traces from channel %1\nUsing APDU with CLA=0x%2 and INS=0x%3\n").arg(m_channel).arg(m_cla, 2, 16, QChar('0')).arg(m_ins, 2, 16, QChar('0'));
    
    cout.flush();
    
    // Begin progress bar
    CoutProgress::get().start(measurements);
        
    // Alloc space
    Matrix<uint8_t> plaintext(16, measurements);
    Matrix<uint8_t> ciphertext(16, measurements);
    PowerTraces<int16_t> measuredTraces(samplesPerTrace, measurements); 
    Vector<uint8_t> commandAPDU(16+6); // 6 bytes APDU fields + 16 bytes of AES plaintext
    Vector<uint8_t> responseAPDU(16+2); // 2 bytes APDU fields + 16 bytes of AES plaintext 
    
    // Constant vs random traces array
    Vector<uint8_t> isTraceConstant(measurements);
    
    // Filenames
    QString randTracesFilename = "random-traces-";
    randTracesFilename.append(measurementId);
    randTracesFilename.append(".bin");
    QString constTracesFilename = "constant-traces-";
    constTracesFilename.append(measurementId);
    constTracesFilename.append(".bin");
    QString plaintextFilename = "plaintext-";
    plaintextFilename.append(measurementId);
    plaintextFilename.append(".bin");
    QString ciphertextFilename = "ciphertext-";
    ciphertextFilename.append(measurementId);
    ciphertextFilename.append(".bin");
    
    QByteArray ba;
    
    // Open files
    ba = randTracesFilename.toLocal8Bit();
    std::fstream randomTracesFile = openOutFile(ba.data());
    ba = constTracesFilename.toLocal8Bit();
    std::fstream constTracesFile = openOutFile(ba.data());
    
    ba = plaintextFilename.toLocal8Bit();
    std::fstream plaintextFile = openOutFile(ba.data());
    ba = ciphertextFilename.toLocal8Bit();
    std::fstream ciphertextFile = openOutFile(ba.data());
    
    // Prepare the command APDU
    commandAPDU(0) = m_cla; // CLA
    commandAPDU(1) = m_ins; // INS
    commandAPDU(2) = 0x00; // P1
    commandAPDU(3) = 0x00; // P2
    commandAPDU(4) = 0x10; // Lc
    // commandAPDU(5:20) = <random data>
    commandAPDU(21) = 0x10; // Le
    
    uint8_t constPlaintext[16] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };    
    
    // Random number generation
    std::random_device trng;
    std::mt19937 prng(trng());
    std::uniform_int_distribution<std::mt19937::result_type> byteUnif(0, 255);
    std::uniform_int_distribution<std::mt19937::result_type> bitUnif(0, 1);
    
    // Measure
    size_t runs = measurements / capturesPerRun; //< Number of independent oscilloscope runs
    
    // We run the oscilloscope runs times
    for(size_t run = 0; run < runs; run++){
        
        try { // try to perform a measurement run
        
            oscilloscope->run(); //< Start capturing capturesPerRun captures
            
            // Send capturesPerRun blocks to cipher
            for(size_t capture = 0; capture < capturesPerRun; capture++){                     
                
                size_t measurement = run * capturesPerRun + capture; //< Number of current measurement
                
                isTraceConstant(measurement) = (uint8_t) bitUnif(prng) % 2; //< Decide whatever next measurement will be random or constant                        
                
                if(isTraceConstant(measurement)){
                
                    // Use constant plaintext
                    for(int byte = 0; byte < 16; byte++){
                        
                        plaintext(byte, measurement) = constPlaintext[byte];
                                
                    }   
                    
                } else {
                    
                    // Generate random plaintext
                    for(int byte = 0; byte < 16; byte++){
                        
                        plaintext(byte, measurement) = (uint8_t) byteUnif(prng);
                                
                    }                                
                    
                }                                                
                
                // Send plaintext
                
                // fill APDU with plaintext
                for(int byte = 0; byte < 16; byte++){
                    commandAPDU(5+byte) = plaintext(byte, measurement);
                }
                // send APDU
                charDevice->send(commandAPDU);
                            
                
                // Receive ciphertext

                // receive APDU
                if(charDevice->receive(responseAPDU) != 18) throw RuntimeException("Failed to receive 18 bytes APDU response (16 bytes ciphertext + SW1 + SW2).");
                // copy ciphertext
                for(int byte = 0; byte < 16; byte++){
                    ciphertext(byte, measurement) = responseAPDU(byte);
                }
                
                
                CoutProgress::get().update(measurement);
                
            }
            
            size_t measuredSamples;
            size_t measuredCaptures;
            
            // Download the sampled data from oscilloscope
            oscilloscope->getValues(m_channel, &( measuredTraces(0, run * capturesPerRun) ), capturesPerRun * samplesPerTrace, measuredSamples, measuredCaptures);
            
            if(measuredSamples != samplesPerTrace || measuredCaptures != capturesPerRun){
                throw RuntimeException("Measurement went wrong: samples*captures mismatch");
            }
            
        } catch (std::exception & e){ // an oscilloscope run or communication with the target failed            
            
            cout << QString("\n[!] An error has occured during the %1. oscilloscope run: %2\n").arg(run+1).arg(e.what());                                    
            cout << QString("[!] Before an error, %1 power traces were measured and will be saved.\n").arg(run * capturesPerRun);
            cout.flush();
            
            measurements = run * capturesPerRun; // update the number of succesfully performed measurements                             
            
            // No need to shrink the containers here, since every single measurement is written to file separately
            
            break; // break the measurement
            
        }
        
    }
    
    size_t randomTracesN = 0;
    size_t constTracesN = 0;    
    
    // Write data to files
    for(size_t i = 0; i < measurements; i++){
     
        if(isTraceConstant(i)){
            
            constTracesN++;
            writeArrayToFile(constTracesFile, &( measuredTraces(0, i) ), samplesPerTrace);
            
        } else {
            
            randomTracesN++;
            writeArrayToFile(randomTracesFile, &( measuredTraces(0, i) ), samplesPerTrace);
            writeArrayToFile(plaintextFile, &( plaintext(0, i) ), 16);
            writeArrayToFile(ciphertextFile, &( ciphertext(0, i) ), 16);
            
        }
        
    }
    
    // Close files
    closeFile(randomTracesFile);
    closeFile(constTracesFile);
    closeFile(plaintextFile);
    closeFile(ciphertextFile);
    
    CoutProgress::get().finish();
    
    // Flush config to json files
    QJsonObject tracesConf;
    tracesConf["random-traces"] = randTracesFilename;
    tracesConf["random-traces-count"] = QString::number(randomTracesN);
    tracesConf["constant-traces"] = constTracesFilename;
    tracesConf["constant-traces-count"] = QString::number(constTracesN);
    tracesConf["samples-per-trace"] = QString::number(samplesPerTrace);
    tracesConf["blocks-count"] = QString::number(randomTracesN);    
    tracesConf["blocks-length"] = QString::number(16);
    QJsonDocument tracesDoc(tracesConf);
    QString tracesDocFilename = measurementId;
    tracesDocFilename.append(".json");
    QFile tracesDocFile(tracesDocFilename);
    if(tracesDocFile.open(QIODevice::WriteOnly)){
        tracesDocFile.write(tracesDoc.toJson());
    }
    
    cout << QString("Measured %1 power traces in total, %8 samples per trace,\n%2 random data based power traces were saved to '%3',\n%4 constant data based power traces were saved to '%5'.\nRandom plaintext blocks were saved to '%6', related ciphertext blocks were saved to '%7'.\n").arg(measurements).arg(randomTracesN).arg(randTracesFilename).arg(constTracesN).arg(constTracesFilename).arg(plaintextFilename).arg(ciphertextFilename).arg(samplesPerTrace);
    
}
