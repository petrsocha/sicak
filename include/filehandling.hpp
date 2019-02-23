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
* \file filehandling.hpp
*
* \brief This header file contains function templates for loading/storing defined types
*
*
* \author Petr Socha
* \version 1.1
*/


#ifndef FILEHANDLING_HPP
#define FILEHANDLING_HPP

#include <fstream>
#include <string>
#include <cstring>
#include "types_basic.hpp"
#include "types_power.hpp"
#include "types_stat.hpp"
#include "exceptions.hpp"
#include <iostream>

/**
*
* \brief Opens filestream for writing
* \ingroup SicakData
*
*/
std::fstream openOutFile(const char * filename){    
    
    std::fstream fs(filename, std::ios_base::binary | std::ios_base::out);
    
    if (fs.fail())
        throw RuntimeException("Could not open the file. Wrong filename or permissions?");
    
    return fs;
    
}

/**
*
* \brief Opens filestream for reading
* \ingroup SicakData
*
*/
std::fstream openInFile(const char * filename){    
    
    std::fstream fs(filename, std::ios_base::binary | std::ios_base::in);
    
    if (fs.fail())
        throw RuntimeException("Could not open the file. Wrong filename or permissions?");
    
    return fs;
    
}

/**
*
* \brief Flushes and closes the filestream
* \ingroup SicakData
*
*/
void closeFile(std::fstream & fs){

    fs.flush();
    
    if (fs.fail())
        throw RuntimeException("Failed to flush the ouput buffers on file.");
    
    fs.close();
    
}

/**
*
* \brief Fills array from file, based on the given Array's size
* \ingroup SicakData
*
*/
template<class T>
void fillArrayFromFile(std::fstream & fs, ArrayType<T> & arr){
    
    fs.read(reinterpret_cast<char *>(arr.data()), arr.size());
    
    if(fs.fail())
        throw RuntimeException("Could not read the data from the file. Not enough data?");
            
}

/**
*
* \brief Loads a power trace from file, based on parameters given
* \ingroup SicakData
*
*/
template<class T>
Vector<T> loadPowerTraceFromFile(std::fstream & fs, size_t samplesPerTrace, size_t trace){     
    
    fs.seekg(sizeof(T) * samplesPerTrace * trace); 

    if(fs.fail())
        throw RuntimeException("Could not skip offset. Not enough data?");
    
    Vector<T> arr;
    arr.init(samplesPerTrace);    
    
    fillArrayFromFile(fs, arr);
    
    return arr;
    
}

/**
*
* \brief Loads a correlation trace from file, based on parameters given
* \ingroup SicakData
*
*/
template<class T>
Vector<T> loadCorrelationTraceFromFile(std::fstream & fs, size_t samplesPerTrace, size_t noOfCandidates, size_t matrix, size_t candidate){
 
    fs.seekg(sizeof(T) * samplesPerTrace * noOfCandidates * matrix + sizeof(T) * samplesPerTrace * candidate); 

    if(fs.fail())
        throw RuntimeException("Could not skip offset. Not enough data?");
    
    Vector<T> arr;
    arr.init(samplesPerTrace);
    
    fillArrayFromFile(fs, arr);
    
    return arr;
    
}

/**
*
* \brief Loads a t-values trace from file, based on parameters given
* \ingroup SicakData
*
*/
template<class T>
Vector<T> loadTValuesFromFile(std::fstream & fs, size_t samplesPerTrace){
 
    fs.seekg(0); 

    if(fs.fail())
        throw RuntimeException("Could not skip offset. Not enough data?");
    
    Vector<T> arr;
    arr.init(samplesPerTrace);
    
    fillArrayFromFile(fs, arr);
    
    return arr;
    
}

/**
*
* \brief Writes array to file
* \ingroup SicakData
*
*/
template<class T>
void writeArrayToFile(std::fstream & fs, const T * buffer, size_t len){
    
    fs.write(reinterpret_cast<const char *>(buffer), len * sizeof(T));
    
    if(fs.fail())
        throw RuntimeException("Could not write the data to the file. Not enough space?");
            
}

/**
*
* \brief Writes array to file
* \ingroup SicakData
*
*/
template<class T>
void writeArrayToFile(std::fstream & fs, const ArrayType<T> & arr){
    
    fs.write(reinterpret_cast<const char *>(arr.data()), arr.size());
    
    if(fs.fail())
        throw RuntimeException("Could not write the data to the file. Not enough space?");
            
}

