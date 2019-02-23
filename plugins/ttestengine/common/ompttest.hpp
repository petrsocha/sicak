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

/*
*  Implemented algorithms are derived from equations published in:
*  Schneider, T., Moradi, A., & Güneysu, T. (2016, April). Robust and one-pass parallel 
*  computation of correlation-based attacks at arbitrary order. In International 
*  Workshop on Constructive Side-Channel Analysis and Secure Design (pp. 199-217). Springer, Cham.
*/

/**
* \file ompttest.hpp
*
* \brief Implementation of t-test statistical algorithms as function templates for various SICAK plugins
*
*
* \author Petr Socha
* \version 1.1
*/

#ifndef OMPCPA_H
#define OMPCPA_H 

#include <cmath>
#include <omp.h>
#include "exceptions.hpp"
#include "types_power.hpp"
#include "types_stat.hpp"

/**
*
* \brief Adds given random and constant power traces to the given statistical context. Use zeroed or meaningful Moments2DContext c!
*
*/
template <class T, class U>
void UniFoTTestAddTraces(Moments2DContext<T>& c, const PowerTraces<U>& randTraces, const PowerTraces<U>& constTraces) {
    
    if(c.p1MOrder() != 1 || c.p1CSOrder() != 2 || c.p12ACSOrder() != 0 || c.p1MOrder() != c.p2MOrder() || c.p1CSOrder() != c.p2CSOrder() || c.p1Width() != c.p2Width())
        throw RuntimeException("Not a valid first-order univariate t-test context!");

    if (c.p1Width() != randTraces.samplesPerTrace() || c.p1Width() != constTraces.samplesPerTrace())
        throw RuntimeException("Numbers of samples don't match.");    

    const long long samplesPerTrace = randTraces.samplesPerTrace();
    const long long noOfRandTraces = randTraces.noOfTraces();
    const long long noOfConstTraces = constTraces.noOfTraces();

    for (long long trace = 0; trace < noOfRandTraces; trace++) {        
        
        for (long long sample = 0; sample < samplesPerTrace; sample++) {
            T temp = 0;
            temp = (static_cast<T>(randTraces(sample, trace)) - c.p1M(1)(sample));
            c.p1M(1)(sample) += (temp / static_cast<T>((c.p1Card() + 1)));
            c.p1CS(2)(sample) += temp * (static_cast<T>(randTraces(sample, trace)) - c.p1M(1)(sample));
        }

        c.p1Card() = c.p1Card() + 1;

    }
    
    for (long long trace = 0; trace < noOfConstTraces; trace++) {        
        
        for (long long sample = 0; sample < samplesPerTrace; sample++) {
            T temp = 0;
            temp = (static_cast<T>(constTraces(sample, trace)) - c.p2M(1)(sample));
            c.p2M(1)(sample) += (temp / static_cast<T>((c.p2Card() + 1)));
            c.p2CS(2)(sample) += temp * (static_cast<T>(constTraces(sample, trace)) - c.p2M(1)(sample));
        }

        c.p2Card() = c.p2Card() + 1;

    }    

}

