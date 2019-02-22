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
* \file types_basic.hpp
*
* \brief This header file contains class templates of basic data containers
*
*
* \author Petr Socha
* \version 1.1
*/

#ifndef TYPES_BASIC_HPP
#define TYPES_BASIC_HPP

#include <algorithm>
#include <memory>
#include <utility>
#include "exceptions.hpp"

/**
* \class DataType
* \ingroup SicakData
*
* \brief A base abstract class, representing all the data types.
*
*/
class DataType {

public:

        /// Empty destructor
        virtual ~DataType() {}

protected:

        /// Protected constructor
        DataType() {}
        
};


/**
* \class ArrayType
* \ingroup SicakData
*
* \brief An abstract class, representing all the array-like data types
*
*/
template <class T>
class ArrayType : public DataType {

public:

        /// Empty destructor
        virtual              ~ArrayType() {}
        /// Returns number of elements in the container
        virtual size_t length() const = 0;
        /// Returns the size of the contained data (i.e. length * sizeof(T))
        virtual size_t size() const = 0;
        /// Fills the container with the 'val'
        virtual       void   fill(T val) = 0;
        /// Returns a pointer to the contained data
        virtual       T *    data() = 0;
        /// Returns a const pointer to the contained data
        virtual const T *    data() const = 0;

protected:

        /// Protected constructor
        ArrayType() {}

};


/**
* \class VectorType
* \ingroup SicakData
*
* \brief An abstract class, representing all the vector-like data types
*
*/
template <class T>
class VectorType : public ArrayType<T> {

public:

        /// Empty destructor
        virtual                ~VectorType() {}
        /// Initializes the vector with a specified number of elements
        virtual         void   init(size_t length) = 0;
        /// Initializes the vector with a specified number of elements and fills it with 'val'
        virtual         void   init(size_t length, T initVal) = 0;

        /// Accesses an element in the vector. Doesn't check for bounds.
        virtual         T &    operator()       (size_t index) = 0;
        /// Accesses an element in the vector. Doesn't check for bounds.
        virtual const   T &    operator()       (size_t index)                   const = 0;

protected:

        /// Protected constructor
        VectorType() {}        
        
};


/**
* \class MatrixType
* \ingroup SicakData
*
* \brief An abstract class, representing all the matrix-like data types
*
*/
template <class T>
class MatrixType : public ArrayType<T> {

public:

        /// Empty destructor
        virtual ~MatrixType() {}

        /// Returns number of columns
        virtual size_t cols() const = 0;
        /// Returns number of rows
        virtual size_t rows() const = 0;

        /// Initializes the matrix with a specified number of cols and rows
        virtual void   init(size_t cols, size_t rows) = 0;
        /// Initializes the matrix with a specified number of cols and rows and fills it with 'val'
        virtual void   init(size_t cols, size_t rows, T initVal) = 0;

        /// Vertically shrinks the matrix, with remaining elements and addressing left intact
        virtual void   shrinkRows(size_t rows) = 0;
        
        /// Accesses an element in the matrix. Doesn't check for bounds.
        virtual         T & operator()       (size_t col, size_t row) = 0;
        /// Accesses an element in the matrix. Doesn't check for bounds.
        virtual const   T & operator()       (size_t col, size_t row)  const = 0;

protected:

        /// Protected constructor
        MatrixType() {};        
        
};


/**
* \class StructuredType
* \ingroup SicakData
*
* \brief An abstract class, representing all the structured data types
*
*/
class StructuredType : public DataType {

public:

        /// Empty destructor
        virtual ~StructuredType() {}

protected:

        /// Protected constructor
        StructuredType() {}        
        
};


/**
* \class ComputationalContext
* \ingroup SicakData
*
* \brief An abstract class, representing all the computational contexts
*
*/
template <class T>
class ComputationalContext : public StructuredType {

protected:

        /// Protected constructor
        ComputationalContext() {}

public:

        /// Empty destructor
        virtual ~ComputationalContext() {}
        /// Fills the context's containers (vectors, matrices,...) with the 'val'
        virtual       void   fill(T val) = 0;

};


/**
* \class Vector
* \ingroup SicakData
*
* \brief A class representing a vector, stored in the machine's free space
*
*/
template <class T>
class Vector : public VectorType<T> {

public:

        /// Constructs an empty Vector with no elements. Needs to be initialized first (init).
        Vector() : m_data(nullptr), m_length(0), m_capacity(0) {}
        /// Constructs a Vector with 'length' elements
        Vector(size_t length) : m_data(nullptr), m_length(0), m_capacity(0) {
            (*this).init(length);
        }
        /// Constructs a Vector with 'length' elements and fills it with 'initVal'
        Vector(size_t length, T initVal) : m_data(nullptr), m_length(0), m_capacity(0) {
            (*this).init(length);
            (*this).fill(initVal);             
        }

