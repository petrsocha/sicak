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
* \version 1.2
*/

#ifndef OMPTTEST_H
#define OMPTTEST_H 

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


/**
*
* \brief Adds given random and constant power traces to the given statistical context performing preprocessing for a specified order of the attack. Note, that this kind of context cannot be meaningfully merged. Use zeroed Moments2DContext c!
*
*/
template <class T, class U>
void UniPrepHoTTestAddTraces(Moments2DContext<T>& c, const PowerTraces<U>& randTraces, const PowerTraces<U>& constTraces, size_t attackOrder) {
    
    if(c.p1MOrder() != 1 
    || c.p1CSOrder() != 2 
    || c.p12ACSOrder() != 0 
    || c.p1MOrder() != c.p2MOrder() 
    || c.p1CSOrder() != c.p2CSOrder() 
    || c.p1Width() != c.p2Width()        
    )
        throw RuntimeException("Not a valid first-order univariate t-test context!");
    
    if(attackOrder < 2) throw RuntimeException("Invalid attack order", attackOrder);
    
    const long long samplesPerTrace = randTraces.samplesPerTrace();
    const long long noOfRandTraces = randTraces.noOfTraces();
    const long long noOfConstTraces = constTraces.noOfTraces();
    
    // preprocessing random traces
    Vector<T> avgs(samplesPerTrace, 0);
    Vector<T> vars(samplesPerTrace, 0);
    PowerTraces<T> prpt(samplesPerTrace, noOfRandTraces);
    
    // compute avg and var in every sample point
    for (long long trace = 0; trace < noOfRandTraces; trace++) {
        
        for (long long sample = 0; sample < samplesPerTrace; sample++) {
            
            T deltaT = static_cast<T>(randTraces(sample, trace)) - avgs(sample);
            
            avgs(sample) += deltaT / static_cast<T>(trace + 1);
            vars(sample) += ( (deltaT * deltaT) * static_cast<T>(trace) ) / static_cast<T>(trace + 1);
            
        }
        
    }
    
    for (long long sample = 0; sample < samplesPerTrace; sample++) {
        
        vars(sample) = std::sqrt(vars(sample));
        
    }
    
    // preprocess the power traces
    for (long long trace = 0; trace < noOfRandTraces; trace++) {
        
        for (long long sample = 0; sample < samplesPerTrace; sample++) {

            prpt(sample, trace) = std::pow(static_cast<T>(randTraces(sample, trace)) - avgs(sample), attackOrder); 
            
            if(attackOrder > 2) {
            
                prpt(sample, trace) /=  std::pow(vars(sample), attackOrder);
                
            }                        
            
        }
        
    }
    
    // preprocessing constant traces
    avgs.fill(0);
    vars.fill(0);
    PowerTraces<T> pcpt(samplesPerTrace, noOfConstTraces);
    
    // compute avg and var in every sample point
    for (long long trace = 0; trace < noOfConstTraces; trace++) {
        
        for (long long sample = 0; sample < samplesPerTrace; sample++) {
            
            T deltaT = static_cast<T>(constTraces(sample, trace)) - avgs(sample);
            
            avgs(sample) += deltaT / static_cast<T>(trace + 1);
            vars(sample) += ( (deltaT * deltaT) * static_cast<T>(trace) ) / static_cast<T>(trace + 1);
            
        }
        
    }
    
    for (long long sample = 0; sample < samplesPerTrace; sample++) {
        
        vars(sample) = std::sqrt(vars(sample));
        
    }
    
    // preprocess the power traces
    for (long long trace = 0; trace < noOfConstTraces; trace++) {
        
        for (long long sample = 0; sample < samplesPerTrace; sample++) {

            pcpt(sample, trace) = std::pow(static_cast<T>(constTraces(sample, trace)) - avgs(sample), attackOrder); 
            
            if(attackOrder > 2) {
            
                pcpt(sample, trace) /=  std::pow(vars(sample), attackOrder);
                
            }                        
            
        }
        
    }
    
    UniFoTTestAddTraces(c, prpt, pcpt);
    return;
    
}


