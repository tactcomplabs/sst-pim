#!/bin/bash
#
# Unregisters libs from the SST infrastructure
#

#-- unregister the libs
sst-register -u AppxTest
sst-register -u RequestGenCPU_KG
sst-register -u SingleStreamGenerator_KG
sst-register -u MemControllerKG
sst-register -u PIMMemController
sst-register -u PIMBackend

#-- forcible remove it from the local script
CONFIG=~/.sst/sstsimulator.conf
if test -f "$CONFIG"; then
  echo "Removing configuration from local config=$CONFIG"
  sed -i.bak '/AppxTest/d' $CONFIG
  sed -i.bak '/RequestGenCPU_KG/d' $CONFIG
  sed -i.bak '/SingleStreamGenerator_KG/d' $CONFIG
  sed -i.bak '/MemControllerKG/d' $CONFIG
  sed -i.bak '/PIMMemController/d' $CONFIG
  sed -i.bak '/PIMBackend/d' $CONFIG
fi

# EOF
