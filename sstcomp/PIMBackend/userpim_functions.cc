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
  uint64_t numBytes = params[3];
  if( numBytes == 0 )
    return;
  assert((numBytes%8)==0);
  this->dst    = params[0];
  this->src    = params[1];
  this->scalar = params[2];
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
      // TODO use SRAM to save intermediate data
      // TODO better utilities for manage the SST payload
      // For now multiply every dword loaded by the scalar (messy)
      for( int i = 0; i < d.size(); i+=8 ) {
        uint64_t data = 0;
        uint8_t* p = (uint8_t*)(&data);
        for ( int j=0; j<8; j++) {
          p[j] = d[i+j];
        }
        data = scalar*data;
        for (int j=0;j<8;j++) {
          parent->buffer[i+j] = p[j];
        }
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

LFSR::LFSR( TCLPIM* p ) : FSM( p ) {};

void LFSR::start( uint64_t params[NUM_FUNC_PARAMS] ) {
  this->dst    = params[0];
  this->seed   = params[1];
  this->len    = params[2];
  dma_state    = DMA_STATE::IDLE;
  loop_state   = LOOP_STATE::INIT;
  parent->output->verbose(
    CALL_INFO, 3, 0, 
    "LFSR: dst=0x%" PRIx64 " seed=0x%" PRIx64 " len=%" PRIu64 "\n", 
    dst,seed,len);
}

bool LFSR::clock() {
#if 1
  unsigned dwords = this->len >= 64 ? 64 : this->len;
#else
  unsigned dwords = 1;
#endif
  unsigned bytes = dwords * sizeof( uint64_t );

  parent->buffer.resize( bytes );
  const bool WRITE = true;
  const bool READ  = false;

  if( loop_state == LOOP_STATE::INIT ) {
    parent->output->verbose(CALL_INFO,3,0,"LFSR Initializing loop\n");

    lfsr = seed;
    samples = 0;
    cycles = 0;
    buffer_head = 0;
    loop_state = (seed == 0 || len == 0) ? LOOP_STATE::DONE : LOOP_STATE::CYCLING;
    return false;
  }

  if( loop_state == LOOP_STATE::CYCLING ) {
    parent->output->verbose(CALL_INFO,3,0,"LFSR Cycling loop\n");

    // combinational logic
    const uint64_t bit = ((lfsr >> 5)  ^ (lfsr >> 8)  ^ (lfsr >> 10) ^ (lfsr >> 20) ^ 
                    (lfsr >> 27) ^ (lfsr >> 28) ^ (lfsr >> 34) ^ (lfsr >> 39) ^ 
                    (lfsr >> 54) ^ (lfsr >> 56) ^ (lfsr >> 59) ) & 1u;

    // write buffer to memory and empty
    const bool buffer_full = buffer_head == bytes;
    const uint64_t lfsr_next = buffer_full ? lfsr : (lfsr >> 1) | (bit << 63);
    const bool dma_idle         = dma_state == DMA_STATE::IDLE;
    const bool buffer_empty = buffer_head == 0;
    const bool all_samples_collected = samples >= len;
    const bool empty_buffer     = (buffer_full || (!buffer_empty && all_samples_collected)) && dma_idle;
    const uint64_t dst_next         = empty_buffer ? dst + bytes : dst;
    const DMA_STATE dma_state_next  = empty_buffer ? DMA_STATE::WAITING : dma_state;
    if(empty_buffer){
      parent->m_issueDRAMRequest(dst,&parent->buffer,WRITE,[this]( const MemEventBase::dataVec& d ){
        dma_state = DMA_STATE::IDLE;
      });
    }

    // save sample to buffer
    const bool save_sample      = cycles >= (13-1) && !buffer_full && !all_samples_collected;
    const uint64_t buffer_head_next = save_sample ? buffer_head + sizeof(uint64_t) : (empty_buffer ? 0 : buffer_head );
    const uint64_t samples_next     = save_sample ? samples + 1 : samples;
    const uint64_t cycles_next      = buffer_full ? cycles : save_sample ? 0 : cycles + 1;
    if(save_sample){
      uint8_t * p = (uint8_t*)(&lfsr_next);
      for(int i = 0; i < sizeof(uint64_t); i++){
        parent->buffer[buffer_head+i] = p[i];
      }
    }

    // loop exit
    const bool exit = all_samples_collected && buffer_empty && dma_idle;
    const LOOP_STATE loop_state_next = exit ? LOOP_STATE::DONE : loop_state;

    // sequential logic
    lfsr = lfsr_next;
    cycles = cycles_next;
    samples = samples_next;
    buffer_head = buffer_head_next;
    dst = dst_next;
    dma_state = dma_state_next;
    loop_state = loop_state_next;
    return false;
  }

  if( loop_state == LOOP_STATE::DONE ) {
    parent->output->verbose( CALL_INFO, 1, 0, "LFSR done\n" );
    loop_state = LOOP_STATE::IDLE;
    return true;
  }
  return false;
}

} // namespace