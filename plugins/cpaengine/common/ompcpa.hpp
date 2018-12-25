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

/*
*  Implemented algorithms are derived from equations published in:
*  Schneider, T., Moradi, A., & Güneysu, T. (2016, April). Robust and one-pass parallel 
*  computation of correlation-based attacks at arbitrary order. In International 
*  Workshop on Constructive Side-Channel Analysis and Secure Design (pp. 199-217). Springer, Cham.
*/

/**
* \file ompcpa.hpp
*
* \brief Implementation of CPA statistical algorithms as function templates for various SICAK plugins
*
*
* \author Petr Socha
* \version 1.0
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
* \brief Adds given power traces and power predictions to the given statistical context. Use zeroed or meaningful UnivariateContext c! Accelerated using OpenMP
*
*/
template <class T, class U, class V>
void UniFoCpaAddTraces(UnivariateContext<T>& c, const PowerTraces<U>& pt, const PowerPredictions<V>& pp) {
    
    if(c.mOrder() != 1 || c.csOrder() != 2 || c.acsOrder() != 1)
        throw RuntimeException("Not a valid first-order univariate CPA context!");

    if (c.p1Width() != pt.samplesPerTrace())
        throw RuntimeException("Incompatible context: Numbers of samples per trace don't match.");

    if (c.p2Width() != pp.noOfCandidates())
        throw RuntimeException("Incompatible context: Numbers of key candidates don't match.");

    if (pt.noOfTraces() != pp.noOfTraces())
        throw RuntimeException("Number of power traces doesn't match the number of power predictions.");

    const long long noOfTraces = pt.noOfTraces();
    const long long samplesPerTrace = pt.samplesPerTrace();
    const long long noOfCandidates = pp.noOfCandidates();

    for (long long trace = 0; trace < noOfTraces; trace++) {

        #pragma omp parallel for            
        for (long long candidate = 0; candidate < noOfCandidates; candidate++) {

            T p_trace = static_cast<T>(c.p1Card()) / (static_cast<T>(c.p1Card()) + 1.0f);
            T p_pp = pp(candidate, trace);
            T p_predsAvg = c.p2M(1)(candidate); // c.predsAvg(candidate);
            T p_optAlpha = p_trace * (p_pp - p_predsAvg);
            T * p_predsTracesCSum = &( c.p12ACS(1)(0, candidate) );  //&(c.predsTracesCSum(0, candidate));
            const U * p_pt = &(pt(0, trace));
            T * p_tracesAvg = &( c.p1M(1)(0) ); //&(c.tracesAvg(0));

            for (long long sample = 0; sample < samplesPerTrace; sample++) {

                //*(p_predsTracesCSum++) += p_trace * ( *(p_pt++) - *(p_tracesAvg++) ) * ( p_pp - p_predsAvg);
                *(p_predsTracesCSum++) += p_optAlpha * (static_cast<T>(*(p_pt++)) - *(p_tracesAvg++));

            }

        }

        for (long long candidate = 0; candidate < noOfCandidates; candidate++) {
            T temp = 0;
            temp = (static_cast<T>(pp(candidate, trace)) - c.p2M(1)(candidate));
            c.p2M(1)(candidate) += (temp / static_cast<T>((c.p1Card() + 1)));
            c.p2CS(2)(candidate) += temp * (static_cast<T>(pp(candidate, trace)) - c.p2M(1)(candidate));
        }

        for (long long sample = 0; sample < samplesPerTrace; sample++) {
            T temp = 0;
            temp = (static_cast<T>(pt(sample, trace)) - c.p1M(1)(sample));
            c.p1M(1)(sample) += (temp / static_cast<T>((c.p1Card() + 1)));
            c.p1CS(2)(sample) += temp * (static_cast<T>(pt(sample, trace)) - c.p1M(1)(sample));
        }

        c.p1Card() = c.p1Card() + 1;

    }
    
    c.p2Card() = c.p1Card();

}

