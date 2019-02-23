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
* \file types_stat.hpp
*
* \brief This header file contains class templates of statistical computational contexts
*
*
* \author Petr Socha
* \version 1.1
*/

#ifndef TYPES_STAT_HPP
#define TYPES_STAT_HPP

#include "types_basic.hpp"


/**
* \class Moments2DContext
* \ingroup SicakData
*
* \brief A class representing a Two-population Univariate Moment-based statistical context
*
*/
template <class T>
class Moments2DContext : public ComputationalContext<T> {
    
#define Moments2DContext_iid "cz.cvut.fit.Sicak.Moments2DContext/1.1"
  
public:        
    
    /// Constructs an empty context, needs to be initialized first (init)
    Moments2DContext():
        m_p1Width(0), m_p2Width(0), m_p1Card(0), m_p2Card(0), 
        m_p1MOrder(0), m_p2MOrder(0), m_p1CSOrder(0), m_p2CSOrder(0), m_p12ACSOrder(0),
        m_p1M(nullptr), m_p2M(nullptr), m_p1CS(nullptr), m_p2CS(nullptr), m_p12ACS(nullptr)
    {}
    
    /// Constructs an initialized context
    Moments2DContext(size_t p1Width, size_t p2Width, size_t p1MOrder, size_t p2MOrder, size_t p1CSOrder, size_t p2CSOrder, size_t p12ACSOrder):
        m_p1Width(0), m_p2Width(0), m_p1Card(0), m_p2Card(0), 
        m_p1MOrder(0), m_p2MOrder(0), m_p1CSOrder(0), m_p2CSOrder(0), m_p12ACSOrder(0),
        m_p1M(nullptr), m_p2M(nullptr), m_p1CS(nullptr), m_p2CS(nullptr), m_p12ACS(nullptr)
    {
            
        (*this).init(p1Width, p2Width, p1MOrder, p2MOrder, p1CSOrder, p2CSOrder, p12ACSOrder);
        
    }
    
    /// Constructs an initialized context and fills it with val
    Moments2DContext(size_t p1Width, size_t p2Width, size_t p1MOrder, size_t p2MOrder, size_t p1CSOrder, size_t p2CSOrder, size_t p12ACSOrder, T val):
        m_p1Width(0), m_p2Width(0), m_p1Card(0), m_p2Card(0), 
        m_p1MOrder(0), m_p2MOrder(0), m_p1CSOrder(0), m_p2CSOrder(0), m_p12ACSOrder(0),
        m_p1M(nullptr), m_p2M(nullptr), m_p1CS(nullptr), m_p2CS(nullptr), m_p12ACS(nullptr)
    {
            
        (*this).init(p1Width, p2Width, p1MOrder, p2MOrder, p1CSOrder, p2CSOrder, p12ACSOrder);
        (*this).fill(val);
        
    }
    
    /// Move constructor
    Moments2DContext(Moments2DContext&& other): m_p1Width(other.m_p1Width), m_p2Width(other.m_p2Width), 
                                                  m_p1Card(other.m_p1Card), m_p2Card(other.m_p2Card),
                                                  m_p1MOrder(other.m_p1MOrder), m_p2MOrder(other.m_p2MOrder), 
                                                  m_p1CSOrder(other.m_p1CSOrder), m_p2CSOrder(other.m_p2CSOrder), 
                                                  m_p12ACSOrder(other.m_p12ACSOrder),
                                                  m_p1M(std::move(other.m_p1M)), m_p2M(std::move(other.m_p2M)), 
                                                  m_p1CS(std::move(other.m_p1CS)), m_p2CS(std::move(other.m_p2CS)), 
                                                  m_p12ACS(std::move(other.m_p12ACS)) 
                                                  {}
    
    /// Move assignment operator
    Moments2DContext& operator=(Moments2DContext&& other) {
            m_p1Width = other.m_p1Width;
            m_p2Width = other.m_p2Width;
            m_p1Card = other.m_p1Card;
            m_p2Card = other.m_p2Card;
            m_p1MOrder = other.m_p1MOrder;
            m_p2MOrder = other.m_p2MOrder;
            m_p1CSOrder = other.m_p1CSOrder;
            m_p2CSOrder = other.m_p2CSOrder;
            m_p12ACSOrder = other.m_p12ACSOrder;            
            m_p1M = std::move(other.m_p1M);
            m_p2M = std::move(other.m_p2M);
            m_p1CS = std::move(other.m_p1CS);
            m_p2CS = std::move(other.m_p2CS);
            m_p12ACS = std::move(other.m_p12ACS);            
            return (*this);
    }

    /// Empty destructor
    virtual ~Moments2DContext() {}
    
