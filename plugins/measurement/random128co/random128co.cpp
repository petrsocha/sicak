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
* \file random128co.cpp
*
* \brief SICAK CPA measurement scenario plugin: sends key command (0x01), followed by the cipher key, followed by N times {encrypt command (0x02) followed by 16 bytes of random data}. Makes use of multiple captures per oscilloscope run (e.g. Picoscope's rapid block mode).
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
#include "random128co.h"
#include "filehandling.hpp"
#include "global_calls.hpp"

Random128CO::Random128CO(): m_channel(1) {
    
}

Random128CO::~Random128CO(){
    
}

QString Random128CO::getPluginName() {
    return "AES-128 random (command oriented)";
}

QString Random128CO::getPluginInfo() {
    return "Sends 0x01 followed by cipher key, then N times {0x02 followed by 128 bits of random data}, receives back every cipher text, and captures the power consumption.";
}

void Random128CO::init(const char * param){
    
    QStringList params = QString(param).split(";");
    QString paramVal;
    
    for (int i = 0; i < params.size(); ++i){ // iterate thru all parameters
        
         if(params.at(i).startsWith("ch=")){
             
             paramVal = params.at(i);
             paramVal.remove(0,3);
             
             m_channel = paramVal.toInt();
             if(m_channel < 0) throw RuntimeException("Invalid measurement channel param");                          
             
         } 
         
    }        
    
}

void Random128CO::deInit(){
    
}

void Random128CO::run(const char * measurementId, size_t measurements, Oscilloscope * oscilloscope, CharDevice * charDevice){
    
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
    
    cout << QString("Downloading power traces from channel %1\n").arg(m_channel);
    cout.flush();
    
    // Begin progress bar
    CoutProgress::get().start(measurements);
        
    // Alloc space
    Matrix<uint8_t> plaintext(16, measurements);
    Matrix<uint8_t> ciphertext(16, measurements);
    PowerTraces<int16_t> measuredTraces(samplesPerTrace, measurements); 
    Vector<uint8_t> command(1); 
    
    QString tracesFilename = "random-traces-";
    tracesFilename.append(measurementId);
    tracesFilename.append(".bin");
    QString plaintextFilename = "plaintext-";
    plaintextFilename.append(measurementId);
    plaintextFilename.append(".bin");
    QString ciphertextFilename = "ciphertext-";
    ciphertextFilename.append(measurementId);
    ciphertextFilename.append(".bin");
    
    QByteArray ba;
    
    // Open files
    ba = tracesFilename.toLocal8Bit();
    std::fstream tracesFile = openOutFile(ba.data());
    ba = plaintextFilename.toLocal8Bit();
    std::fstream plaintextFile = openOutFile(ba.data());
    ba = ciphertextFilename.toLocal8Bit();
    std::fstream ciphertextFile = openOutFile(ba.data());
    
    // Setup the device
    command(0) = 0x01; //< "Set key" command
    charDevice->send(command); //< Send the command
    
    uint8_t key[16] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
    charDevice->send(key, 16); //< Send the key
    
    // Random number generation
    std::random_device trng;
    std::mt19937 prng(trng());
    std::uniform_int_distribution<std::mt19937::result_type> byteUnif(0, 255);
    
    // Measure
    size_t runs = measurements / capturesPerRun; //< Number of independent oscilloscope runs
    command(0) = 0x02; //< "Encryption" command        
    
    // We run the oscilloscope runs times
    for(size_t run = 0; run < runs; run++){
        
        try { // try to perform a measurement run
        
            oscilloscope->run(); //< Start capturing capturesPerRun captures
            
            // Send capturesPerRun blocks to cipher
            for(size_t capture = 0; capture < capturesPerRun; capture++){                     
                
                size_t measurement = run * capturesPerRun + capture; //< Number of current measurement
                
                // Generate random plaintext
                for(int byte = 0; byte < 16; byte++){
                    
                    plaintext(byte, measurement) = (uint8_t) byteUnif(prng);
                            
                }
                
                // Send plaintext
                charDevice->send(command); //< Send the encryption command
                charDevice->send( &( plaintext(0, measurement) ), 16); //< Send 16 bytes of plaintext //TODO MatrixRowPtr
                
                // Receive ciphertext
                charDevice->receive( &( ciphertext(0, measurement) ), 16); //< Receive 16 bytes of ciphertext //TODO MatrixRowPtr
                
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
            
            // Shrink the containers to fit the data actually measured
            measuredTraces.shrinkRows(measurements);
            plaintext.shrinkRows(measurements);
            ciphertext.shrinkRows(measurements);
            
            break; // break the measurement
            
        }
        
    }
    
    // Write data to files
    writeArrayToFile(tracesFile, measuredTraces);
    writeArrayToFile(plaintextFile, plaintext);
    writeArrayToFile(ciphertextFile, ciphertext);
    
    // Close files
    closeFile(tracesFile);
    closeFile(plaintextFile);
    closeFile(ciphertextFile);
    
    CoutProgress::get().finish();
    
    // Flush config to json files
    QJsonObject tracesConf;
    tracesConf["random-traces"] = tracesFilename;
    tracesConf["random-traces-count"] = QString::number(measurements);
    tracesConf["samples-per-trace"] = QString::number(samplesPerTrace);
    tracesConf["blocks"] = plaintextFilename;
    tracesConf["blocks"] = ciphertextFilename;
    tracesConf["blocks-count"] = QString::number(measurements);    
    tracesConf["blocks-length"] = QString::number(16);
    QJsonDocument tracesDoc(tracesConf);
    QString tracesDocFilename = measurementId;
    tracesDocFilename.append(".json");
    QFile tracesDocFile(tracesDocFilename);
    if(tracesDocFile.open(QIODevice::WriteOnly)){
        tracesDocFile.write(tracesDoc.toJson());
    }
    
    cout << QString("Measured %1 power traces, %5 samples per trace, and saved them to '%2'.\nUsed plaintext blocks were saved to '%3', retrieved ciphertext blocks were saved to '%4'.\n").arg(measurements).arg(tracesFilename).arg(plaintextFilename).arg(ciphertextFilename).arg(samplesPerTrace);
    
}
