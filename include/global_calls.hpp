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
* \file global_calls.hpp
*
* \brief This header file contains global routines
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef GLOBAL_CALLS_HPP
#define GLOBAL_CALLS_HPP

#include <QElapsedTimer>
#include <iostream>

/**
* \class CoutProgress
*
* \brief A singleton class providing a standard output progress bar, including remaining time estimate
*
*/
class CoutProgress {
    
public:        

    /// Singleton instance getter
    static CoutProgress & get(){
        static CoutProgress instance;
        return instance;
        
    }
    
    /// Start displaying the progress bar and define the amout of work that's left to be done
    void start(size_t workSize){
        m_workSize = workSize > 1 ? workSize : 1;
        m_workProgress = 0;
        m_lastPercentage = 0;
        m_startTime.start();
        std::cout << '\r' << "0% done... remaining time not yet available" << std::flush;
    }
    
    /// Update the progress bar, 0 <= workProgress <= workSize
    void update(size_t workProgress){
        m_workProgress = workProgress;
        int percentage = ((float)m_workProgress / (float)m_workSize) * 100.0;
        if(percentage >= 100) percentage = 99;
        if(percentage > m_lastPercentage){
            m_lastPercentage = percentage;
            std::cout << '\r' << percentage << "% done... approx. ";
            printFormattedTime((((float)m_startTime.elapsed() / (float)m_workProgress) * (float)(m_workSize - m_workProgress)) / 1000.0);
            std::cout << " remaining                                                 " << std::flush;
        }
    }
    
    /// Call when the work is done to stop the progress bar
    void finish(){
        m_lastPercentage = 100;
        std::cout << '\r' << "100% done... ";
        printFormattedTime((float)m_startTime.elapsed() / 1000.0);
        std::cout << " elapsed.                                                 " << std::endl << std::flush;
    }
    
    static void printFormattedTime(size_t sec){
        if(!sec){
            std::cout << "<1s";
            return;
        }
        size_t days = sec / 86400;
        sec %= 86400;
        size_t hours = sec / 3600;
        sec %= 3600;
        size_t minutes = sec / 60;
        size_t seconds = sec % 60;
        if(days) std::cout << days << "d, ";
        if(days || hours) std::cout << hours << "h, ";
        if(days || hours || minutes) std::cout << minutes << "m, ";
        if(days || hours || minutes || seconds) std::cout << seconds << "s";
    }
    
protected:
    
    size_t m_workSize;
    size_t m_workProgress;
    int m_lastPercentage;
    QElapsedTimer m_startTime;        

private:   
    
    CoutProgress(): m_workSize(1), m_workProgress(0), m_lastPercentage(0) {}
    CoutProgress(CoutProgress const&);
    void operator=(CoutProgress const&);    
    
};

#endif /* GLOBAL_CALLS_HPP */
