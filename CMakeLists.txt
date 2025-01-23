#
# sst-pim top-level CMake
#
# Copyright (C) 2024-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

#-- Prevent in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "DO NOT BUILD in-tree.")
endif()

# Minimum required version of CMake and project information
cmake_minimum_required(VERSION 3.19)
project(sstpim CXX)
# Make sure Git hooks are set properly
execute_process(COMMAND_ERROR_IS_FATAL ANY COMMAND_ECHO STDERR COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/test_git_hooks.sh)

#-- SST Setup
execute_process(COMMAND sst-config --prefix
                OUTPUT_VARIABLE SST_INSTALL_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(SST_INCLUDE_DIR "${SST_INSTALL_DIR}/include")
if(NOT (EXISTS "${SST_INSTALL_DIR}"))
  message(FATAL_ERROR " SST_INSTALL_DIR (${SST_INSTALL_DIR}) is invalid.")
endif()
# include_directories(SST_SRC ${SST_INSTALL_DIR})

# message(STATUS "SST_INSTALL_DIR = ${SST_INSTALL_DIR}")
# message(STATUS "CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")

# SST Configuration Sanity Check
find_program(SST sst)
find_program(SST_CONFIG sst-config)
if(NOT SST OR NOT SST_CONFIG)
  message(FATAL_ERROR "No SST binary or sst-config binary found in path")
endif()

# SST Environment Derivation
execute_process(COMMAND sst-config --CXX
                OUTPUT_VARIABLE CXX
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND sst-config --ELEMENT_CXXFLAGS
                OUTPUT_VARIABLE SST_CXXFLAGS
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(CXXFLAGS "${SST_CXXFLAGS}  -fno-stack-protector")
execute_process(COMMAND sst-config --ELEMENT_LDFLAGS
                OUTPUT_VARIABLE SST_LDFLAGS
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(LDFLAGS "${SST_LDFLAGS}  -fno-stack-protector")

execute_process(COMMAND sst-config SST_ELEMENT_LIBRARY SST_ELEMENT_LIBRARY_LIBDIR
                OUTPUT_VARIABLE SST_ELEMENT_LIBRARY_LIBDIR
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/sst-major-version.sh
                OUTPUT_VARIABLE SST_MAJOR_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE
                )

message(STATUS "SST MAJOR VERSION=${SST_MAJOR_VERSION}")

#-- Compiler Options
# Compiler Options
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(FP_MODE_FLAG "-ffp-model=strict -frounding-math -ftrapping-math")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-command-line-argument")
else()
  set(FP_MODE_FLAG "-frounding-math -ftrapping-math")
  #TODO set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wmissing-format-attribute -Wsuggest-final-methods -Wsuggest-final-types -Wvolatile")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wmissing-format-attribute -Wvolatile")
endif()

set(WERROR_FLAG "-Werror")
set(CMAKE_CXX_FLAGS "-std=c++17 ${FP_MODE_FLAG} -O2 -Wall -Wextra -Wsuggest-override -Wmissing-noreturn -Wvla -Wuninitialized -Wdouble-promotion -Wsign-conversion -Wconversion -Wno-unused-parameter -Wno-deprecated-declarations -Wno-macro-redefined ${WERROR_FLAG} ${CMAKE_CXX_FLAGS} -I./ ${LDFLAGS} ${REVCPU_COMPILER_MACROS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall ${REVCPU_COMPILER_MACROS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall ${REVCPU_COMPILER_MACROS}")

#-- REV repository root directory
if (NOT DEFINED ENV{REVHOME})
  message(FATAL_ERROR "REVHOME not set")
endif()

if(NOT EXISTS $ENV{REVHOME})
  message(FATAL_ERROR " REVHOME (ENV{REVHOME}) is invalid.")
endif()
#-- SSTPIM Paths
set(SSTPIM_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/sstcomp/include" CACHE PATH "sst-pim/sstcomp include path")
include_directories(SSTPIM_INCLUDE ${SSTPIM_INCLUDE_PATH})

#---------------------------------------------
# TARGET OPTIONS
#---------------------------------------------

#-- Testing Options
option(SSTPIM_ENABLE_TESTING "Enable Testing" ON)
if(SSTPIM_ENABLE_TESTING)
 enable_testing()
 add_subdirectory(test)
endif()

#-- Component source directories
add_subdirectory(sstcomp)
#-- Custom Commands
add_custom_target(uninstall COMMAND
  "${CMAKE_CURRENT_SOURCE_DIR}/scripts/uninstall.sh")


# EOF
