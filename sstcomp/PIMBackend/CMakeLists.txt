#
# sst-bench/sst/PIMBackend CMake
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

set(PIMBackendSrcs
  PIMBackend.cc
  PIMBackend.h
  PIMDecoder.cc
  PIMDecoder.h
  PIMMemController.cc
  PIMMemController.h
  memoryControllerKG.cc
  memoryControllerKG.h
  pim.cc
  pim.h
  tclpim.cc
  tclpim.h
  memHierarchy/cacheListener.h
  memHierarchy/memEvent.h
  memHierarchy/memEventBase.h
  memHierarchy/memEventCustom.h
  memHierarchy/memLink.h
  memHierarchy/memNIC.h
  memHierarchy/util.h
)

# some includes needed from sst elements
execute_process(COMMAND sst-config SST_ELEMENT_LIBRARY SST_ELEMENT_LIBRARY_HOME
                OUTPUT_VARIABLE SST_ELEMENTS_HOME
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(SST_ELEMENTS_INCLUDE_DIR "${SST_ELEMENTS_HOME}/include")
if(NOT (EXISTS "${SST_ELEMENTS_INCLUDE_DIR}"))
  message(FATAL_ERROR " SST_ELEMENTS_INCLUDE_DIR (${SST_ELEMENTS_INCLUDE_DIR}) is invalid.")
endif()
include_directories(SST_ELEMENTS_INC ${SST_ELEMENTS_INCLUDE_DIR})
message(STATUS "SST_ELEMENTS_INC = ${SST_ELEMENTS_INCLUDE_DIR}")

add_library(PIM SHARED ${PIMBackendSrcs})
target_include_directories(PIM  
  PUBLIC ${SST_INSTALL_DIR}/include
  PUBLIC memHierarchy)
install(TARGETS PIM DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})

# register sst components
install(CODE "execute_process(COMMAND sst-register MemControllerKG  MemControllerKG_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
install(CODE "execute_process(COMMAND sst-register PIMMemController PIMMemController_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
install(CODE "execute_process(COMMAND sst-register PIMBackend PIMBackend_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")

# EOF
