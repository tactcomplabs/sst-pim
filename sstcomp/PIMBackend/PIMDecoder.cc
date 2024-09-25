//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
// See LICENSE in the top level directory for licensing details
//

#include "PIMDecoder.h"
#include <assert.h>

namespace SST::PIM {

//TODO remove dependencies
PIMDecoder::PIMDecoder( uint64_t _node ) : node( _node ) {
  assert(node==0);
  // TODO nodeOffset = node * NODE_OFFSET_MULTIPLIER
  nodeOffset=0;
};

PIMDecodeInfo PIMDecoder::decode( const uint64_t& addr ) {
  assert(_allSegmentsInit);
  PIMDecodeInfo info;

  if(addr < _funcBaseAddr) {
    return info;
  }

  if(addr < _sramBaseAddr){
    info.pimAccType = PIM_ACCESS_TYPE::FUNC;
    info.isIO = true;
    info.isDRAM = false;
    return info;
  }

  if(addr < _dramBaseAddr){
    info.pimAccType = PIM_ACCESS_TYPE::SRAM;
    info.isIO = true;
    info.isDRAM = false;
    return info;
  }

  if(addr < _regBoundAddr){
    info.pimAccType = PIM_ACCESS_TYPE::DRAM;
    info.isIO = false;
    info.isDRAM = true;
    return info;
  }

  return info;
}

void PIMDecoder::setPIMSegments(  const uint64_t funcBaseAddr,
                                  const uint64_t sramBaseAddr,
                                  const uint64_t dramBaseAddr,
                                  const uint64_t regBoundAddr) {
    assert(!_allSegmentsInit);
    assert((funcBaseAddr & 0x7) == 0);
    assert((sramBaseAddr & 0x7) == 0);
    assert((dramBaseAddr & 0x7) == 0);
    assert((regBoundAddr & 0x7) == 0);
    assert(sramBaseAddr >= (funcBaseAddr + (SST::PIM::FUNC_LEN * sizeof(uint64_t))));
    assert(dramBaseAddr > sramBaseAddr);
    assert(regBoundAddr > dramBaseAddr);
    _funcBaseAddr = funcBaseAddr;
    _sramBaseAddr = sramBaseAddr;
    _dramBaseAddr = dramBaseAddr;
    _regBoundAddr = regBoundAddr;
    _allSegmentsInit = true;
  };
}  // namespace SST::PIM

// EOF
