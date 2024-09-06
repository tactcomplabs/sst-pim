#
# sst-pim/sst CMake
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

message(STATUS "[SST-PIM] Enabling AppGen")
add_subdirectory(AppGen)

message(STATUS "[SST-PIM] Enabling PIMBackend")
add_subdirectory(PIMBackend)

# EOF
