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
* \file exceptions.hpp
*
* \brief This header file contains exceptions
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <exception>
#include <string>



/**
* \class Exception
*
* \brief A base exception
*
*/
class Exception : public std::exception {

public:

    /// Constructor with a default error message
    Exception() : m_msg("Unspecified error") {}    
    /// Constructor with a custom error message and error code
    Exception(const char * msg, int errCode = 0) : m_msg(msg) {
        if(errCode){
            m_msg.append(", error code: ");
            m_msg.append(std::to_string(errCode));
        }
    }

    /// Return the error message, potentially with error code included
    virtual const char * what() const throw () {         
        return m_msg.c_str();        
    }

protected:
    
    /// Error message
    std::string m_msg;

};


/**
* \class RuntimeException
*
* \brief An exception which cannot be directly influenced by the user, or predicted beforehand
*
*/
class RuntimeException : public Exception {

public:
    /// Constructor with a default error message
    RuntimeException() : Exception() {}    
    /// Constructor with a custom error message and error code
    RuntimeException(const char * msg, int errCode = 0) : Exception(msg, errCode) {}

};


/**
* \class InvalidInputException
*
* \brief An exception caused by bad settings of input parameters or arguments
*
*/
class InvalidInputException : public Exception {

public:
    /// Constructor with a default error message
    InvalidInputException() : Exception() {}    
    /// Constructor with a custom error message and error code
    InvalidInputException(const char * msg, int errCode = 0) : Exception(msg, errCode) {}

};


#endif /* EXCEPTIONS_HPP */