/**
*
* \brief Merges two UnivariateContext and leaves the result in first context given
*
*/
template <class T>
void UniFoCpaMergeContexts(UnivariateContext<T>& firstAndOut, const UnivariateContext<T>& second) {
    
    if(firstAndOut.mOrder() != 1 || firstAndOut.csOrder() != 2 || firstAndOut.acsOrder() != 1 || second.mOrder() != 1 || second.csOrder() != 2 || second.acsOrder() != 1)
        throw RuntimeException("Not valid first-order univariate CPA contexts!");
    
    if(firstAndOut.p1Card() != firstAndOut.p2Card() || second.p1Card() != second.p2Card())
        throw RuntimeException("Malformed CPA context");
    
    if(firstAndOut.p1Width() != second.p1Width() || firstAndOut.p2Width() != second.p2Width())
        throw RuntimeException("Only contexts with same number of candidates and same number of samples per trace can be merged");
        
    const size_t samplesPerTrace = firstAndOut.p1Width();
    const size_t noOfCandidates = firstAndOut.p2Width();
    
    const size_t firstSize = firstAndOut.p1Card();
    const size_t secondSize = second.p1Card();
    
    // First, merge the ACSs
    for(size_t candidate = 0; candidate < noOfCandidates; candidate++){
        
        for(size_t sample = 0; sample < samplesPerTrace; sample++) {                
            
            firstAndOut.p12ACS(1)(sample, candidate) += second.p12ACS(1)(sample, candidate);            
            firstAndOut.p12ACS(1)(sample, candidate) += ( (secondSize * firstSize * firstSize + firstSize * secondSize * secondSize) /
                                                     ((firstSize + secondSize) * (firstSize + secondSize)) ) * 
                                                     (second.p1M(1)(sample) - firstAndOut.p1M(1)(sample)) *
                                                     (second.p2M(1)(candidate) - firstAndOut.p2M(1)(candidate));            
            
        }
        
    }
    
    // then merge the MSums
    for(size_t sample = 0; sample < samplesPerTrace; sample++) {        
        firstAndOut.p1CS(2)(sample) += second.p1CS(2)(sample);
        firstAndOut.p1CS(2)(sample) += (firstSize * secondSize) *                                          
                                      ( (second.p1M(1)(sample) - firstAndOut.p1M(1)(sample)) / (firstSize + secondSize) ) *
                                      ( (second.p1M(1)(sample) - firstAndOut.p1M(1)(sample)) / (firstSize + secondSize) ) *
                                      (firstSize + secondSize);
    }
    
    for(size_t candidate = 0; candidate < noOfCandidates; candidate++){
        firstAndOut.p2CS(2)(candidate) += second.p2CS(2)(candidate);        
        firstAndOut.p2CS(2)(candidate) += (firstSize * secondSize) *                                            
                                         ( (second.p2M(1)(candidate) - firstAndOut.p2M(1)(candidate)) / (firstSize + secondSize) ) *
                                         ( (second.p2M(1)(candidate) - firstAndOut.p2M(1)(candidate)) / (firstSize + secondSize) ) *
                                         (firstSize + secondSize);
                
    }        
    
    // then merge the means
    for(size_t sample = 0; sample < samplesPerTrace; sample++) {        
        firstAndOut.p1M(1)(sample) = ( (firstAndOut.p1M(1)(sample) * firstSize) + (second.p1M(1)(sample) * secondSize) ) / (firstSize + secondSize);
    }
    
    for(size_t candidate = 0; candidate < noOfCandidates; candidate++){
        firstAndOut.p2M(1)(candidate) = ( (firstAndOut.p2M(1)(candidate) * firstSize) + (second.p2M(1)(candidate) * secondSize) ) / (firstSize + secondSize);
    }        
    
    // finally update the cardinality of the context
    firstAndOut.p1Card() += second.p1Card();
    firstAndOut.p2Card() = firstAndOut.p1Card();
    
}

/**
*
* \brief Computes final correlation matrix based on a UnivariateContext given, stores results in correlations
*
*/
template <class T>
void UniFoCpaComputeCorrelationMatrix(const UnivariateContext<T> & c, MatrixType<T> & correlations){
    
    if(c.mOrder() != 1 || c.csOrder() != 2 || c.acsOrder() != 1 || c.p1Card() != c.p2Card())
        throw RuntimeException("Not a valid first-order univariate CPA context!");

    size_t samplesPerTrace = c.p1Width();
    size_t noOfCandidates = c.p2Width();
    
    correlations.init(samplesPerTrace, noOfCandidates);
    Vector<T> sqrtTracesCS2(samplesPerTrace);
    Vector<T> sqrtPredsCS2(noOfCandidates);
            

    for (size_t sample = 0; sample < samplesPerTrace; sample++) {
        sqrtTracesCS2(sample) = sqrt(c.p1CS(2)(sample));
    }

    for (size_t candidate = 0; candidate < noOfCandidates; candidate++) {
        sqrtPredsCS2(candidate) = sqrt(c.p2CS(2)(candidate));
    }

    for (size_t candidate = 0; candidate < noOfCandidates; candidate++) {
            
        for (size_t sample = 0; sample < samplesPerTrace; sample++) {

            if (sqrtTracesCS2(sample) == 0 || sqrtPredsCS2(candidate) == 0) throw RuntimeException("Division by zero");

            correlations(sample, candidate) = c.p12ACS(1)(sample, candidate) / (sqrtTracesCS2(sample) * sqrtPredsCS2(candidate));

        }
        
    }
    
}

#endif /* OMPCPA_H */
 

