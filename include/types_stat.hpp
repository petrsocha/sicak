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
* \file types_stat.hpp
*
* \brief This header file contains class templates of statistical computational contexts
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef TYPES_STAT_HPP
#define TYPES_STAT_HPP

#include "types_basic.hpp"


/**
* \class UnivariateContext
* \ingroup SicakData
*
* \brief A class representing a Two-population Univariate Moment-based statistical context
*
*/
template <class T>
class UnivariateContext : public ComputationalContext<T> {
  
public:
    
    /// Constructs an empty context, needs to be initialized first (init)
    UnivariateContext():
        m_p1Width(0), m_p2Width(0), m_p1Card(0), m_p2Card(0), m_mOrder(0), m_csOrder(0), m_acsOrder(0),
        m_p1M(nullptr), m_p2M(nullptr), m_p1CS(nullptr), m_p2CS(nullptr), m_p12ACS(nullptr)
    {}
    
    /// Constructs an initialized context
    UnivariateContext(size_t firstWidth, size_t secondWidth, size_t mOrder, size_t csOrder, size_t acsOrder):
        m_p1Width(0), m_p2Width(0), m_p1Card(0), m_p2Card(0), m_mOrder(0), m_csOrder(0), m_acsOrder(0),
        m_p1M(nullptr), m_p2M(nullptr), m_p1CS(nullptr), m_p2CS(nullptr), m_p12ACS(nullptr){
            
        (*this).init(firstWidth, secondWidth, mOrder, csOrder, acsOrder);
        
    }
    
    /// Constructs an initialized context and fills it with val
    UnivariateContext(size_t firstWidth, size_t secondWidth, size_t mOrder, size_t csOrder, size_t acsOrder, T val):
        m_p1Width(0), m_p2Width(0), m_p1Card(0), m_p2Card(0), m_mOrder(0), m_csOrder(0), m_acsOrder(0),
        m_p1M(nullptr), m_p2M(nullptr), m_p1CS(nullptr), m_p2CS(nullptr), m_p12ACS(nullptr){
            
        (*this).init(firstWidth, secondWidth, mOrder, csOrder, acsOrder);
        (*this).fill(val);
        
    }
    
    /// Move constructor
    UnivariateContext(UnivariateContext&& other): m_p1Width(other.m_p1Width), m_p2Width(other.m_p2Width), 
                                                  m_p1Card(other.m_p1Card), m_p2Card(other.m_p2Card),
                                                  m_mOrder(other.m_mOrder), m_csOrder(other.m_csOrder), m_acsOrder(other.m_acsOrder),
                                                  m_p1M(std::move(other.m_p1M)), m_p2M(std::move(other.m_p2M)), 
                                                  m_p1CS(std::move(other.m_p1CS)), m_p2CS(std::move(other.m_p2CS)), 
                                                  m_p12ACS(std::move(other.m_p12ACS)) 
                                                  {}
    
    /// Move assignment operator
    UnivariateContext& operator=(UnivariateContext&& other) {
            m_p1Width = other.m_p1Width;
            m_p2Width = other.m_p2Width;
            m_p1Card = other.m_p1Card;
            m_p2Card = other.m_p2Card;
            m_mOrder = other.m_mOrder;
            m_csOrder = other.m_csOrder;
            m_acsOrder = other.m_acsOrder;
            m_p1M = std::move(other.m_p1M);
            m_p2M = std::move(other.m_p2M);
            m_p1CS = std::move(other.m_p1CS);
            m_p2CS = std::move(other.m_p2CS);
            m_p12ACS = std::move(other.m_p12ACS);            
            return (*this);
    }

    /// Empty destructor
    virtual ~UnivariateContext() {}
    
    virtual void init(size_t firstWidth, size_t secondWidth, size_t mOrder, size_t csOrder, size_t acsOrder) {
        
        if(firstWidth == m_p1Width && secondWidth == m_p2Width && mOrder == m_mOrder && csOrder == m_csOrder && acsOrder == m_acsOrder)
            return; // already there, nothing to do
        
        try{
            
            m_p1Width = firstWidth;
            m_p1Card = 0;
            m_p2Width = secondWidth;
            m_p2Card = 0;
            m_mOrder = mOrder;
            m_csOrder = csOrder;
            m_acsOrder = acsOrder;            
            
            // M
            m_p1M.reset(new Vector<T>[mOrder]);
            m_p2M.reset(new Vector<T>[mOrder]);
            for(size_t order = 0; order < mOrder; order++){
                m_p1M[order].init(firstWidth);
                m_p2M[order].init(secondWidth);
            }
            
            // CS            
            size_t csLen = (m_csOrder > 1) ? m_csOrder - 1 : 0; // 1st order CS is a constant '0', dont allocate a Vector for that   
            m_p1CS.reset(new Vector<T>[csLen]);
            m_p2CS.reset(new Vector<T>[csLen]);
            for(size_t order = 0; order < csLen; order++){
                m_p1CS[order].init(firstWidth);
                m_p2CS[order].init(secondWidth);
            }
            
            // ACS
            m_p12ACS.reset(new Matrix<T>[acsOrder]);
            for(size_t order = 0; order < acsOrder; order++){
                m_p12ACS[order].init(firstWidth, secondWidth);
            }
            
        } catch (std::bad_alloc & e) {
            throw RuntimeException("Context memory allocation failed"); 
        } 
    }
    
