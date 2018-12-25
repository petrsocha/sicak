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
* \file filehandling.hpp
*
* \brief This header file contains function templates for loading/storing defined types
*
*
* \author Petr Socha
* \version 1.0
*/


#ifndef FILEHANDLING_HPP
#define FILEHANDLING_HPP

#include <fstream>
#include <string>
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
UnivariateContext<T> readContextFromFile(std::fstream & fs){
    
    uint64_t ctxAttrs[7];
    
    fs.read(reinterpret_cast<char *>(ctxAttrs), sizeof(uint64_t) * 7);
    
    if(fs.fail())
        throw RuntimeException("Failed to read context from file");
    
    UnivariateContext<T> ret(ctxAttrs[0], ctxAttrs[1], ctxAttrs[4], ctxAttrs[5], ctxAttrs[6]);
    ret.p1Card() = ctxAttrs[2];
    ret.p2Card() = ctxAttrs[3];
    
    for(size_t order = 1; order <= ret.mOrder(); order++){
        fillArrayFromFile(fs, ret.p1M(order));
        fillArrayFromFile(fs, ret.p2M(order));
    }
        
    for(size_t order = 2; order <= ret.csOrder(); order++){
        fillArrayFromFile(fs, ret.p1CS(order));
        fillArrayFromFile(fs, ret.p2CS(order));
    }
    
    for(size_t order = 1; order <= ret.acsOrder(); order++){
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
void writeContextToFile(std::fstream & fs, const UnivariateContext<T> & ctx){
     
    uint64_t ctxAttrs[7];
    ctxAttrs[0] = ctx.p1Width();
    ctxAttrs[1] = ctx.p2Width();
    ctxAttrs[2] = ctx.p1Card();
    ctxAttrs[3] = ctx.p2Card();
    ctxAttrs[4] = ctx.mOrder();
    ctxAttrs[5] = ctx.csOrder();
    ctxAttrs[6] = ctx.acsOrder();
    
    fs.write(reinterpret_cast<char *>(ctxAttrs), sizeof(uint64_t) * 7);
    
    if(fs.fail())
        throw RuntimeException("Failed to write context to file");
    
    for(size_t order = 1; order <= ctx.mOrder(); order++){
        writeArrayToFile(fs, ctx.p1M(order));
        writeArrayToFile(fs, ctx.p2M(order));
    }
        
    for(size_t order = 2; order <= ctx.csOrder(); order++){
        writeArrayToFile(fs, ctx.p1CS(order));
        writeArrayToFile(fs, ctx.p2CS(order));
    }
    
    for(size_t order = 1; order <= ctx.acsOrder(); order++){
        writeArrayToFile(fs, ctx.p12ACS(order));
    }

}


#endif /* FILEHANDLING_HPP */