/**
*
* \brief Reads context from file, based on the context's file format
* \ingroup SicakData
*
*/
template<class T>
Moments2DContext<T> readContextFromFile(std::fstream & fs){    
    
    // Read and compare ID signature
    Vector<uint8_t> ctxIdAttr(256);
    
    fillArrayFromFile(fs, ctxIdAttr);
    
    if(strcmp(reinterpret_cast<char *>(ctxIdAttr.data()), "cz.cvut.fit.Sicak.Moments2DContext/1.1")) throw RuntimeException("Error reading a context from a file: invalid ID signature. Maybe incompatible version?");
        
    // Read size parameters
    Vector<uint64_t> ctxSizeAttrs(9);
    
    fillArrayFromFile(fs, ctxSizeAttrs);
        
    Moments2DContext<T> ret(ctxSizeAttrs(0), ctxSizeAttrs(1), ctxSizeAttrs(2), ctxSizeAttrs(3), ctxSizeAttrs(4), ctxSizeAttrs(5), ctxSizeAttrs(6));
    ret.p1Card() = ctxSizeAttrs(7);
    ret.p2Card() = ctxSizeAttrs(8);
    
    // Read the data
    for(size_t order = 1; order <= ret.p1MOrder(); order++){
        fillArrayFromFile(fs, ret.p1M(order));
    }
    
    for(size_t order = 1; order <= ret.p2MOrder(); order++){
        fillArrayFromFile(fs, ret.p2M(order));
    }
        
    for(size_t order = 2; order <= ret.p1CSOrder(); order++){
        fillArrayFromFile(fs, ret.p1CS(order));
    }
    
    for(size_t order = 2; order <= ret.p2CSOrder(); order++){
        fillArrayFromFile(fs, ret.p2CS(order));
    }
    
    for(size_t order = 1; order <= ret.p12ACSOrder(); order++){
        fillArrayFromFile(fs, ret.p12ACS(order));
    }
    
    return ret;
    
}

/**
*
* \brief Writes context to file
* \ingroup SicakData
*
*/
template<class T>
void writeContextToFile(std::fstream & fs, const Moments2DContext<T> & ctx){
     
    // Write ID signature
    const char * ctxId = ctx.getId();            
    if(strlen(ctxId) > 255) throw RuntimeException("Context ID overflow.");
    
    Vector<uint8_t> ctxIdAttr(256, 0);
    for(size_t i = 0; i < strlen(ctxId); i++){
        ctxIdAttr(i) = ctxId[i];
    }
    
    writeArrayToFile(fs, ctxIdAttr);      
    
    // Write size attributes
    Vector<uint64_t> ctxSizeAttrs(9);
    ctxSizeAttrs(0) = ctx.p1Width();
    ctxSizeAttrs(1) = ctx.p2Width();
    ctxSizeAttrs(2) = ctx.p1MOrder();
    ctxSizeAttrs(3) = ctx.p2MOrder();
    ctxSizeAttrs(4) = ctx.p1CSOrder();
    ctxSizeAttrs(5) = ctx.p2CSOrder();
    ctxSizeAttrs(6) = ctx.p12ACSOrder();
    ctxSizeAttrs(7) = ctx.p1Card();
    ctxSizeAttrs(8) = ctx.p2Card();
    
    writeArrayToFile(fs, ctxSizeAttrs); 
    
    // Write the data
    for(size_t order = 1; order <= ctx.p1MOrder(); order++){
        writeArrayToFile(fs, ctx.p1M(order));
    }
    
    for(size_t order = 1; order <= ctx.p2MOrder(); order++){
        writeArrayToFile(fs, ctx.p2M(order));
    }
        
    for(size_t order = 2; order <= ctx.p1CSOrder(); order++){
        writeArrayToFile(fs, ctx.p1CS(order));
    }
    
    for(size_t order = 2; order <= ctx.p2CSOrder(); order++){
        writeArrayToFile(fs, ctx.p2CS(order));
    }
    
    for(size_t order = 1; order <= ctx.p12ACSOrder(); order++){
        writeArrayToFile(fs, ctx.p12ACS(order));
    }

}


#endif /* FILEHANDLING_HPP */
