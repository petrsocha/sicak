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
* \file oclcpaengine.hpp
*
* \brief OpenCL implementation of CPA statistical algorithms as function templates
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef OCLCPAENGINE_HPP
#define OCLCPAENGINE_HPP

#include "types_power.hpp"
#include "types_stat.hpp"
#include "exceptions.hpp"
#include "oclengine.hpp"

/**
* \class OclEngine
*
* \brief OpenCL base class template used in other SICAK plugins
*
*/
template<class Tc, class Tt, class Tp>
class OclCpaEngine : public OclEngine<Tc> {

protected:

        static const char * m_programCode;
        unsigned int m_samplesPerTrace;
        unsigned int m_noOfCandidates;
        unsigned int m_noOfTraces;
        bool m_compiled;

        /* ocl device buffers */
        // input data
        cl_mem m_predictions_mem;
        cl_mem m_traces_mem;
        // CPA context variables
        cl_mem m_predsAvg_mem;
        cl_mem m_predsMSum_mem;
        cl_mem m_tracesAvg_mem;
        cl_mem m_tracesMSum_mem;
        cl_mem m_predsTracesCSum_mem;

        /* ocl program and kernels */
        cl_program m_program;
        cl_kernel m_kernel_computeTracesAvgMSum;
        cl_kernel m_kernel_computePredsAvgMSum;
        cl_kernel m_kernel_computeCSum;
        
public:

        /// Initialize given platform and device, create command queue and allocate device memory buffers 
        OclCpaEngine(unsigned int platform, unsigned int device, unsigned int samplesPerTrace, unsigned int noOfCandidates, unsigned int noOfTraces);

        virtual ~OclCpaEngine();

        /// Build the OpenCL kernels
        void buildProgram();
        /// Load power predictions from local memory to device buffers
        void loadPredictionsToDevice(const PowerPredictions<Tp> & pp, bool blocking = false);
        /// Load power traces from local memory to device buffers
        void loadTracesToDevice(const PowerTraces<Tt> & pt, bool blocking = false);
        /// Launch the computation kernel, divide the work by sliceSize (long running GPU kernel is not good), return result in UnivariateContext context
        void compute(UnivariateContext<Tc> & context, unsigned int sliceSize);

};


template<class Tc, class Tt, class Tp>
OclCpaEngine<Tc, Tt, Tp>::OclCpaEngine(unsigned int platform, unsigned int device, unsigned int samplesPerTrace, unsigned int noOfCandidates, unsigned int noOfTraces) 
        : OclEngine<Tc>(platform, device), m_samplesPerTrace(samplesPerTrace), m_noOfCandidates(noOfCandidates), m_noOfTraces(noOfTraces), m_compiled(false) {

        cl_int ret;

        // OclEngine provides with a working context and queue; so just acquire the memory buffers
        m_predictions_mem = clCreateBuffer(this->m_context, CL_MEM_READ_ONLY, noOfCandidates * noOfTraces * sizeof(Tp), NULL, &ret);
        if (ret) {
                throw RuntimeException("Couldn't allocate a data buffer on the device", ret);
        }

        m_traces_mem = clCreateBuffer(this->m_context, CL_MEM_READ_ONLY, samplesPerTrace * noOfTraces * sizeof(Tt), NULL, &ret);
        if (ret) {
                clReleaseMemObject(m_predictions_mem);
                throw RuntimeException("Couldn't allocate a data buffer on the device", ret);
        }

        m_predsAvg_mem = clCreateBuffer(this->m_context, CL_MEM_READ_WRITE, noOfCandidates * sizeof(Tc), NULL, &ret);
        if (ret) {
                clReleaseMemObject(m_predictions_mem);
                clReleaseMemObject(m_traces_mem);
                throw RuntimeException("Couldn't allocate a working context buffer on the device", ret);
        }

        m_predsMSum_mem = clCreateBuffer(this->m_context, CL_MEM_READ_WRITE, noOfCandidates * sizeof(Tc), NULL, &ret);
        if (ret) {
                clReleaseMemObject(m_predictions_mem);
                clReleaseMemObject(m_traces_mem);
                clReleaseMemObject(m_predsAvg_mem);
                throw RuntimeException("Couldn't allocate a working context buffer on the device", ret);
        }

        m_tracesAvg_mem = clCreateBuffer(this->m_context, CL_MEM_READ_WRITE, samplesPerTrace * sizeof(Tc), NULL, &ret);
        if (ret) {
                clReleaseMemObject(m_predictions_mem);
                clReleaseMemObject(m_traces_mem);
                clReleaseMemObject(m_predsAvg_mem);
                clReleaseMemObject(m_predsMSum_mem);
                throw RuntimeException("Couldn't allocate a working context buffer on the device", ret);
        }

        m_tracesMSum_mem = clCreateBuffer(this->m_context, CL_MEM_READ_WRITE, samplesPerTrace * sizeof(Tc), NULL, &ret);
        if (ret) {
                clReleaseMemObject(m_predictions_mem);
                clReleaseMemObject(m_traces_mem);
                clReleaseMemObject(m_predsAvg_mem);
                clReleaseMemObject(m_predsMSum_mem);
                clReleaseMemObject(m_tracesAvg_mem);
                throw RuntimeException("Couldn't allocate a working context buffer on the device", ret);
        }

        m_predsTracesCSum_mem = clCreateBuffer(this->m_context, CL_MEM_READ_WRITE, noOfCandidates * samplesPerTrace * sizeof(Tc), NULL, &ret);
        if (ret) {
                clReleaseMemObject(m_predictions_mem);
                clReleaseMemObject(m_traces_mem);
                clReleaseMemObject(m_predsAvg_mem);
                clReleaseMemObject(m_predsMSum_mem);
                clReleaseMemObject(m_tracesAvg_mem);
                clReleaseMemObject(m_tracesMSum_mem);
                throw RuntimeException("Couldn't allocate a working context buffer on the device", ret);
        }

}


