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
*  When referring to these implementations, please cite following:
*
*  Socha, P., Miškovský, V., Kubátová, H., & Novotný, M. (2017, April). Optimization of Pearson 
*  correlation coefficient calculation for DPA and comparison of different approaches. In 2017 
*  IEEE 20th International Symposium on Design and Diagnostics of Electronic Circuits & Systems 
*  (DDECS) (pp. 184-189). IEEE.
* 
*  Implemented algorithms are derived from equations published in:
* 
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
* \version 1.2
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
* \brief Adds given power traces and power predictions to the given statistical context. Use zeroed or meaningful Moments2DContext c! Accelerated using OpenMP
*
*/
template <class T, class U, class V>
void UniFoCpaAddTraces(Moments2DContext<T>& c, const PowerTraces<U>& pt, const PowerPredictions<V>& pp) {
    
    if(c.p1MOrder() != 1 || c.p1CSOrder() != 2 || c.p12ACSOrder() != 1 || c.p1MOrder() != c.p2MOrder() || c.p1CSOrder() != c.p2CSOrder())
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
            T p_predsAvg = c.p2M(1)(candidate); 
            T p_optAlpha = p_trace * (p_pp - p_predsAvg);
            T * p_predsTracesCSum = &( c.p12ACS(1)(0, candidate) );  
            const U * p_pt = &(pt(0, trace));
            T * p_tracesAvg = &( c.p1M(1)(0) ); 

            for (long long sample = 0; sample < samplesPerTrace; sample++) {

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
* \brief Merges two Moments2DContext and leaves the result in first context given
*
*/
template <class T>
void UniFoCpaMergeContexts(Moments2DContext<T>& firstAndOut, const Moments2DContext<T>& second) {
    
    if(firstAndOut.p1MOrder() != 1 || firstAndOut.p1CSOrder() != 2 || firstAndOut.p12ACSOrder() != 1 
        || firstAndOut.p1MOrder() != firstAndOut.p2MOrder() || firstAndOut.p1CSOrder() != firstAndOut.p2CSOrder()
        || second.p1MOrder() != 1 || second.p1CSOrder() != 2 || second.p12ACSOrder() != 1
        || second.p1MOrder() != second.p2MOrder() || second.p1CSOrder() != second.p2CSOrder())
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
* \brief Computes final correlation matrix based on a Moments2DContext given, stores results in correlations
*
*/
template <class T>
void UniFoCpaComputeCorrelationMatrix(const Moments2DContext<T> & c, MatrixType<T> & correlations){
    
    if(c.p1MOrder() != 1 || c.p1CSOrder() != 2 || c.p12ACSOrder() != 1 || c.p1MOrder() != c.p2MOrder() || c.p1CSOrder() != c.p2CSOrder() || c.p1Card() != c.p2Card())
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

/**
*
* \brief Adds given power traces and power predictions to the statistical context, performing preprocessing for a specified order of the attack. Note, that this kind of context cannot be meaningfully merged.
*
*/
template <class T, class U, class V>
Moments2DContext<T> UniPrepHoCpaAddTraces(const PowerTraces<U>& pt, const PowerPredictions<V>& pp, size_t attackOrder) {
    
    if(attackOrder < 2) throw RuntimeException("Invalid attack order", attackOrder);

    const size_t noOfTraces = pt.noOfTraces();
    const size_t samplesPerTrace = pt.samplesPerTrace();
    
    Vector<T> avgs(samplesPerTrace, 0);
    Vector<T> vars(samplesPerTrace, 0);
    PowerTraces<T> ppt(samplesPerTrace, noOfTraces);
    
    // compute avg and var in every sample point
    for (long long trace = 0; trace < noOfTraces; trace++) {
        
        for (long long sample = 0; sample < samplesPerTrace; sample++) {
            
            T deltaT = static_cast<T>(pt(sample, trace)) - avgs(sample);
            
            avgs(sample) += deltaT / static_cast<T>(trace + 1);
            vars(sample) += ( (deltaT * deltaT) * static_cast<T>(trace) ) / static_cast<T>(trace + 1);
            
        }
        
    }
    
    for (long long sample = 0; sample < samplesPerTrace; sample++) {
        
        vars(sample) = std::sqrt(vars(sample));
        
    }
    
    // preprocess the power traces
    for (long long trace = 0; trace < noOfTraces; trace++) {
        
        for (long long sample = 0; sample < samplesPerTrace; sample++) {

            ppt(sample, trace) = std::pow(static_cast<T>(pt(sample, trace)) - avgs(sample), attackOrder); 
            
            if(attackOrder > 2) {
            
                ppt(sample, trace) /=  std::pow(vars(sample), attackOrder);
                
            }                        
            
        }
        
    }
    
    // create an empty first-order context
    Moments2DContext<T> context(pt.samplesPerTrace(), pp.noOfCandidates(), 1, 1, 2, 2, 1);
    context.reset();
    
    // Run first-order CPA on the preprocessed power traces set
    UniFoCpaAddTraces(context, ppt, pp);
    
    return context;
    
}

/**
*
* \brief Adds given power traces and power predictions to the given statistical context, upto specified attack order. Use zeroed or meaningful Moments2DContext c! Accelerated using OpenMP
*
*/
template <class T, class U, class V>
void UniHoCpaAddTraces(Moments2DContext<T>& c, const PowerTraces<U>& pt, const PowerPredictions<V>& pp, size_t attackOrder) {    
    
    if(c.p1MOrder() != 1 || c.p1CSOrder() != (2 * attackOrder) || c.p2CSOrder() != 2 || c.p12ACSOrder() != attackOrder || c.p1MOrder() != c.p2MOrder())
        throw RuntimeException("Not a valid higher-order univariate CPA context!", attackOrder);

    if (c.p1Width() != pt.samplesPerTrace())
        throw RuntimeException("Incompatible context: Numbers of samples per trace don't match.");

    if (c.p2Width() != pp.noOfCandidates())
        throw RuntimeException("Incompatible context: Numbers of key candidates don't match.");

    if (pt.noOfTraces() != pp.noOfTraces())
        throw RuntimeException("Number of power traces doesn't match the number of power predictions.");
    
    if(attackOrder < 1)
        throw RuntimeException("Invalid order of the attack.");

    const long long noOfTraces = pt.noOfTraces();
    const long long samplesPerTrace = pt.samplesPerTrace();
    const long long noOfCandidates = pp.noOfCandidates();            
    
    // precomputed values
    Matrix<T> deltaT(samplesPerTrace, 2 * attackOrder);
    Matrix<T> deltaL(noOfCandidates, 1);
    Matrix<T> minusDivN(2 * attackOrder, 1);
    Matrix<T> nCr(2*attackOrder + 1, 2 * attackOrder + 1, 0);    
    
    
    // precompute combination numbers        
    for(size_t n = 0; n <= 2*attackOrder; n++){
        nCr(n, 0) = 1;
        for(size_t r = 1; r <= n; r++){
            nCr(n, r) = (nCr(n, r-1) * (n - r + 1)) / r;                
        }
    }
        
    
    // Add every power trace
    for (long long trace = 0; trace < noOfTraces; trace++) {                
        
        T n = c.p1Card() + 1.0; // n = cardinality of the merged set
        T divN = 1.0 / n;
        
        {
            //  precompute deltaT
            T * p_deltaT = &(deltaT(0, 0));
            const U * p_pt = &(pt(0, trace));
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
            
            // also precompute deltaL
            T * p_deltaL = &(deltaL(0, 0));
            const V * p_pp = &(pp(0, trace));
            const T * p_predsAvg = &( c.p2M(1)(0) );
            for (long long candidate = 0; candidate < noOfCandidates; candidate++) {
                
                (*p_deltaL++) = static_cast<T>(*p_pp++) - (*p_predsAvg++);
                
            }
            
            // precompute (-1*divN)^d
            minusDivN(0,0) = (-1.0) * divN;            
            for (size_t order = 1; order < 2 * attackOrder; order++){
                
                minusDivN(order, 0) = minusDivN(order-1, 0) * minusDivN(0,0); 
                
            }                                    
            
        }
        
        // update ACSs
        for(long long deg = attackOrder; deg >= 1; deg--){
            
            const T p_beta = ( ( std::pow(-1.0, deg+1) * static_cast<T>(n-1) + std::pow(n-1, deg+1) ) / std::pow(n, deg+1) );
            
            #pragma omp parallel for
            for (long long candidate = 0; candidate < noOfCandidates; candidate++) {                                
                                                
                T * p_acs = &(c.p12ACS(deg)(0, candidate));
                const T * p_deltaT = &( deltaT(0, deg - 1) );
                const T * p_cs = (deg >= 2) ? &(c.p1CS(deg)(0)) : nullptr; // be sure not to dereference, unless deg is >= 2 !!!
                const T p_alpha = p_beta * deltaL(candidate, 0);                                 
                const T p_gamma = ( (-1.0) * ( deltaL(candidate, 0) * divN ) );                
                
                for (long long sample = 0; sample < samplesPerTrace; sample++) {                                        
                                                                            
                    (*p_acs) += (p_alpha * (*p_deltaT++));
                    (*p_acs) += (deg >= 2) ? (p_gamma * (*p_cs++)) : 0;
                                                    
                    p_acs++;                                    
                
                }
                
                for(long long p = 1; p <= deg - 1; p++){
                    
                    p_acs = &(c.p12ACS(deg)(0, candidate));
                    const T * p_deltaTPow = &( deltaT(0, p - 1) );
                    const T * p_lessAcs = &(c.p12ACS(deg - p)(0, candidate));
                    const T * p_lessCs = (deg-p >= 2) ? &(c.p1CS(deg-p)(0)) : nullptr;
                    const T p_delta = minusDivN(p - 1, 0) * nCr(deg, p);
                                        
                    for (long long sample = 0; sample < samplesPerTrace; sample++) {
                                                    
                        T sumTerm = (*p_lessAcs++);
                        sumTerm += (deg-p >= 2) ? ( p_gamma * (*p_lessCs++) ) : 0;
                        
                        sumTerm *= p_delta * (*p_deltaTPow++);                                                                        
                        
                        (*p_acs++) += sumTerm;
                        
                    }
                                            
                }
                
            }                                                                                                            
            
        }        

        // update traces CSs        
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
        
        // update predictions CSs      
        for (long long candidate = 0; candidate < noOfCandidates; candidate++) {
            
            T deltaL2 = static_cast<T>(pp(candidate, trace)) - c.p2M(1)(candidate);
            
            c.p2CS(2)(candidate) += ((deltaL2 * deltaL2) * (n-1.0)) * divN;
            
        }
        
        // update Ms        
        for (long long sample = 0; sample < samplesPerTrace; sample++) {
            
            T deltaT2 = static_cast<T>(pt(sample, trace)) - c.p1M(1)(sample);
            
            c.p1M(1)(sample) += deltaT2 * divN;            
            
        }
        
        for (long long candidate = 0; candidate < noOfCandidates; candidate++) {
            
            T deltaL2 = static_cast<T>(pp(candidate, trace)) - c.p2M(1)(candidate);
            
            c.p2M(1)(candidate) += deltaL2 * divN;
            
        }

        // update Card
        c.p1Card() = c.p1Card() + 1;

    }
    
    c.p2Card() = c.p1Card();

}


/**
*
* \brief Merges two Moments2DContext and leaves the result in first context given
*
*/
template <class T>
void UniHoCpaMergeContexts(Moments2DContext<T>& firstAndOut, const Moments2DContext<T>& second) {
    
    if(    firstAndOut.p1MOrder() != 1 
        || firstAndOut.p2MOrder() != 1
        || firstAndOut.p12ACSOrder() < 1
        || firstAndOut.p1CSOrder() != 2 * firstAndOut.p12ACSOrder()
        || firstAndOut.p2CSOrder() != 2
        || second.p1MOrder() != 1 
        || second.p2MOrder() != 1
        || second.p12ACSOrder() != firstAndOut.p12ACSOrder()
        || second.p1CSOrder() != firstAndOut.p1CSOrder()
        || second.p2CSOrder() != 2
    )
        throw RuntimeException("Not valid higher-order univariate CPA contexts!");
    
    if(firstAndOut.p1Card() != firstAndOut.p2Card() || second.p1Card() != second.p2Card())
        throw RuntimeException("Malformed CPA context");
    
    if(firstAndOut.p1Width() != second.p1Width() || firstAndOut.p2Width() != second.p2Width())
        throw RuntimeException("Only contexts with same number of candidates and same number of samples per trace can be merged");
        
    const size_t samplesPerTrace = firstAndOut.p1Width();
    const size_t noOfCandidates = firstAndOut.p2Width();
    
    const T n1 = firstAndOut.p1Card();
    const T n2 = second.p1Card();
    
    if(n1 == 0 || n2 == 0) throw RuntimeException("Empty context");
    
    const size_t acsOrder = firstAndOut.p12ACSOrder();
    const size_t csOrder = firstAndOut.p1CSOrder();
    
    // precompute delta-s
    Vector<T> deltaT(samplesPerTrace);
    Vector<T> deltaL(noOfCandidates);
    
    for(size_t sample = 0; sample < samplesPerTrace; sample++) {                
        
        deltaT(sample) = second.p1M(1)(sample) - firstAndOut.p1M(1)(sample);
        
    }    
    
    for(size_t candidate = 0; candidate < noOfCandidates; candidate++){
        
        deltaL(candidate) = second.p2M(1)(candidate) - firstAndOut.p2M(1)(candidate);
        
    }
    
    // precompute combination numbers
    Matrix<T> nCr(csOrder + 1, csOrder + 1, 0);    
        
    for(size_t n = 0; n <= csOrder; n++){
        nCr(n, 0) = 1;
        for(size_t r = 1; r <= n; r++){
            nCr(n, r) = (nCr(n, r-1) * (n - r + 1)) / r;                
        }
    }

    
    // merge the ACSs
    for(size_t deg = acsOrder; deg >= 1; deg--){
            
        for(size_t candidate = 0; candidate < noOfCandidates; candidate++){
            
            const T p_alpha = ( n1 * std::pow( (-1.0) * n2 , deg+1) + n2 * std::pow(n1, deg+1) ) / std::pow(n1+n2, deg+1);
        
            for(size_t sample = 0; sample < samplesPerTrace; sample++) {                
                
                firstAndOut.p12ACS(deg)(sample, candidate) += second.p12ACS(deg)(sample, candidate); 
                
                if(deg > 1) {
                    
                    firstAndOut.p12ACS(deg)(sample, candidate) += ( deltaL(candidate) / (n1+n2) ) 
                                                                * ( n1 * second.p1CS(deg)(sample) - n2 * firstAndOut.p1CS(deg)(sample) );
                                                            
                }
                
                firstAndOut.p12ACS(deg)(sample, candidate) += std::pow(deltaT(sample), deg) * deltaL(candidate) 
                                                            * ( p_alpha );                                                                            
                
            }
            
            
            for(size_t p = 1; p <= deg - 1; p++){    
                
                const T p_beta = std::pow( (-1.0) * n2, p+1 );
                const T p_gamma = std::pow( n1, p+1 );
                const T p_delta = std::pow( (-1.0) * n2, p );
                const T p_phi = std::pow( n1, p );
                
                for(size_t sample = 0; sample < samplesPerTrace; sample++) { 
                    
                    T sumTerm = 0;
                    
                    if(deg-p >= 2) {
                        
                        sumTerm += (  ( p_beta * firstAndOut.p1CS(deg-p)(sample) )
                                    + ( p_gamma * second.p1CS(deg-p)(sample) ) 
                                   ) 
                                 * ( deltaL(candidate) / (n1+n2) );
                        
                    }
                    
                    sumTerm += p_delta * firstAndOut.p12ACS(deg-p)(sample, candidate);
                    
                    sumTerm += p_phi * second.p12ACS(deg-p)(sample, candidate);
                    
                    sumTerm *= std::pow( deltaT(sample) / (n1+n2) , p );
                    
                    sumTerm *= nCr(deg, p);
                    
                    firstAndOut.p12ACS(deg)(sample, candidate) += sumTerm;
            
                }
                    
            }
            
        }
        
    }
    
    // then merge the CSs
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
    
    for(size_t candidate = 0; candidate < noOfCandidates; candidate++){
        
        firstAndOut.p2CS(2)(candidate) += second.p2CS(2)(candidate);  
        
        firstAndOut.p2CS(2)(candidate) += (n1*n2) * std::pow( deltaL(candidate) / (n1+n2) , 2) * (n1+n2);
                
    }
            
    
    // then merge the Means
    for(size_t sample = 0; sample < samplesPerTrace; sample++) {        
        firstAndOut.p1M(1)(sample) = ( (firstAndOut.p1M(1)(sample) * n1) + (second.p1M(1)(sample) * n2) ) / (n1 + n2);
    }
    
    for(size_t candidate = 0; candidate < noOfCandidates; candidate++){
        firstAndOut.p2M(1)(candidate) = ( (firstAndOut.p2M(1)(candidate) * n1) + (second.p2M(1)(candidate) * n2) ) / (n1 + n2);
    }        
    
    // finally update the cardinality of the context
    firstAndOut.p1Card() += second.p1Card();
    firstAndOut.p2Card() = firstAndOut.p1Card();
    
}


/**
*
* \brief Computes final correlation matrix based on a Moments2DContext given, stores results in correlations
*
*/
template <class T>
void UniHoCpaComputeCorrelationMatrix(const Moments2DContext<T> & c, MatrixType<T> & correlations, size_t attackOrder){
    
    if(c.p1MOrder() != 1 || c.p1CSOrder() < attackOrder * 2 || c.p2CSOrder() != 2 || c.p12ACSOrder() < attackOrder || c.p1MOrder() != c.p2MOrder() || c.p1Card() != c.p2Card())
        throw RuntimeException("Not a valid higher-order univariate CPA context!", attackOrder);

    if(attackOrder < 1)
        throw RuntimeException("Invalid order of the attack.", attackOrder);
    
    size_t samplesPerTrace = c.p1Width();
    size_t noOfCandidates = c.p2Width();
    
    correlations.init(samplesPerTrace, noOfCandidates);
    
    T n = c.p1Card();
    
    if(n == 0) throw RuntimeException("Empty context.");
    
    T divN = 1.0 / n;
    
    Vector<T> sqrtTracesCS(samplesPerTrace);
    Vector<T> sqrtPredsCS(noOfCandidates);   
    
    // predictions variance
    for (size_t candidate = 0; candidate < noOfCandidates; candidate++) {
        sqrtPredsCS(candidate) = std::sqrt(divN * c.p2CS(2)(candidate));
    }

    // traces variance
    if(attackOrder == 1){
    
        for (size_t sample = 0; sample < samplesPerTrace; sample++) {
            sqrtTracesCS(sample) = std::sqrt(divN * c.p1CS(2)(sample));
        }        
        
    } else { // higher
        
        for (size_t sample = 0; sample < samplesPerTrace; sample++) {
            sqrtTracesCS(sample) = std::sqrt( c.p1CS(attackOrder*2)(sample) - ( std::pow(c.p1CS(attackOrder)(sample), 2) * divN) );
        }   
        
    }                

    for (size_t candidate = 0; candidate < noOfCandidates; candidate++) {
            
        for (size_t sample = 0; sample < samplesPerTrace; sample++) {

            if (sqrtTracesCS(sample) == 0 || sqrtPredsCS(candidate) == 0) throw RuntimeException("Division by zero");            
            
            correlations(sample, candidate) = (divN * c.p12ACS(attackOrder)(sample, candidate)) / (sqrtTracesCS(sample) * sqrtPredsCS(candidate));

        }
        
    }
    
}


#endif /* OMPCPA_H */
 

