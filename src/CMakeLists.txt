cmake_minimum_required(VERSION 3.4.3)
project(hdbe)

find_package(LLVM REQUIRED CONFIG)

set(hdlbe_src ../src/)



message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Using LLVM_DEFINITIONS   : ${LLVM_DEFINITIONS}")

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

include_directories(${LLVM_INCLUDE_DIRS} ${hdlbe_src})
add_definitions(${LLVM_DEFINITIONS})
add_definitions(-DLOGURU_WITH_STREAMS=1)

SET(GCC_DBG_FLAGS "-g -rdynamic")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_DBG_FLAGS}")

# Now build our tools
add_executable(hdbe hdbe.cpp )

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader analysis passes)

# Link against LLVM libraries
target_link_libraries(hdbe ${llvm_libs} )
target_sources(hdbe PUBLIC      
    ${hdlbe_src}/IRUtil.cpp
    ${hdlbe_src}/HDLUtil.cpp
    ${hdlbe_src}/logging/logger.cpp
    ${hdlbe_src}/DataAnalyzer.cpp
    ${hdlbe_src}/VerilogGenerator.cpp
    ${hdlbe_src}/IRPreprocessor.cpp
    ${hdlbe_src}/ValueLifeInfo.cpp
    ${hdlbe_src}/InstructionScheduler.cpp
    ${hdlbe_src}/HardwareDescription.cpp
    ${hdlbe_src}/ControlDataInfo.cpp)

set_property(TARGET hdbe PROPERTY CXX_STANDARD 17)