template<class Tc, class Tt, class Tp>
OclCpaEngine<Tc, Tt, Tp>::~OclCpaEngine() {
        
        if (m_compiled) {
                clReleaseKernel(m_kernel_computeCSum);
                clReleaseKernel(m_kernel_computePredsAvgMSum);
                clReleaseKernel(m_kernel_computeTracesAvgMSum);
                clReleaseProgram(m_program);
        }

        clReleaseMemObject(m_predictions_mem);
        clReleaseMemObject(m_traces_mem);
        clReleaseMemObject(m_predsAvg_mem);
        clReleaseMemObject(m_predsMSum_mem);
        clReleaseMemObject(m_tracesAvg_mem);
        clReleaseMemObject(m_tracesMSum_mem);
        clReleaseMemObject(m_predsTracesCSum_mem);

}


template<class Tc, class Tt, class Tp>
void OclCpaEngine<Tc, Tt, Tp>::buildProgram() {
    
        if(m_compiled) return;

        cl_int ret;
        
        Tc dummyTc;
        Tt dummyTt;
        Tp dummyTp;
        
        std::string code("");

        // get the program together first
        if (!(this->getTypeName(dummyTc)).compare("double") || !(this->getTypeName(dummyTt)).compare("double") || !(this->getTypeName(dummyTp)).compare("double")) {
                code.append("#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n");
        }        
        
        code.append("typedef ").append(this->getTypeName(dummyTc)).append(" Tc;\n");
        code.append("typedef ").append(this->getTypeName(dummyTt)).append(" Tt;\n");
        code.append("typedef ").append(this->getTypeName(dummyTp)).append(" Tp;\n");

        code.append(m_programCode);

        // no changes to code from now on
        const char * codePtr = code.c_str(); // clCreateProgramWithSource requires **; & operator requires l-value
        const size_t codeSize = code.size(); 

        m_program = clCreateProgramWithSource(this->m_context, 1, (const char **)&codePtr, (const size_t *)&codeSize, &ret);
        if (ret) throw RuntimeException("Couldn't create the ocl program from source", ret);

        // build the ocl program
        ret = clBuildProgram(m_program, 1, &((this->m_devices)[this->m_device]), NULL, NULL, NULL);
        if (ret) {
                clReleaseProgram(m_program);
                throw RuntimeException("Couldn't build the ocl program", ret);
        }
        
        // create the kernels
        m_kernel_computeTracesAvgMSum = clCreateKernel(m_program, "computeTracesAvgMSum", &ret);
        if (ret) {
                clReleaseProgram(m_program);
                throw RuntimeException("Couldn't create a kernel", ret);
        }

        m_kernel_computePredsAvgMSum = clCreateKernel(m_program, "computePredsAvgMSum", &ret);
        if (ret) {
                clReleaseKernel(m_kernel_computeTracesAvgMSum);
                clReleaseProgram(m_program);
                throw RuntimeException("Couldn't create a kernel", ret);
        }

        m_kernel_computeCSum = clCreateKernel(m_program, "computeCSum", &ret);
        if (ret) {
                clReleaseKernel(m_kernel_computePredsAvgMSum);
                clReleaseKernel(m_kernel_computeTracesAvgMSum);
                clReleaseProgram(m_program);
                throw RuntimeException("Couldn't create a kernel", ret);
        }

        m_compiled = true; // since now, the destructor takes care of the program and kernels structs

        // set kernel args
        ret = clSetKernelArg(m_kernel_computeTracesAvgMSum, 0, sizeof(cl_mem), (void *)&m_traces_mem);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        ret = clSetKernelArg(m_kernel_computeTracesAvgMSum, 1, sizeof(cl_mem), (void *)&m_tracesAvg_mem);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        ret = clSetKernelArg(m_kernel_computeTracesAvgMSum, 2, sizeof(cl_mem), (void *)&m_tracesMSum_mem);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        ret = clSetKernelArg(m_kernel_computeTracesAvgMSum, 3, sizeof(unsigned int), (void *)&m_samplesPerTrace);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        // set kernel args
        ret = clSetKernelArg(m_kernel_computePredsAvgMSum, 0, sizeof(cl_mem), (void *)&m_predictions_mem);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        ret = clSetKernelArg(m_kernel_computePredsAvgMSum, 1, sizeof(cl_mem), (void *)&m_predsAvg_mem);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        ret = clSetKernelArg(m_kernel_computePredsAvgMSum, 2, sizeof(cl_mem), (void *)&m_predsMSum_mem);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        ret = clSetKernelArg(m_kernel_computePredsAvgMSum, 3, sizeof(unsigned int), (void *)&m_noOfCandidates);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        // set kernel args
        ret = clSetKernelArg(m_kernel_computeCSum, 0, sizeof(cl_mem), (void *)&m_traces_mem);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        ret = clSetKernelArg(m_kernel_computeCSum, 1, sizeof(cl_mem), (void *)&m_predictions_mem);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        ret = clSetKernelArg(m_kernel_computeCSum, 2, sizeof(cl_mem), (void *)&m_tracesAvg_mem);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        ret = clSetKernelArg(m_kernel_computeCSum, 3, sizeof(cl_mem), (void *)&m_predsAvg_mem);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        ret = clSetKernelArg(m_kernel_computeCSum, 4, sizeof(cl_mem), (void *)&m_predsTracesCSum_mem);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        ret = clSetKernelArg(m_kernel_computeCSum, 5, sizeof(unsigned int), (void *)&m_samplesPerTrace);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        ret = clSetKernelArg(m_kernel_computeCSum, 6, sizeof(unsigned int), (void *)&m_noOfCandidates);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

}