    virtual void init(size_t firstWidth, size_t secondWidth, size_t mOrder, size_t csOrder, size_t acsOrder, T val) {
        (*this).init(firstWidth, secondWidth, mOrder, csOrder, acsOrder);
        (*this).fill(val);
    }
    
    virtual void fill(T val) {
        
        for(size_t order = 0; order < m_mOrder; order++){
            m_p1M[order].fill(val);
            m_p2M[order].fill(val);
        }
        
        size_t csLen = (m_csOrder > 1) ? m_csOrder - 1 : 0;
        for(size_t order = 0; order < csLen; order++){
            m_p1CS[order].fill(val);
            m_p2CS[order].fill(val);
        }
        
        for(size_t order = 0; order < m_acsOrder; order++){
            m_p12ACS[order].fill(val);
        }
                
    }
    
    /// Fill the whole context with zeroes
    virtual void reset() {
        
        (*this).fill(0);
        m_p1Card = 0;
        m_p2Card = 0;
        
    }
    
    /// Width of the first population
    virtual size_t p1Width() const { return m_p1Width; }
    /// Width of the second population
    virtual size_t p2Width() const { return m_p2Width; }
    
    /// Maximum order of the raw moments, 1 upto mOrder
    virtual size_t mOrder() const { return m_mOrder; }
    /// Maximum order of the central moment sums, 2 upto csOrder
    virtual size_t csOrder() const { return m_csOrder; }
    /// Maximum order of the adjusted central moment sums, 1 upto acsOrder
    virtual size_t acsOrder() const { return m_acsOrder; }
        
    /// Cardinality of the first population
    virtual          size_t    & p1Card() { return m_p1Card; }
    /// Cardinality of the first population (const)
    virtual  const   size_t    & p1Card() const { return m_p1Card; }
    
    /// Cardinality of the second population
    virtual          size_t    & p2Card() { return m_p2Card; }
    /// Cardinality of the second population (const)
    virtual  const   size_t    & p2Card() const { return m_p2Card; }
    
    /// Raw moment of the first population, order 1 upto mOrder
    virtual          Vector<T> & p1M(size_t order)       { return m_p1M[order-1]; }
    /// Raw moment of the first population, order 1 upto mOrder (const)
    virtual  const   Vector<T> & p1M(size_t order) const { return m_p1M[order-1]; }
    
    /// Raw moment of the second population, order 1 upto mOrder
    virtual          Vector<T> & p2M(size_t order)       { return m_p2M[order-1]; }
    /// Raw moment of the second population, order 1 upto mOrder (const)
    virtual  const   Vector<T> & p2M(size_t order) const { return m_p2M[order-1]; }
    
    /// Central moment sum of the first population, order 2 upto csOrder
    virtual          Vector<T> & p1CS(size_t order)       { return m_p1CS[order-2]; }
    /// Central moment sum of the first population, order 2 upto csOrder (const)
    virtual  const   Vector<T> & p1CS(size_t order) const { return m_p1CS[order-2]; }
    
    /// Central moment sum of the second population, order 2 upto csOrder
    virtual          Vector<T> & p2CS(size_t order)       { return m_p2CS[order-2]; }
    /// Central moment sum of the second population, order 2 upto csOrder (const)
    virtual  const   Vector<T> & p2CS(size_t order) const { return m_p2CS[order-2]; }
    
    /// Adjusted central moment sum both populations, order 1 upto acsOrder
    virtual          Matrix<T> & p12ACS(size_t order)       { return m_p12ACS[order-1]; }
    /// Adjusted central moment sum both populations, order 1 upto acsOrder (const)
    virtual  const   Matrix<T> & p12ACS(size_t order) const { return m_p12ACS[order-1]; }
    
    
protected:
    
    size_t m_p1Width;
    size_t m_p2Width;
    
    size_t m_p1Card;
    size_t m_p2Card;
    
    size_t m_mOrder;
    size_t m_csOrder;
    size_t m_acsOrder;        
    
    std::unique_ptr<Vector<T>[]> m_p1M;
    std::unique_ptr<Vector<T>[]> m_p2M;
    
    std::unique_ptr<Vector<T>[]> m_p1CS;
    std::unique_ptr<Vector<T>[]> m_p2CS;
    
    std::unique_ptr<Matrix<T>[]> m_p12ACS;
};


#endif /* TYPES_STAT_HPP */
