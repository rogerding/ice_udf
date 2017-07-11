# Copyright 2012 Cloudera Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 2.6)
#cmakedefine BOOST_DATE_TIME_POSIX_TIME_STD_CONFIG
# where to put generated libraries
add_definitions(-DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG)

set(LIBRARY_OUTPUT_PATH "build")
# where to put generated binaries
set(EXECUTABLE_OUTPUT_PATH "build")

find_program(CLANG_EXECUTABLE clang++)


set(Boost_INCLUDE_DIR /home/rding/software/boost_1_61_0/)
set(BOOST_LIBRARYDIR /home/rding/software/boost_1_61_0/libs)
find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package ( Boost COMPONENTS system filesystem regex program_options date_time REQUIRED )
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

#set(BOOST_ROOT /home/cloudera/impala_udfs/impala-udf-samples-master/boost_1_47_0)
#set(Boost_USE_STATIC_LIBS   ON)
#find_package(Boost REQUIRED ...)

#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb")

# Function to generate rule to cross compile a source file to an IR module.
# This should be called with the .cc src file and it will generate a
# src-file-ir target that can be built.
# e.g. COMPILE_TO_IR(test.cc) generates the "test-ir" make target.
# Disable compiler optimizations because generated IR is optimized at runtime
set(IR_COMPILE_FLAGS "-emit-llvm" "-c")
function(COMPILE_TO_IR SRC_FILE)
  get_filename_component(BASE_NAME ${SRC_FILE} NAME_WE)
  set(OUTPUT_FILE "build/${BASE_NAME}.ll")
  add_custom_command(
    OUTPUT ${OUTPUT_FILE}
    COMMAND ${CLANG_EXECUTABLE} ${IR_COMPILE_FLAGS} ${SRC_FILE} -o ${OUTPUT_FILE}
    DEPENDS ${SRC_FILE})
  add_custom_target(${BASE_NAME}-ir ALL DEPENDS ${OUTPUT_FILE})
endfunction(COMPILE_TO_IR)

# Build the UDA/UDFs into a shared library.
#add_library(udfsample SHARED udf-sample.cc)
#add_library(formatting SHARED formatting.cc)
#add_library(ice_diff SHARED ice_diff.cc)
#add_library(trunc SHARED trunc.cc)
#add_library(last_day SHARED last_day.cc)
#add_library(age SHARED age.cc)
add_library(ice_to_char SHARED ice_to_char.cc)
add_library(ice_to_datets SHARED ice_to_datets.cc)

#add_library(udasample SHARED uda-sample.cc hyperloglog-uda.cc variance-uda.cc)

# Custom targest to cross compile UDA/UDF to ir
if (CLANG_EXECUTABLE)
 # COMPILE_TO_IR(udf-sample.cc )
  #COMPILE_TO_IR(uda-sample.cc )
  #COMPILE_TO_IR(formatting.cc)
  #COMPILE_TO_IR(trunc.cc)
  #COMPILE_TO_IR(last_day.cc)
  #COMPILE_TO_IR(age.cc)
  #COMPILE_TO_IR(ice_diff.cc)
  COMPILE_TO_IR(ice_to_char.cc)
  COMPILE_TO_IR(ice_to_datets.cc)
endif(CLANG_EXECUTABLE)

target_link_libraries(ice_to_char ImpalaUdf)
target_link_libraries(ice_to_datets ImpalaUdf)

# This is an example of how to use the test harness to help develop UDF and UDAs.
#target_link_libraries(udfsample ImpalaUdf)
#target_link_libraries(udasample ImpalaUdf)
#add_executable(udf-sample-test udf-sample-test.cc)
#target_link_libraries(udf-sample-test udfsample)
#add_executable(uda-sample-test uda-sample-test.cc)
#target_link_libraries(uda-sample-test udasample)
#target_link_libraries( udf-sample-test ${Boost_LIBRARIES} )