template<class Tc, class Tt, class Tp>
void OclCpaEngine<Tc, Tt, Tp>::loadPredictionsToDevice(const PowerPredictions<Tp>& pp, bool blocking) {

        if (m_noOfTraces * m_noOfCandidates * sizeof(Tp) != pp.size()) 
                throw RuntimeException("Number of traces and/or number of candidates conflicts with values set within construction of the ocl engine");

        cl_int ret = clEnqueueWriteBuffer(this->m_command_queue, m_predictions_mem, blocking ? CL_TRUE : CL_FALSE, 0, pp.size(), pp.data(), 0, NULL, NULL);
        if (ret) throw RuntimeException("Couldn't enqueue a data transmit to the device", ret);
        
}


template<class Tc, class Tt, class Tp>
void OclCpaEngine<Tc, Tt, Tp>::loadTracesToDevice(const PowerTraces<Tt>& pt, bool blocking) {

        if (m_noOfTraces * m_samplesPerTrace * sizeof(Tt) != pt.size()) 
                throw RuntimeException("Number of traces and/or number of samples per trace conflicts with values set within construction of the ocl engine");

        cl_int ret = clEnqueueWriteBuffer(this->m_command_queue, m_traces_mem, blocking ? CL_TRUE : CL_FALSE, 0, pt.size(), pt.data(), 0, NULL, NULL);
        if (ret) throw RuntimeException("Couldn't enqueue a data transmit to the device", ret);

}


