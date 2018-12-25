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
* \file oclengine.hpp
*
* \brief OpenCL base class template for SICAK plugins
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef OCLENGINE_HPP
#define OCLENGINE_HPP


#include <string>

#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include "exceptions.hpp"

/**
* \class OclEngine
*
* \brief OpenCL base class template used in other SICAK plugins
*
*/
template<class T>
class OclEngine {

protected:

        unsigned int m_platform;
        unsigned int m_device;
        cl_context m_context;
        cl_command_queue m_command_queue;
        cl_platform_id * m_platforms;
        cl_device_id * m_devices;				

public:

        /// Initializes the specified OpenCL device and creates an OpenCL command queue
        OclEngine(unsigned int platform, unsigned int device);
        
        virtual ~OclEngine();

        /// Query available platforms and devices
        static std::string queryDevices();
                
        /// Returns the data type name
        std::string getTypeName(float dummy) { dummy=dummy; return std::string("float"); }
        /// Returns the data type name
        std::string getTypeName(double dummy) { dummy=dummy; return std::string("double"); }

        /// Returns the data type name
        std::string getTypeName(int8_t dummy) { dummy=dummy; return std::string("char"); }
        /// Returns the data type name
        std::string getTypeName(uint8_t dummy) { dummy=dummy; return std::string("unsigned char"); }

        /// Returns the data type name
        std::string getTypeName(int16_t dummy) { dummy=dummy; return std::string("short"); }
        /// Returns the data type name
        std::string getTypeName(uint16_t dummy) { dummy=dummy; return std::string("unsigned short"); }

        /// Returns the data type name
        std::string getTypeName(int32_t dummy) { dummy=dummy; return std::string("int"); }
        /// Returns the data type name
        std::string getTypeName(uint32_t dummy) { dummy=dummy; return std::string("unsigned int"); }

        /// Returns the data type name
        std::string getTypeName(int64_t dummy) { dummy=dummy; return std::string("long"); }
        /// Returns the data type name
        std::string getTypeName(uint64_t dummy) { dummy=dummy; return std::string("unsigned long"); }

        /// Trims white space from string
        static void trimWS(std::string & str);
};



template<class T>
OclEngine<T>::OclEngine(unsigned int platform, unsigned int device): m_platform(platform), m_device(device), m_platforms(NULL), m_devices(NULL) {
        
        // Get number of OpenCL platforms available
        cl_uint ret_num_platforms;
        cl_int ret = clGetPlatformIDs(0, NULL, &ret_num_platforms);
        if (ret) throw RuntimeException("Couldn't get number of platforms available", ret);
        if (ret_num_platforms == 0) throw RuntimeException("No OpenCL platforms found");

        if (platform >= ret_num_platforms) throw RuntimeException("No such OpenCL platform found");
        
        // Alloc memory for the platforms
        m_platforms = (cl_platform_id*)malloc(ret_num_platforms * sizeof(cl_platform_id));
        if (m_platforms == NULL) throw RuntimeException("Couldn't alloc memory");

        // Get the platforms
        ret = clGetPlatformIDs(ret_num_platforms, m_platforms, NULL);
        if (ret) {
                free(m_platforms);
                throw RuntimeException("Couldn't get platform IDs", ret);
        }

        // Get number of OpenCL devices available
        cl_uint ret_num_devices;
        ret = clGetDeviceIDs(m_platforms[platform], CL_DEVICE_TYPE_ALL, 0, NULL, &ret_num_devices);
        if (ret) {
                free(m_platforms);
                throw RuntimeException("Couldn't get number of devices available", ret);
        }

        if (device >= ret_num_devices) {
                free(m_platforms);
                throw RuntimeException("No such OpenCL device found on this platform");
        }

        // Alloc memory for the devices
        m_devices = (cl_device_id*)malloc(ret_num_devices * sizeof(cl_device_id));
        if (m_devices == NULL) {
                free(m_platforms);
                throw RuntimeException("Couldn't alloc memory");
        }

        // Get the devices
        ret = clGetDeviceIDs(m_platforms[platform], CL_DEVICE_TYPE_ALL, ret_num_devices, m_devices, &ret_num_devices);
        if (ret) {
                free(m_devices);
                free(m_platforms);
                throw RuntimeException("Couldn't get device IDs", ret);
        }

        // Create an OpenCL context for the selected device
        m_context = clCreateContext(NULL, 1, &(m_devices[device]), NULL, NULL, &ret);		
        if (ret) {			
                free(m_devices);
                free(m_platforms);
                throw RuntimeException("Couldn't create the OpenCL context", ret);
        }

        // Create a command queue
        m_command_queue = clCreateCommandQueue(m_context, m_devices[device], 0, &ret);
        if (ret) {
                ret = clReleaseContext(m_context);
                free(m_devices);
                free(m_platforms);
                throw RuntimeException("Couldn't create a command queue", ret);
        }
        
        // from now on, the destructor becomes responsible for the release
}

