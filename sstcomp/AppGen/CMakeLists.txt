#
# sst-bench/sst/AppGen CMake
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

set(AppGenSrcs
  app.cc
  appTest.cc
  appTransactor.cc
  mirandaCPU_kg.cc
  singlestream_kg.cc
)

add_library(AppGen SHARED ${AppGenSrcs})
target_include_directories(AppGen  PUBLIC ${SST_INSTALL_DIR}/include)
install(TARGETS AppGen DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})

# register sst components
install(CODE "execute_process(COMMAND sst-register RequestGenCPU_KG  RequestGenCPU_KG_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
install(CODE "execute_process(COMMAND sst-register SingleStreamGenerator_KG SingleStreamGenerator_KG_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
# register application tests
install(CODE "execute_process(COMMAND sst-register AppxTest AppxTest_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")

# EOF