template<class Tc, class Tt, class Tp>
void OclCpaEngine<Tc, Tt, Tp>::compute(UnivariateContext<Tc> & corrContext, unsigned int sliceSize) {

        corrContext.init(m_samplesPerTrace, m_noOfCandidates, 1, 2, 1);
        cl_int ret;
        unsigned int noOfSlices = m_noOfTraces / sliceSize;
        unsigned int remaindingSliceSize = m_noOfTraces - noOfSlices * sliceSize;
        unsigned int offset;

        // first, compute the Avgs and MSums (variance) of the traces
        size_t traces_global_item_size = (((m_samplesPerTrace - 1) / 64) + 1) * 64; // divide the work by 64
        size_t traces_local_item_size = 64; 

        ret = clSetKernelArg(m_kernel_computeTracesAvgMSum, 4, sizeof(unsigned int), (void *)&sliceSize);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        for (unsigned int i = 0; i < noOfSlices; i++) {

                offset = i * sliceSize;			

                ret = clSetKernelArg(m_kernel_computeTracesAvgMSum, 5, sizeof(unsigned int), (void *)&offset);
                if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

                ret = clEnqueueNDRangeKernel(this->m_command_queue, m_kernel_computeTracesAvgMSum, 1, NULL, &traces_global_item_size, &traces_local_item_size, 0, NULL, NULL);
                if (ret) throw RuntimeException("Couldn't enqueue a kernel to the device", ret);

                ret = clFinish(this->m_command_queue);
                if (ret) throw RuntimeException("Error while processing the queue", ret);

        }
        
        offset = noOfSlices * sliceSize;

        ret = clSetKernelArg(m_kernel_computeTracesAvgMSum, 4, sizeof(unsigned int), (void *)&remaindingSliceSize);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        ret = clSetKernelArg(m_kernel_computeTracesAvgMSum, 5, sizeof(unsigned int), (void *)&offset);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        ret = clEnqueueNDRangeKernel(this->m_command_queue, m_kernel_computeTracesAvgMSum, 1, NULL, &traces_global_item_size, &traces_local_item_size, 0, NULL, NULL);
        if (ret) throw RuntimeException("Couldn't enqueue a kernel to the device", ret);

        ret = clFinish(this->m_command_queue);
        if (ret) throw RuntimeException("Error while processing the queue", ret);


        // also Avgs and MSums (variance) of the predictions
        size_t preds_global_item_size = (((m_noOfCandidates - 1) / 64) + 1) * 64; // divide the work by 64
        size_t preds_local_item_size = 64;

        ret = clSetKernelArg(m_kernel_computePredsAvgMSum, 4, sizeof(unsigned int), (void *)&sliceSize);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        for (unsigned int i = 0; i < noOfSlices; i++) {

                offset = i * sliceSize;

                ret = clSetKernelArg(m_kernel_computePredsAvgMSum, 5, sizeof(unsigned int), (void *)&offset);
                if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

                ret = clEnqueueNDRangeKernel(this->m_command_queue, m_kernel_computePredsAvgMSum, 1, NULL, &preds_global_item_size, &preds_local_item_size, 0, NULL, NULL);
                if (ret) throw RuntimeException("Couldn't enqueue a kernel to the device", ret);

                ret = clFinish(this->m_command_queue);
                if (ret) throw RuntimeException("Error while processing the queue", ret);

        }

        offset = noOfSlices * sliceSize;

        ret = clSetKernelArg(m_kernel_computePredsAvgMSum, 4, sizeof(unsigned int), (void *)&remaindingSliceSize);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        ret = clSetKernelArg(m_kernel_computePredsAvgMSum, 5, sizeof(unsigned int), (void *)&offset);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        ret = clEnqueueNDRangeKernel(this->m_command_queue, m_kernel_computePredsAvgMSum, 1, NULL, &preds_global_item_size, &preds_local_item_size, 0, NULL, NULL);
        if (ret) throw RuntimeException("Couldn't enqueue a kernel to the device", ret);

        ret = clFinish(this->m_command_queue);
        if (ret) throw RuntimeException("Error while processing the queue", ret);


        // finally, compute the CSums (covariance) matrix
        size_t csum_global_item_size[2];
        csum_global_item_size[0] = (((m_samplesPerTrace - 1) / 16) + 1) * 16; // divide the work by 16x16
        csum_global_item_size[1] = (((m_noOfCandidates - 1) / 16) + 1) * 16; 
        size_t csum_local_item_size[] = { 16, 16 };  

        ret = clSetKernelArg(m_kernel_computeCSum, 7, sizeof(unsigned int), (void *)&sliceSize);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        for (unsigned int i = 0; i < noOfSlices; i++) {

                offset = i * sliceSize;			

                ret = clSetKernelArg(m_kernel_computeCSum, 8, sizeof(unsigned int), (void *)&offset);
                if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

                ret = clEnqueueNDRangeKernel(this->m_command_queue, m_kernel_computeCSum, 2, NULL, csum_global_item_size, csum_local_item_size, 0, NULL, NULL);
                if (ret) throw RuntimeException("Couldn't enqueue a kernel to the device", ret);

                ret = clFinish(this->m_command_queue);
                if (ret) throw RuntimeException("Error while processing the queue", ret);

        }

        offset = noOfSlices * sliceSize;

        ret = clSetKernelArg(m_kernel_computeCSum, 7, sizeof(unsigned int), (void *)&remaindingSliceSize);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        ret = clSetKernelArg(m_kernel_computeCSum, 8, sizeof(unsigned int), (void *)&offset);
        if (ret) throw RuntimeException("Couldn't set kernel argument", ret);

        ret = clEnqueueNDRangeKernel(this->m_command_queue, m_kernel_computeCSum, 2, NULL, csum_global_item_size, csum_local_item_size, 0, NULL, NULL);
        if (ret) throw RuntimeException("Couldn't enqueue a kernel to the device", ret);

        ret = clFinish(this->m_command_queue);
        if (ret) throw RuntimeException("Error while processing the queue", ret);


        // read the data back from the device
        ret = clEnqueueReadBuffer(this->m_command_queue, m_predsAvg_mem, CL_TRUE, 0, corrContext.p2M(1).size(), corrContext.p2M(1).data(), 0, NULL, NULL);
        if (ret) throw RuntimeException("Couldn't enqueue a data transmit from the device", ret);

        ret = clEnqueueReadBuffer(this->m_command_queue, m_predsMSum_mem, CL_TRUE, 0, corrContext.p2CS(2).size(), corrContext.p2CS(2).data(), 0, NULL, NULL);
        if (ret) throw RuntimeException("Couldn't enqueue a data transmit from the device", ret);

        ret = clEnqueueReadBuffer(this->m_command_queue, m_tracesAvg_mem, CL_TRUE, 0, corrContext.p1M(1).size(), corrContext.p1M(1).data(), 0, NULL, NULL);
        if (ret) throw RuntimeException("Couldn't enqueue a data transmit from the device", ret);

        ret = clEnqueueReadBuffer(this->m_command_queue, m_tracesMSum_mem, CL_TRUE, 0, corrContext.p1CS(2).size(), corrContext.p1CS(2).data(), 0, NULL, NULL);
        if (ret) throw RuntimeException("Couldn't enqueue a data transmit from the device", ret);

        ret = clEnqueueReadBuffer(this->m_command_queue, m_predsTracesCSum_mem, CL_TRUE, 0, corrContext.p12ACS(1).size(), corrContext.p12ACS(1).data(), 0, NULL, NULL);
        if (ret) throw RuntimeException("Couldn't enqueue a data transmit from the device", ret);

        ret = clFinish(this->m_command_queue);
        if (ret) throw RuntimeException("Error while processing the queue", ret);
        
        corrContext.p1Card() = m_noOfTraces;
        corrContext.p2Card() = corrContext.p1Card();

}


