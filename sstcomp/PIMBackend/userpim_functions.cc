//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
// See LICENSE in the top level directory for licensing details
//

#include "userpim_functions.h"

namespace SST::PIM {

MulVecByScalar::MulVecByScalar( TCLPIM* p ) : FSM( p ) {};

void MulVecByScalar::start( uint64_t params[NUM_FUNC_PARAMS] ) {
  uint64_t numBytes = params[2];
  if( numBytes == 0 )
    return;
  assert((numBytes%8)==0);
  this->dst    = params[1];
  this->src    = params[2];
  this->scalar = 10;
  total_words  = numBytes/8;
  word_counter = numBytes/8;
  dma_state    = DMA_STATE::READ;
  parent->output->verbose(
    CALL_INFO, 3, 0, 
    "MulVecByScalar: dst=0x%" PRIx64 " src=0x%" PRIx64 "scalar=" PRId64 " total_words=%" PRId64 "\n", 
    dst, src, scalar, total_words);
}

bool MulVecByScalar::clock() {
#if 1
  unsigned words = word_counter >= 64 ? 64 : word_counter;
#else
  unsigned words = 1;
#endif
  unsigned bytes = words * sizeof( uint64_t );
  //if (parent->buffer.size() != bytes)
  parent->buffer.resize( bytes );
  const bool WRITE = true;
  const bool READ  = false;
  if( dma_state == DMA_STATE::READ ) {
    parent->m_issueDRAMRequest( src, &parent->buffer, READ, [this]( const MemEventBase::dataVec& d ) {
      assert( parent->buffer.size() == d.size() );
      for( int i = 0; i < d.size(); i++ ) {
        parent->buffer[i] = d[i];
      }
      dma_state = DMA_STATE::WRITE;
    } );
    dma_state = DMA_STATE::WAITING;
    src += bytes;
  } else if( dma_state == DMA_STATE::WRITE ) {
    assert( word_counter >= words );
    word_counter = word_counter - words;
    if( word_counter > 0 ) {
      parent->m_issueDRAMRequest( dst, &parent->buffer, WRITE, [this]( const MemEventBase::dataVec& d ) {
        dma_state = DMA_STATE::READ;
      } );
    } else {
      parent->m_issueDRAMRequest( dst, &parent->buffer, WRITE, [this]( const MemEventBase::dataVec& d ) {
        dma_state = DMA_STATE::DONE;
      } );
    }
    dst += bytes;
    dma_state = DMA_STATE::WAITING;
  } else if( dma_state == DMA_STATE::DONE ) {
    parent->output->verbose( CALL_INFO, 1, 0, "DMA Done\n" );
    dma_state = DMA_STATE::IDLE;
    return true;  // finished!
  }
  return false;
}



} // namespace