    virtual void init(size_t p1Width, size_t p2Width, size_t p1MOrder, size_t p2MOrder, size_t p1CSOrder, size_t p2CSOrder, size_t p12ACSOrder) {
        
        if(p1Width == m_p1Width && p2Width == m_p2Width && p1MOrder == m_p1MOrder && p2MOrder == m_p2MOrder && p1CSOrder == m_p1CSOrder && p2CSOrder == m_p2CSOrder && p12ACSOrder == m_p12ACSOrder)
            return; // already there, nothing to do
        
        try{
            
            m_p1Width = p1Width;
            m_p1Card = 0;
            m_p2Width = p2Width;
            m_p2Card = 0;
            m_p1MOrder = p1MOrder;
            m_p2MOrder = p2MOrder;
            m_p1CSOrder = p1CSOrder;
            m_p2CSOrder = p2CSOrder;
            m_p12ACSOrder = p12ACSOrder;            
            
            // M
            m_p1M.reset(new Vector<T>[m_p1MOrder]);
            m_p2M.reset(new Vector<T>[m_p2MOrder]);
            
            for(size_t order = 0; order < m_p1MOrder; order++){
                m_p1M[order].init(p1Width);
            }
            
            for(size_t order = 0; order < m_p2MOrder; order++){
                m_p2M[order].init(p2Width);
            }
            
            
            // CS            
            size_t p1CSLen = (m_p1CSOrder > 1) ? m_p1CSOrder - 1 : 0; // 1st order CS is a constant '0', dont allocate a Vector for that   
            size_t p2CSLen = (m_p2CSOrder > 1) ? m_p2CSOrder - 1 : 0; // 1st order CS is a constant '0', dont allocate a Vector for that   
            
            m_p1CS.reset(new Vector<T>[p1CSLen]);
            m_p2CS.reset(new Vector<T>[p2CSLen]);
            
            for(size_t order = 0; order < p1CSLen; order++){
                m_p1CS[order].init(p1Width);
            }
            
            for(size_t order = 0; order < p2CSLen; order++){
                m_p2CS[order].init(p2Width);
            }
            
            
            // ACS
            m_p12ACS.reset(new Matrix<T>[m_p12ACSOrder]);
            
            for(size_t order = 0; order < m_p12ACSOrder; order++){
                m_p12ACS[order].init(p1Width, p2Width);
            }
            
        } catch (std::bad_alloc & e) {
            throw RuntimeException("Context memory allocation failed"); 
        } 
    }
    
    virtual void init(size_t p1Width, size_t p2Width, size_t p1MOrder, size_t p2MOrder, size_t p1CSOrder, size_t p2CSOrder, size_t p12ACSOrder, T val) {
        (*this).init(p1Width, p2Width, p1MOrder, p2MOrder, p1CSOrder, p2CSOrder, p12ACSOrder);
        (*this).fill(val);
    }
    
    virtual void fill(T val) {                
        
        // M
        for(size_t order = 0; order < m_p1MOrder; order++){
            m_p1M[order].fill(val);
        }
        
        for(size_t order = 0; order < m_p2MOrder; order++){
            m_p2M[order].fill(val);
        }
                
        // CS            
        size_t p1CSLen = (m_p1CSOrder > 1) ? m_p1CSOrder - 1 : 0; // 1st order CS is a constant '0', dont allocate a Vector for that   
        size_t p2CSLen = (m_p2CSOrder > 1) ? m_p2CSOrder - 1 : 0; // 1st order CS is a constant '0', dont allocate a Vector for that   
        
        for(size_t order = 0; order < p1CSLen; order++){
            m_p1CS[order].fill(val);
        }
        
        for(size_t order = 0; order < p2CSLen; order++){
            m_p2CS[order].fill(val);
        }
                
        // ACS        
        for(size_t order = 0; order < m_p12ACSOrder; order++){
            m_p12ACS[order].fill(val);
        }        
                
    }
    
    /// Fill the whole context with zeroes
    virtual void reset() {
        
        (*this).fill(0);
        m_p1Card = 0;
        m_p2Card = 0;
        
    }
    
    virtual const char * getId() const { return Moments2DContext_iid; }
    
    /// Width of the first population
    virtual size_t p1Width() const { return m_p1Width; }
    /// Width of the second population
    virtual size_t p2Width() const { return m_p2Width; }
    
    /// Maximum order of the raw moments, 1 upto mOrder
    virtual size_t p1MOrder() const { return m_p1MOrder; }
    
    /// Maximum order of the raw moments, 1 upto mOrder
    virtual size_t p2MOrder() const { return m_p2MOrder; }
    
    /// Maximum order of the central moment sums, 2 upto csOrder
    virtual size_t p1CSOrder() const { return m_p1CSOrder; }
    
    /// Maximum order of the central moment sums, 2 upto csOrder
    virtual size_t p2CSOrder() const { return m_p2CSOrder; }
    
    /// Maximum order of the adjusted central moment sums, 1 upto acsOrder
    virtual size_t p12ACSOrder() const { return m_p12ACSOrder; }
        
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
    
    size_t m_p1MOrder;
    size_t m_p2MOrder;
    
    size_t m_p1CSOrder;
    size_t m_p2CSOrder;
    
    size_t m_p12ACSOrder;        
    
    std::unique_ptr<Vector<T>[]> m_p1M;
    std::unique_ptr<Vector<T>[]> m_p2M;
    
    std::unique_ptr<Vector<T>[]> m_p1CS;
    std::unique_ptr<Vector<T>[]> m_p2CS;
    
    std::unique_ptr<Matrix<T>[]> m_p12ACS;
};


#endif /* TYPES_STAT_HPP */
