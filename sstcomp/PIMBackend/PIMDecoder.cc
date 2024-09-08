//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
// See LICENSE in the top level directory for licensing details
//

#include "PIMDecoder.h"
#include <assert.h>

namespace SST::PIM {

PIMDecoder::PIMDecoder( uint64_t _node ) : node( _node ) {
  assert(node==0);
  // TODO nodeOffset = node * NODE_OFFSET_MULTIPLIER
  nodeOffset=0;
  PIMSegs.emplace_back( std::make_shared<PIMMemSegment>( PIM_ACCESS_TYPE::DRAM, DRAM_BASE, SEG_SIZE ) );
  PIMSegs.emplace_back( std::make_shared<PIMMemSegment>( PIM_ACCESS_TYPE::SRAM, SRAM_BASE + nodeOffset, SEG_SIZE ) );
  PIMSegs.emplace_back( std::make_shared<PIMMemSegment>( PIM_ACCESS_TYPE::F0, FUNC_BASE + nodeOffset, SEG_SIZE ) );
};

PIMDecodeInfo PIMDecoder::decode( const uint64_t& addr ) {
  PIMDecodeInfo info;
  for( auto seg : PIMSegs ) {
    if( seg->decode( addr ) != PIM_ACCESS_TYPE::NONE ) {
      info.pimAccType = seg->getType();
      break;
    }
  }
  info.isIO   = ( info.pimAccType == PIM_ACCESS_TYPE::F0 ) || ( info.pimAccType == PIM_ACCESS_TYPE::SRAM );
  info.isDRAM = info.pimAccType == PIM_ACCESS_TYPE::DRAM;
  return info;
}

}  // namespace SST::PIM

// EOF