/**
*
* \brief Merges two Moments2DContext and leaves the result in first context given
*
*/
template <class T>
void UniFoTTestMergeContexts(Moments2DContext<T>& firstAndOut, const Moments2DContext<T>& second) {
    
    if(firstAndOut.p1MOrder() != 1 || firstAndOut.p1CSOrder() != 2 || firstAndOut.p12ACSOrder() != 0 
        || firstAndOut.p1MOrder() != firstAndOut.p2MOrder() || firstAndOut.p1CSOrder() != firstAndOut.p2CSOrder()
        || second.p1MOrder() != 1 || second.p1CSOrder() != 2 || second.p12ACSOrder() != 0
        || second.p1MOrder() != second.p2MOrder() || second.p1CSOrder() != second.p2CSOrder()
        || firstAndOut.p1Width() != firstAndOut.p2Width() || second.p1Width() != second.p2Width()) 
        throw RuntimeException("Not valid first-order univariate t-test contexts!");        
    
    if(firstAndOut.p1Width() != second.p1Width())
        throw RuntimeException("Only contexts with same number of samples per trace can be merged");
                        
    const size_t samplesPerTrace = firstAndOut.p1Width();
    
    
    // random
    size_t firstSize = firstAndOut.p1Card();
    size_t secondSize = second.p1Card();        
    // merge the MSums    
    for(size_t sample = 0; sample < samplesPerTrace; sample++) {        
        firstAndOut.p1CS(2)(sample) += second.p1CS(2)(sample);
        firstAndOut.p1CS(2)(sample) += (firstSize * secondSize) *                                          
                                      ( (second.p1M(1)(sample) - firstAndOut.p1M(1)(sample)) / (firstSize + secondSize) ) *
                                      ( (second.p1M(1)(sample) - firstAndOut.p1M(1)(sample)) / (firstSize + secondSize) ) *
                                      (firstSize + secondSize);
    }
    // then merge the means
    for(size_t sample = 0; sample < samplesPerTrace; sample++) {        
        firstAndOut.p1M(1)(sample) = ( (firstAndOut.p1M(1)(sample) * firstSize) + (second.p1M(1)(sample) * secondSize) ) / (firstSize + secondSize);
    }
    
    
    // const
    firstSize = firstAndOut.p2Card();
    secondSize = second.p2Card(); 
    
    for(size_t sample = 0; sample < samplesPerTrace; sample++) {        
        firstAndOut.p2CS(2)(sample) += second.p2CS(2)(sample);
        firstAndOut.p2CS(2)(sample) += (firstSize * secondSize) *                                          
                                      ( (second.p2M(1)(sample) - firstAndOut.p2M(1)(sample)) / (firstSize + secondSize) ) *
                                      ( (second.p2M(1)(sample) - firstAndOut.p2M(1)(sample)) / (firstSize + secondSize) ) *
                                      (firstSize + secondSize);
    }
    
    
    
    for(size_t sample = 0; sample < samplesPerTrace; sample++) {        
        firstAndOut.p2M(1)(sample) = ( (firstAndOut.p2M(1)(sample) * firstSize) + (second.p2M(1)(sample) * secondSize) ) / (firstSize + secondSize);
    }     
    
    // finally update the cardinality of the context
    firstAndOut.p1Card() += second.p1Card();
    firstAndOut.p2Card() += second.p2Card();
    
}

/**
*
* \brief Computes final t-values and degrees of freedom based on a Moments2DContext given, stores t-vals in first row, d.o.f. in second row of output matrix
*
*/
template <class T>
void UniFoTTestComputeTValsDegs(const Moments2DContext<T> & c, MatrixType<T> & tValsDegs){
    
    if(c.p1MOrder() != 1 || c.p1CSOrder() != 2 || c.p12ACSOrder() != 0 || c.p1MOrder() != c.p2MOrder() || c.p1CSOrder() != c.p2CSOrder() || c.p1Width() != c.p2Width())
        throw RuntimeException("Not a valid first-order univariate t-test context!");

    size_t samplesPerTrace = c.p1Width();
    
    tValsDegs.init(samplesPerTrace, 2);
    
    for(size_t sample = 0; sample < samplesPerTrace; sample++){

        // t-values
        tValsDegs(sample, 0) = (c.p2M(1)(sample) - c.p1M(1)(sample))
                               / sqrt(((c.p2CS(2)(sample) / (double)(c.p2Card() - 1)) / (double)c.p2Card()) 
                                    + ((c.p1CS(2)(sample) / (double)(c.p1Card() - 1)) / (double)c.p1Card()));
    
        // degrees of freedom
        tValsDegs(sample, 1) = ( ( ( (c.p2CS(2)(sample) / (double)(c.p2Card() - 1) ) / (double)c.p2Card() ) 
                                 + ( (c.p1CS(2)(sample) / (double)(c.p1Card() - 1) ) / (double)c.p1Card() ) )                 
                               * ( ( (c.p2CS(2)(sample) / (double)(c.p2Card() - 1) ) / (double)c.p2Card() ) 
                                 + ( (c.p1CS(2)(sample) / (double)(c.p1Card() - 1) ) / (double)c.p1Card() ) ) ) 
                             /
                               ( ( ( ( (c.p2CS(2)(sample) / (double)(c.p2Card() - 1) ) / (double)c.p2Card() ) 
                                   * ( (c.p2CS(2)(sample) / (double)(c.p2Card() - 1) ) / (double)c.p2Card() ) ) 
                                 / (double)(c.p2Card() - 1) ) 
                               + ( ( ( (c.p1CS(2)(sample) / (double)(c.p1Card() - 1) ) / (double)c.p1Card()) 
                                   * ( (c.p1CS(2)(sample) / (double)(c.p1Card() - 1) ) / (double)c.p1Card()) ) 
                                 / (double)(c.p1Card() - 1) ) );                                                                
                                
    }
        
}

#endif /* OMPCPA_H */
 