/**
*
* \brief Adds given random and constant power traces to the given statistical context. Use zeroed or meaningful Moments2DContext c!
*
*/
template <class T, class U>
void UniHoTTestAddTraces(Moments2DContext<T>& c, const PowerTraces<U>& randTraces, const PowerTraces<U>& constTraces, size_t attackOrder) {
    
    if(c.p1MOrder() != 1 
    || c.p1CSOrder() != 2 * attackOrder 
    || c.p12ACSOrder() != 0 
    || c.p1MOrder() != c.p2MOrder() 
    || c.p1CSOrder() != c.p2CSOrder() 
    || c.p1Width() != c.p2Width()        
    )
        throw RuntimeException("Not a valid higher-order univariate t-test context!");

    if (c.p1Width() != randTraces.samplesPerTrace() || c.p1Width() != constTraces.samplesPerTrace())
        throw RuntimeException("Numbers of samples don't match.");    
    
    if(attackOrder < 1) 
        throw RuntimeException("Invalid order of the t-test.", attackOrder);

    const long long samplesPerTrace = randTraces.samplesPerTrace();
    const long long noOfRandTraces = randTraces.noOfTraces();
    const long long noOfConstTraces = constTraces.noOfTraces();    
    
    Matrix<T> deltaT(samplesPerTrace, 2 * attackOrder);
    Matrix<T> minusDivN(2 * attackOrder, 1);
    Matrix<T> nCr(2*attackOrder + 1, 2 * attackOrder + 1, 0);        
    
    // precompute combination numbers        
    for(size_t n = 0; n <= 2*attackOrder; n++){
        nCr(n, 0) = 1;
        for(size_t r = 1; r <= n; r++){
            nCr(n, r) = (nCr(n, r-1) * (n - r + 1)) / r;                
        }
    }

    // random traces
    for (long long trace = 0; trace < noOfRandTraces; trace++) {        
        
        T n = c.p1Card() + 1.0; // n = cardinality of the merged set
        T divN = 1.0 / n;        
        
        {
            //  precompute deltaT
            T * p_deltaT = &(deltaT(0, 0));
            const U * p_pt = &(randTraces(0, trace));
            const T * p_tracesAvg = &( c.p1M(1)(0) );
            for (long long sample = 0; sample < samplesPerTrace; sample++) {
                
                (*p_deltaT++) = static_cast<T>((*p_pt++)) - (*p_tracesAvg++);
                
            }
            
            // and all its necessary powers
            for (size_t order = 1; order < 2 * attackOrder; order++){
                
                const T * p_firstDeltaT = &(deltaT(0, 0));
                const T * p_lastDeltaT = &(deltaT(0, order-1));
                p_deltaT = &(deltaT(0, order));
                
                for (long long sample = 0; sample < samplesPerTrace; sample++) {
                    (*p_deltaT++) = (*p_lastDeltaT++) * (*p_firstDeltaT++); 
                }
                            
            }
            
            // precompute (-1*divN)^d
            minusDivN(0,0) = (-1.0) * divN;            
            for (size_t order = 1; order < 2 * attackOrder; order++){
                
                minusDivN(order, 0) = minusDivN(order-1, 0) * minusDivN(0,0); 
                
            }
            
        }
        
        for(long long deg = 2 * attackOrder; deg >= 2; deg--){            
            
            const T p_alpha = ( (n > 1) ? (1.0 - std::pow( (-1.0)/(n-1.0), deg-1 ) ) : 0 );
            const T p_beta = p_alpha * std::pow( ((n-1.0) * divN), deg);
            const T * p_deltaT = &( deltaT(0, deg - 1) );
            T * p_p1CSdeg = &(c.p1CS(deg)(0));
            
            for (long long sample = 0; sample < samplesPerTrace; sample++) {             
                
                (*p_p1CSdeg++) += p_beta * (*p_deltaT++);
                
            }
            
            for(long long p = 1; p <= deg - 2; p++){
                
                const T * p_p1CSless = &( c.p1CS(deg-p)(0) );
                const T p_delta = minusDivN(p - 1, 0) * nCr(deg, p);
                const T * p_deltaTPow = &( deltaT(0, p - 1) );
                p_p1CSdeg = &(c.p1CS(deg)(0));
                
                for (long long sample = 0; sample < samplesPerTrace; sample++) {
                                                            
                    (*p_p1CSdeg++) += *(p_p1CSless++) * p_delta * (*p_deltaTPow++);
                    
                }
                    
            }
            
        }
        
        for (long long sample = 0; sample < samplesPerTrace; sample++) {            
            
            c.p1M(1)(sample) += deltaT(sample, 0) * divN;            
            
        }

        c.p1Card() = c.p1Card() + 1;

    }        
    
    // constant traces
    for (long long trace = 0; trace < noOfConstTraces; trace++) {        
        
        T n = c.p2Card() + 1.0; // n = cardinality of the merged set
        T divN = 1.0 / n;        
        
        {
            //  precompute deltaT
            T * p_deltaT = &(deltaT(0, 0));
            const U * p_pt = &(constTraces(0, trace));
            const T * p_tracesAvg = &( c.p2M(1)(0) );
            for (long long sample = 0; sample < samplesPerTrace; sample++) {
                
                (*p_deltaT++) = static_cast<T>((*p_pt++)) - (*p_tracesAvg++);
                
            }
            
            // and all its necessary powers
            for (size_t order = 1; order < 2 * attackOrder; order++){
                
                const T * p_firstDeltaT = &(deltaT(0, 0));
                const T * p_lastDeltaT = &(deltaT(0, order-1));
                p_deltaT = &(deltaT(0, order));
                
                for (long long sample = 0; sample < samplesPerTrace; sample++) {
                    (*p_deltaT++) = (*p_lastDeltaT++) * (*p_firstDeltaT++); 
                }
                            
            }
            
            // precompute (-1*divN)^d
            minusDivN(0,0) = (-1.0) * divN;            
            for (size_t order = 1; order < 2 * attackOrder; order++){
                
                minusDivN(order, 0) = minusDivN(order-1, 0) * minusDivN(0,0); 
                
            }
            
        }
        
        for(long long deg = 2 * attackOrder; deg >= 2; deg--){            
            
            const T p_alpha = ( (n > 1) ? (1.0 - std::pow( (-1.0)/(n-1.0), deg-1 ) ) : 0 );
            const T p_beta = p_alpha * std::pow( ((n-1.0) * divN), deg);
            const T * p_deltaT = &( deltaT(0, deg - 1) );
            T * p_p2CSdeg = &(c.p2CS(deg)(0));
            
            for (long long sample = 0; sample < samplesPerTrace; sample++) {             
                
                (*p_p2CSdeg++) += p_beta * (*p_deltaT++);
                
            }
            
            for(long long p = 1; p <= deg - 2; p++){
                
                const T * p_p2CSless = &( c.p2CS(deg-p)(0) );
                const T p_delta = minusDivN(p - 1, 0) * nCr(deg, p);
                const T * p_deltaTPow = &( deltaT(0, p - 1) );
                p_p2CSdeg = &(c.p2CS(deg)(0));
                
                for (long long sample = 0; sample < samplesPerTrace; sample++) {
                                                            
                    (*p_p2CSdeg++) += *(p_p2CSless++) * p_delta * (*p_deltaTPow++);
                    
                }
                    
            }
            
        }
        
        for (long long sample = 0; sample < samplesPerTrace; sample++) {            
            
            c.p2M(1)(sample) += deltaT(sample, 0) * divN;            
            
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
void UniHoTTestMergeContexts(Moments2DContext<T>& firstAndOut, const Moments2DContext<T>& second) {
    
    if(firstAndOut.p1MOrder() != 1 
    || firstAndOut.p1CSOrder() < 2 
    || firstAndOut.p12ACSOrder() != 0 
    || firstAndOut.p1MOrder() != firstAndOut.p2MOrder() 
    || firstAndOut.p1CSOrder() != firstAndOut.p2CSOrder()
    || second.p1MOrder() != 1 
    || second.p1CSOrder() < 2 
    || second.p12ACSOrder() != 0
    || second.p1MOrder() != second.p2MOrder() 
    || second.p1CSOrder() != second.p2CSOrder()
    || firstAndOut.p1CSOrder() != second.p1CSOrder()
    || firstAndOut.p1Width() != firstAndOut.p2Width() 
    || second.p1Width() != second.p2Width()        
    ) 
        throw RuntimeException("Not valid mergable first-order univariate t-test contexts!");        
    
    if(firstAndOut.p1Width() != second.p1Width())
        throw RuntimeException("Only contexts with same number of samples per trace can be merged");
                        
    const size_t samplesPerTrace = firstAndOut.p1Width();
    const size_t csOrder = firstAndOut.p1CSOrder();                  
    
    // precompute combination numbers
    Matrix<T> nCr(csOrder + 1, csOrder + 1, 0);    
        
    for(size_t n = 0; n <= csOrder; n++){
        nCr(n, 0) = 1;
        for(size_t r = 1; r <= n; r++){
            nCr(n, r) = (nCr(n, r-1) * (n - r + 1)) / r;                
        }
    }
            
    // random
    size_t n1 = firstAndOut.p1Card();
    size_t n2 = second.p1Card();  
    
    if(n1 == 0 || n2 == 0) throw RuntimeException("Empty context");
    
    // precompute delta-s
    Vector<T> deltaT(samplesPerTrace);
    
    for(size_t sample = 0; sample < samplesPerTrace; sample++) {                
        
        deltaT(sample) = second.p1M(1)(sample) - firstAndOut.p1M(1)(sample);
        
    }  
    
    for(size_t deg = csOrder; deg >= 2; deg--){
        
        const T p_alpha = ((n1*n2)/(n1+n2));
        const T p_beta = ( std::pow(1.0 / n2, deg-1) - std::pow( (-1.0) / n1, deg-1) );
        
        for(size_t sample = 0; sample < samplesPerTrace; sample++) {
            
            firstAndOut.p1CS(deg)(sample) += second.p1CS(deg)(sample);
            
            firstAndOut.p1CS(deg)(sample) += std::pow( p_alpha * deltaT(sample), deg) * p_beta;                                                       
            
        }
        
        
        for(size_t p = 1; p <= deg - 2; p++){     
            
            const T p_gamma = std::pow( ( (-1.0) * n2 ) / (n1+n2) , p );
            const T p_delta = std::pow( n1 / (n1+n2) , p );
            
            for(size_t sample = 0; sample < samplesPerTrace; sample++) {
                    
                T sumTerm = 0;
                
                if(deg-p >= 2) {
                    
                    sumTerm += p_gamma * firstAndOut.p1CS(deg-p)(sample);
                    sumTerm += p_delta * second.p1CS(deg-p)(sample);
                    
                }
                
                sumTerm *= nCr(deg, p);
                
                sumTerm *= std::pow( deltaT(sample), p);
                
                firstAndOut.p1CS(deg)(sample) += sumTerm;
                
            }
                
        }
            
    }
    
    for(size_t sample = 0; sample < samplesPerTrace; sample++) {        
        firstAndOut.p1M(1)(sample) = ( (firstAndOut.p1M(1)(sample) * n1) + (second.p1M(1)(sample) * n2) ) / (n1 + n2);
    }
    
    
    // const
    n1 = firstAndOut.p2Card();
    n2 = second.p2Card();  
    
    if(n1 == 0 || n2 == 0) throw RuntimeException("Empty context");
    
    // precompute delta-s 
    for(size_t sample = 0; sample < samplesPerTrace; sample++) {                
        
        deltaT(sample) = second.p2M(1)(sample) - firstAndOut.p2M(1)(sample);
        
    }  
    
    for(size_t deg = csOrder; deg >= 2; deg--){
        
        const T p_alpha = ((n1*n2)/(n1+n2));
        const T p_beta = ( std::pow(1.0 / n2, deg-1) - std::pow( (-1.0) / n1, deg-1) );
        
        for(size_t sample = 0; sample < samplesPerTrace; sample++) {
            
            firstAndOut.p2CS(deg)(sample) += second.p2CS(deg)(sample);
            
            firstAndOut.p2CS(deg)(sample) += std::pow( p_alpha * deltaT(sample), deg) * p_beta;                                                       
            
        }
        
        
        for(size_t p = 1; p <= deg - 2; p++){     
            
            const T p_gamma = std::pow( ( (-1.0) * n2 ) / (n1+n2) , p );
            const T p_delta = std::pow( n1 / (n1+n2) , p );
            
            for(size_t sample = 0; sample < samplesPerTrace; sample++) {
                    
                T sumTerm = 0;
                
                if(deg-p >= 2) {
                    
                    sumTerm += p_gamma * firstAndOut.p2CS(deg-p)(sample);
                    sumTerm += p_delta * second.p2CS(deg-p)(sample);
                    
                }
                
                sumTerm *= nCr(deg, p);
                
                sumTerm *= std::pow( deltaT(sample), p);
                
                firstAndOut.p2CS(deg)(sample) += sumTerm;
                
            }
                
        }
            
    }
    
    
    for(size_t sample = 0; sample < samplesPerTrace; sample++) {        
        firstAndOut.p2M(1)(sample) = ( (firstAndOut.p2M(1)(sample) * n1) + (second.p2M(1)(sample) * n2) ) / (n1 + n2);
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
void UniHoTTestComputeTValsDegs(const Moments2DContext<T> & c, MatrixType<T> & tValsDegs, size_t attackOrder){
    
    if(c.p1MOrder() != 1 
    || c.p1CSOrder() < attackOrder * 2 
    || c.p12ACSOrder() != 0 
    || c.p1MOrder() != c.p2MOrder() 
    || c.p1CSOrder() != c.p2CSOrder() 
    || c.p1Width() != c.p2Width()        
    )
        throw RuntimeException("Not a valid higher-order univariate t-test context!", c.p1CSOrder());

    size_t samplesPerTrace = c.p1Width();
    
    tValsDegs.init(samplesPerTrace, 2);
    
    T randomCardinality = c.p1Card();
    T constCardinality = c.p2Card();
    
    for(size_t sample = 0; sample < samplesPerTrace; sample++){
        
        T meanDelta = 0;
        T randomVariance = 1;
        T constVariance = 1;
        
        if(attackOrder == 1){
            
            T randomMean = c.p1M(1)(sample);
            T constMean  = c.p2M(1)(sample);                        
            meanDelta = constMean - randomMean;
            
            randomVariance = c.p1CS(2)(sample) / randomCardinality;
            constVariance  = c.p2CS(2)(sample) / constCardinality;
            
        } else if(attackOrder == 2){
            
            T randomMean = c.p1CS(2)(sample) / randomCardinality;
            T constMean  = c.p2CS(2)(sample) / constCardinality; 
            meanDelta = constMean - randomMean;
            
            randomVariance = (c.p1CS(4)(sample) / randomCardinality) - std::pow( c.p1CS(2)(sample) / randomCardinality , 2 );
            constVariance  = (c.p2CS(4)(sample) / constCardinality)  - std::pow( c.p2CS(2)(sample) / constCardinality  , 2 );
            
            
        } else { // > 2 

            T randomMean = (c.p1CS(attackOrder)(sample) / randomCardinality) / std::pow( std::sqrt( c.p1CS(2)(sample) / randomCardinality ) , attackOrder);
            T constMean  = (c.p2CS(attackOrder)(sample) / constCardinality)  / std::pow( std::sqrt( c.p2CS(2)(sample) / constCardinality  ) , attackOrder);
            meanDelta = constMean - randomMean;
            
            randomVariance = ( (c.p1CS(attackOrder*2)(sample) / randomCardinality) - std::pow( c.p1CS(attackOrder)(sample) / randomCardinality , 2) )
                           / std::pow( c.p1CS(2)(sample) / randomCardinality , attackOrder);
            constVariance  = ( (c.p2CS(attackOrder*2)(sample) / constCardinality)  - std::pow( c.p2CS(attackOrder)(sample) / constCardinality  , 2) )
                           / std::pow( c.p2CS(2)(sample) / constCardinality  , attackOrder);
            
        }
        
        
        // t-values
        tValsDegs(sample, 0) = (meanDelta) / std::sqrt((constVariance / constCardinality) + (randomVariance / randomCardinality));
    
        T num = ( constVariance / constCardinality ) + ( randomVariance / randomCardinality );
        num = num * num;
        
        T den1 = ( constVariance / constCardinality );
        den1 = den1 * den1;
        den1 = den1 / ( constCardinality - 1.0);
        
        T den2 = ( randomVariance / randomCardinality );
        den2 = den2 * den2;
        den2 = den2 / ( randomCardinality - 1.0);
        
        // degrees of freedom
        tValsDegs(sample, 1) = num / (den1 + den2);                                                                
                                
    }
        
}

#endif /* OMPTTEST_H */
