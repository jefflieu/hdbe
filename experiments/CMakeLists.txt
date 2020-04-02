cmake_minimum_required(VERSION 3.4.3)
project(ir_parser)

find_package(LLVM REQUIRED CONFIG)

set(dep_root ../..)
set(hdlbe_src ../src)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

#add_library(loguru STATIC ${dep_root}/loguru/loguru.cpp)
#add_library(hdlbe  STATIC ${hdlbe_src}/SchedulingAlgorithm.cpp)

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

include_directories(${LLVM_INCLUDE_DIRS} ${dep_root} ${hdlbe_src})
add_definitions(${LLVM_DEFINITIONS})

# Now build our tools
add_executable(ir_parser ir_parser.cpp ${hdlbe_src}/SchedulingAlgorithm.cpp)

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader)

# Link against LLVM libraries
target_link_libraries(ir_parser ${llvm_libs})