template<class T>
OclEngine<T>::~OclEngine() {

        cl_int ret;
        // Release the command queue, context and devices/platforms lists
        ret = clReleaseCommandQueue(m_command_queue);
        ret = clReleaseContext(m_context);
        free(m_devices);
        free(m_platforms);
        // ...ignore errors while releasing members (cannot throw out of a destructor; and since the object is being destructed, it doesn't matter much anyway)
        ret = ret;
}

template<class T>
void OclEngine<T>::trimWS(std::string & str) {
    
    const std::string whitespaces = "\t\n\v\f\r ";
    str.erase(0, str.find_first_not_of(whitespaces));
    str.erase(str.find_last_not_of(whitespaces) + 1);
    
}

template<class T>
std::string OclEngine<T>::queryDevices() {

        std::string devicesRet;
        char textBuffer[1024];
        std::string textBufferStr;

        // Get number of OpenCL platforms available
        cl_uint ret_num_platforms;
        cl_int ret = clGetPlatformIDs(0, NULL, &ret_num_platforms);
        if (ret) throw RuntimeException("Couldn't get number of platforms available", ret);

        if (ret_num_platforms == 0) {
                return std::string("    No OpenCL platforms found!\n");
        }

        // Alloc memory for the platforms
        cl_platform_id *platforms = NULL;
        platforms = (cl_platform_id*)malloc(ret_num_platforms * sizeof(cl_platform_id));
        if (platforms == NULL) throw RuntimeException("Couldn't alloc memory");

        // Get the platforms
        ret = clGetPlatformIDs(ret_num_platforms, platforms, NULL);
        if (ret) {
                free(platforms);
                throw RuntimeException("Couldn't get platform IDs", ret);
        }

        // Loop through the platforms
        for (unsigned int i = 0; i < ret_num_platforms; i++) {

                // Get the platform name
                ret = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(textBuffer), textBuffer, NULL);
                if (ret) {
                        free(platforms);
                        throw RuntimeException("Couldn't get platform name", ret);
                }

                textBufferStr = textBuffer;
                trimWS(textBufferStr);
                devicesRet.append("    * Platform ID: '").append(std::to_string(i)).append("', name: '").append(textBufferStr).append("'");

                // Get the platform version
                ret = clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, sizeof(textBuffer), textBuffer, NULL);
                if (ret) {
                        free(platforms);
                        throw RuntimeException("Couldn't get platform version", ret);
                }

                devicesRet.append(" (").append(textBuffer).append(")\n");

                // Get number of OpenCL devices available
                cl_uint ret_num_devices;
                ret = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &ret_num_devices);
                if (ret) {
                        free(platforms);
                        throw RuntimeException("Couldn't get number of devices available", ret);
                }

                if (ret_num_devices == 0) {
                        devicesRet.append("No OpenCL devices found\n\n");
                        continue;
                }

                // Alloc memory for the devices
                cl_device_id *devices = NULL;
                devices = (cl_device_id*)malloc(ret_num_devices * sizeof(cl_device_id));
                if (devices == NULL) {
                        free(platforms);
                        throw RuntimeException("Couldn't alloc memory");
                }

                // Get the devices
                ret = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, ret_num_devices, devices, &ret_num_devices);
                if (ret) {
                        free(devices);
                        free(platforms);
                        throw RuntimeException("Couldn't get device IDs", ret);
                }

                // Loop through the devices
                for (unsigned int k = 0; k < ret_num_devices; k++) {

                        // Get device name
                        ret = clGetDeviceInfo(devices[k], CL_DEVICE_NAME, sizeof(textBuffer), &textBuffer, NULL);
                        if (ret) {
                                free(devices);
                                free(platforms);
                                throw RuntimeException("Couldn't get device name", ret);
                        }

                        textBufferStr = textBuffer;
                        trimWS(textBufferStr); // Intel for some reasons indents this value -> trim whitespace
                        devicesRet.append("        * Device ID: '").append(std::to_string(k)).append("', name: '").append(textBufferStr).append("'\n");

                }

                devicesRet.append("\n");

                free(devices);
        }

        free(platforms);

        return devicesRet;

}


#endif /* OCLENGINE_HPP */
