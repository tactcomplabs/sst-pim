//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
// See LICENSE in the top level directory for licensing details
//

#include "tclpim_functions.h"

namespace SST::PIM {

MemCopy::MemCopy( TCLPIM* p ) : FSM( p ) {};

// Param 0: Destination Address
// Param 1: Source Address
// Param 2: Number of Bytes to transfer ( must by divisible by 8 ) 
// TODO: Check address ranges for SRAM and FUNC overlap. Should be able to support DRAM<->SRAM and SRAM<->SRAM

void MemCopy::start( uint64_t params[NUM_FUNC_PARAMS] ) {
  uint64_t numBytes = params[2];
  if( numBytes == 0 )
    return;
  assert((numBytes%8)==0);
  this->dst    = params[0];
  this->src    = params[1];
  total_words  = numBytes/8;
  word_counter = numBytes/8;
  dma_state    = DMA_STATE::READ;
  parent->output->verbose(
    CALL_INFO, 3, 0, "start dma: dst=0x%" PRIx64 " src=0x%" PRIx64 " total_words=%" PRId64 "\n", dst, src, total_words
  );
}

bool MemCopy::clock() {
#if 1
  unsigned words = word_counter >= 64 ? 64 : word_counter;
#else
  unsigned words = 1;
#endif
  unsigned bytes = words * sizeof( uint64_t );
  parent->buffer.resize( bytes );
  switch (dma_state) {
    case DMA_STATE::READ:
      sequence_dram_read(DMA_STATE::WRITE);
      dma_state = DMA_STATE::WAITING;
      src += bytes;
      break;
    case DMA_STATE::WRITE:
      assert( word_counter >= words );
      word_counter = word_counter - words;
      if( word_counter > 0 ) {
        sequence_dram_write(DMA_STATE::READ);
      } else {
        sequence_dram_write(DMA_STATE::DONE);
      }
      dst += bytes;
      dma_state = DMA_STATE::WAITING;
      break;
    case DMA_STATE::DONE:
      parent->output->verbose( CALL_INFO, 1, 0, "DMA Done\n" );
      dma_state = DMA_STATE::IDLE;
      return true;  // finished!
    default:
      break;
  }
  return false;
}

void MemCopy::sequence_dram_read(DMA_STATE nextState)
{
  parent->m_issueDRAMRequest(src, &parent->buffer, false, [this, nextState]( const MemEventBase::dataVec& d ) 
    {
      assert( parent->buffer.size() == d.size() );
      for( size_t i = 0; i < d.size(); i++ ) {
        parent->buffer[i] = d[i];
      }
      dma_state = nextState;
    });
}

void MemCopy::sequence_dram_write(DMA_STATE nextState)
{
  parent->m_issueDRAMRequest( dst, &parent->buffer, true, [this, nextState]( const MemEventBase::dataVec& d ) {
    dma_state = nextState;
  } );
}

} // namespace