        /// Move constructor
        Vector(Vector&& other) : m_data(std::move(other.m_data)), m_length(other.m_length) {  }
        /// Move assignment operator
        Vector& operator=(Vector&& other) {
                m_data = std::move(other.m_data);
                m_length = other.m_length;
                return (*this);
        }

        /// Empty destructor
        virtual ~Vector() {}

        virtual size_t length() const { return m_length; }

        virtual size_t size() const { return m_length * sizeof(T); }

        virtual void   init(size_t length) {
            
            // realloc if asking for more than I can take
            if(length > m_capacity){
                
                try {
                    
                    m_data.reset(new T[length]);
                    m_capacity = length;
                    
                } catch (std::bad_alloc & e) {
                    (void)e; // supress MSVC warnings about an unused local variable
                    throw RuntimeException("Memory allocation failed"); 
                }
                
            } 

            m_length = length;  //< set the length the user have asked for (array is at least this big)
            
        }

        virtual void   init(size_t length, T initVal) {
                (*this).init(length);
                (*this).fill(initVal);
        }

        virtual void    fill(T val) {
                std::fill_n(m_data.get(), m_length, val);
        }

        virtual T *     data() { return m_data.get(); }

        virtual const T *     data() const { return m_data.get(); }

        T & operator()       (size_t index) { return m_data[index]; }

        const   T & operator()       (size_t index) const { return m_data[index]; }

protected:

        /// Unique_ptr holding the allocated space
        std::unique_ptr<T[]> m_data;
        /// The number of elements in the vector
        size_t m_length;
        size_t m_capacity;
        
};


/**
* \class Matrix
* \ingroup SicakData
*
* \brief A class representing a matrix, stored in the machine's free space
*
*/
template <class T>
class Matrix : public MatrixType<T> {

public:

        /// Constructs an empty Matrix with no elements. Needs to be initialized first (init).
        Matrix() : m_vector(), m_cols(0), m_rows(0) {}
        /// Constructs a Matrix with 'cols' * 'rows' elements
        Matrix(size_t cols, size_t rows) : m_vector(cols * rows), m_cols(cols), m_rows(rows) {}
        /// Constructs a Matrix with 'cols' * 'rows' elements and fills it with 'initVal'
        Matrix(size_t cols, size_t rows, T initVal) : m_vector(cols * rows, initVal), m_cols(cols), m_rows(rows) {}

        /// Move constructor
        Matrix(Matrix&& other) : m_vector(std::move(other.m_vector)), m_cols(other.m_cols), m_rows(other.m_rows) {}
        /// Move assignment operator
        Matrix& operator=(Matrix&& other) {
                m_vector = std::move(other.m_vector);
                m_cols = other.m_cols;
                m_rows = other.m_rows;
                return (*this);
        }

        /// Empty destructor
        virtual ~Matrix() {}

        virtual size_t cols() const { return m_cols; }
        virtual size_t rows() const { return m_rows; }

        virtual void   init(size_t cols, size_t rows) {
                m_vector.init(cols * rows);
                m_cols = cols;
                m_rows = rows;
        }
        virtual void   init(size_t cols, size_t rows, T initVal) {
                m_vector.init(cols * rows, initVal);
                m_cols = cols;
                m_rows = rows;
        }
        
        virtual void   shrinkRows(size_t rows) {
                if(rows > m_rows) throw RuntimeException("Cannot shrink Matrix to a larger size!");
                m_vector.init(m_cols * rows); // (rows <= m_rows) -> (Vector::m_capacity >= m_cols * rows) -> no memory reallocation happens, only upper bound is lowered
                m_rows = rows;                
                // Matrix now appears "less tall"... enlargening it back to the previous size would actually give back the original matrix, but we cant guarantee that generally
        }

        virtual T *    data() { return m_vector.data(); }
        virtual const T *    data() const { return m_vector.data(); }

        virtual size_t length() const { return m_vector.length(); }
        virtual size_t size() const { return m_vector.size(); }

        virtual     void fill(T val) {
                m_vector.fill(val);
        }

        virtual         T & operator()       (size_t col, size_t row) { return m_vector(row * m_cols + col); }
        virtual const   T & operator()       (size_t col, size_t row) const { return m_vector(row * m_cols + col); }

protected:

        /// A Matrix is composed using a large Vector
        Vector<T> m_vector;
        /// Number of columns
        size_t m_cols;
        /// Number of rows
        size_t m_rows;        
        
};


#endif /* TYPES_BASIC_HPP */