template<class Tc, class Tt, class Tp>
const char * OclCpaEngine<Tc, Tt, Tp>::m_programCode = "\n\
\
        __kernel void computeCSum(__global const Tt *traces, __global const Tp *predictions, __global const Tc *tracesAvg, __global const Tc *predsAvg, __global Tc *CSums,  unsigned int samplesPerTrace, unsigned int noOfCandidates, unsigned int noOfTraces, unsigned int traceOffset) { \
\
                unsigned int sample = get_global_id(0); \
                unsigned int candidate = get_global_id(1);\
\
                Tc localTracesAvg = (sample < samplesPerTrace) ? tracesAvg[sample] : (Tc)0;\
                Tc localPredsAvg = (candidate < noOfCandidates) ? predsAvg[candidate] : (Tc)0;\
                Tc localCSum = ((traceOffset > 0) && (sample < samplesPerTrace) && (candidate < noOfCandidates)) ? CSums[candidate * samplesPerTrace + sample] : (Tc)0;\
\
                Tc val1;\
                Tc val2;\
\
                for (int trace = traceOffset; trace < (traceOffset + noOfTraces); trace++) {\
\
                        val1 = (sample < samplesPerTrace) ? traces[trace * samplesPerTrace + sample] : (Tc)0;\
                        val2 = (candidate < noOfCandidates) ? predictions[trace * noOfCandidates + candidate] : (Tc)0;\
\
                        localCSum += (val1 - localTracesAvg) * (val2 - localPredsAvg);\
\
                        barrier(CLK_LOCAL_MEM_FENCE);\
\
                }\
\
                if ((sample < samplesPerTrace) && (candidate < noOfCandidates)) {\
\
                        CSums[candidate * samplesPerTrace + sample] = localCSum;\
\
                }\
\
        }\
\
\
        __kernel void computeTracesAvgMSum(__global const Tt *traces, __global Tc *avgs, __global Tc *msums, unsigned int samplesPerTrace, unsigned int noOfTraces, unsigned int traceOffset) {\
\
                unsigned int sample = get_global_id(0);\
\
                if (sample >= samplesPerTrace)\
                        return;\
\
                unsigned int trace = traceOffset;\
                Tc ctrace = (Tc)traceOffset;\
\
                Tc localAvg = (traceOffset > 0) ? avgs[sample] : (Tc)0;\
                Tc localMSum = (traceOffset > 0) ? msums[sample] : (Tc)0;\
                Tc val;\
\
                Tc temp;\
\
                for (; trace < (traceOffset + noOfTraces); trace++) {\
\
                        val = traces[trace * samplesPerTrace + sample];\
\
                        temp = val - localAvg;\
                        ctrace = ctrace + (Tc)1;\
                        localAvg += temp / ctrace;\
                        localMSum += temp * (val - localAvg);\
\
                }\
\
                avgs[sample] = localAvg;\
                msums[sample] = localMSum;\
\
        }\
\
\
        __kernel void computePredsAvgMSum(__global const Tp *preds, __global Tc *avgs, __global Tc *msums, unsigned int noOfCandidates, unsigned int noOfTraces, unsigned int traceOffset) {\
\
                unsigned int candidate = get_global_id(0);\
\
                if (candidate >= noOfCandidates)\
                        return;\
\
                unsigned int trace = traceOffset;\
                Tc ctrace = (Tc)traceOffset;\
\
                Tc localAvg = (traceOffset > 0) ? avgs[candidate] : (Tc)0;\
                Tc localMSum = (traceOffset > 0) ? msums[candidate] : (Tc)0;\
                Tc val;\
\
                Tc temp;\
\
                for (; trace < (traceOffset + noOfTraces); trace++) {\
\
                        val = preds[trace * noOfCandidates + candidate];\
\
                        temp = val - localAvg;\
                        ctrace = ctrace + (Tc)1;\
                        localAvg += temp / ctrace;\
                        localMSum += temp * (val - localAvg);\
\
                }\
\
                avgs[candidate] = localAvg;\
                msums[candidate] = localMSum;\
\
        } \n";



#endif /* OCLCPAENGINE_HPP */
