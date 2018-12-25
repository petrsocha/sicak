################################################################################
# Follow instructions to configure the project for build                       #
#                                                                              #
# # When using paths, do not include the trailing slash                        #
#                                                                              #
################################################################################

#
# To allow for compilation of VISA dependent parts, 
# uncomment following lines and set the variables properly
#

win32 {
#    VISAINCLUDES = "C:\Program Files\IVI Foundation\VISA\Win64\Include"
#    VISALIBSDIR = "C:\Program Files\IVI Foundation\VISA\Win64\Lib_x64\msc"
#    VISALIB = "visa64.lib"
}

#
# To allow for compilation of PicoScope 6000 dependent parts, 
# uncomment following lines and set the variables properly
#

win32 {
#    PS6000INCLUDES = "C:\Program Files\Pico Technology\SDK\inc" 
#    PS6000LIBSDIR = "C:\Program Files\Pico Technology\SDK\lib" 
#    PS6000LIB = "ps6000.lib"
}
unix {
#    PS6000INCLUDES = "/opt/picoscope/include/libps6000-1.4"
#    PS6000LIBSDIR = "/opt/picoscope/lib"
#    PS6000LIB = "libps6000.so"
}

#
# To allow for compilation of OpenCL dependent parts (GPU computation modules),
# uncomment following lines and set the variables properly
#

win32 {
#    OPENCLINCLUDES = "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v9.2\include" 
#    OPENCLLIBSDIR = "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v9.2\lib\x64" 
#    OPENCLLIB = "OpenCL.lib"
}
unix {
#    OPENCLINCLUDES = "/opt/intel/opencl-sdk/include/"
#    OPENCLLIBSDIR = "/opt/intel/opencl-sdk/lib64/"
#    OPENCLLIB = "libOpenCL.so"
}

