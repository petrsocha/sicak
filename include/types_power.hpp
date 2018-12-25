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
* \file types_power.hpp
*
* \brief This header file contains class templates of power traces and power consumption containers
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef TYPES_POWER_HPP
#define TYPES_POWER_HPP

#include "types_basic.hpp"



/**
* \class PowerTraces
* \ingroup SicakData
*
* \brief A class representing a Matrix with 'noOfTraces' power traces, with 'samplesPerTrace' samples per power trace
*
*/
template <class T>
class PowerTraces : public Matrix<T> {

public:
        /// Constructs an empty Matrix with no elements. Needs to be initialized first (init).
        PowerTraces() {}
        /// Constructs a Matrix with 'samplesPerTrace' * 'noOfTraces' elements
        PowerTraces(size_t samplesPerTrace, size_t noOfTraces) : Matrix<T>(samplesPerTrace, noOfTraces) {}
        /// Constructs a Matrix with 'samplesPerTrace' * 'noOfTraces' elements and fills it with 'initVal'
        PowerTraces(size_t samplesPerTrace, size_t noOfTraces, T initVal) : Matrix<T>(samplesPerTrace, noOfTraces, initVal) {}

        /// Move constructor
        PowerTraces(PowerTraces&& other) : Matrix<T>(std::move(other)) { }
        /// Move assignment operator
        PowerTraces& operator=(PowerTraces&& other) { return static_cast<PowerTraces&>(Matrix<T>::operator=(std::move(other))); }

        /// Empty destructor
        virtual ~PowerTraces() {}

        virtual void   init(size_t samplesPerTrace, size_t noOfTraces) { return Matrix<T>::init(samplesPerTrace, noOfTraces); }

        /// Returns number of samples per trace
        virtual size_t samplesPerTrace() const { return (*this).cols(); }
        /// Returns number of power traces
        virtual size_t noOfTraces() const { return (*this).rows(); }

        virtual         T & operator()       (size_t sample, size_t trace) { return Matrix<T>::operator()(sample, trace); }
        virtual const   T & operator()       (size_t sample, size_t trace) const { return Matrix<T>::operator()(sample, trace); }
};


/**
* \class PowerPredictions
* \ingroup SicakData
*
* \brief A class representing a Matrix with 'noOfTraces' power predictions, with 'noOfCandidates' key candidates per prediction
*
*/
template <class T>
class PowerPredictions : public Matrix<T> {

public:
        /// Constructs an empty Matrix with no elements. Needs to be initialized first (init).
        PowerPredictions() {}
        /// Constructs a Matrix with 'noOfCandidates' * 'noOfTraces' elements
        PowerPredictions(size_t noOfCandidates, size_t noOfTraces) : Matrix<T>(noOfCandidates, noOfTraces) {}
        /// Constructs a Matrix with 'noOfCandidates' * 'noOfTraces' elements and fills it with 'initVal'
        PowerPredictions(size_t noOfCandidates, size_t noOfTraces, T initVal) : Matrix<T>(noOfCandidates, noOfTraces, initVal) {}

        /// Move constructor
        PowerPredictions(PowerPredictions&& other) : Matrix<T>(std::move(other)) { }
        /// Move assignment operator
        PowerPredictions& operator=(PowerPredictions&& other) { return static_cast<PowerPredictions&>(Matrix<T>::operator=(std::move(other))); }

        /// Empty destructor
        ~PowerPredictions() {}

        virtual void   init(size_t noOfCandidates, size_t noOfTraces) { return Matrix<T>::init(noOfCandidates, noOfTraces); }

        /// Returns number of key candidates per power prediction
        virtual size_t noOfCandidates() const { return (*this).cols(); }
        /// Returns number of power predictions
        virtual size_t noOfTraces() const { return (*this).rows(); }

        virtual         T & operator()       (size_t keyCandidate, size_t trace) { return Matrix<T>::operator()(keyCandidate, trace); }
        virtual const   T & operator()       (size_t keyCandidate, size_t trace) const { return Matrix<T>::operator()(keyCandidate, trace); }
};


#endif /* TYPES_POWER_HPP */

