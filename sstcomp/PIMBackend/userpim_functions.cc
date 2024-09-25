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
    "MulVecByScalar: dst=0x%" PRIx64 " src=0x%" PRIx64 "scalar=%" PRId64 " total_words=%" PRId64 "\n", 
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
      for( size_t i = 0; i < d.size(); i+=8 ) {
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
    const bool dump_buffer     = (buffer_full || (!buffer_empty && all_samples_collected)) && dma_idle;
    const uint64_t dst_next         = dump_buffer ? dst + bytes : dst;
    const DMA_STATE dma_state_next  = dump_buffer ? DMA_STATE::WAITING : dma_state;
    if(dump_buffer){
      parent->m_issueDRAMRequest(dst,&parent->buffer,WRITE,[this]( const MemEventBase::dataVec& d ){
        dma_state = DMA_STATE::IDLE;
      });
    }

    // save sample to buffer
    const bool save_sample      = cycles >= (13-1) && !buffer_full && !all_samples_collected;
    const uint64_t buffer_head_next = save_sample ? buffer_head + sizeof(uint64_t) : (dump_buffer ? 0 : buffer_head );
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

SymmetricDistanceMatrix::SymmetricDistanceMatrix( TCLPIM* p) : FSM( p ) {};

void SymmetricDistanceMatrix::start( uint64_t params [NUM_FUNC_PARAMS] ) {
  this->matrix_base_addr = params[0];
  this->rand_base_addr   = params[1];
  this->dist_mask        = params[2];
  this->vertices         = params[3];

  // random sources must be in SRAM
  assert(PIMDecoder::decode(this->rand_base_addr).pimAccType == PIM_ACCESS_TYPE::SRAM);

  dma_state   = DMA_STATE::IDLE;
  loop_state  = LOOP_STATE::INIT_ROW;
  curr_row    = 0;
  curr_col    = 0;
  row_loaded  = false;
  col_loaded  = false;
  matrix_head = 0;
  buffer_head = 0;
  const uint32_t p = 29 - __builtin_clz(vertices);
  prob_mask = (1 << p) - 1;

  parent->output->verbose(
    CALL_INFO, 3, 0, 
    "SymmetricDistanceMatrix: matrix_base_addr=0x%" PRIx64 " rand_base_addr=0x%" PRIx64 " dist_mask=0x%" PRIx64 " vertices=%" PRIu64 "\n", 
    matrix_base_addr,rand_base_addr,dist_mask,vertices);
}

bool SymmetricDistanceMatrix::clock() {
#if 1
  unsigned dwords = this->vertices >= 64 ? 64 : this->vertices;
#else
  unsigned dwords = 1;
#endif
  unsigned bytes = dwords * sizeof( uint64_t );

  parent->buffer.resize( bytes );
  const bool WRITE = true;

  dma_state.clock();
  loop_state.clock();
  curr_row.clock();
  curr_col.clock();
  row_loaded.clock();
  col_loaded.clock();
  matrix_head.clock();
  buffer_head.clock();

  if(loop_state() == LOOP_STATE::INIT_ROW) {
    parent->output->verbose(CALL_INFO,3,0,"SymmetricDistanceMatrix Init Row\n");

    // exit when all rows complete
    const bool all_rows_complete = curr_row() >= vertices;
    const bool buffer_empty = buffer_head() == 0;
    if(all_rows_complete){
      loop_state = buffer_empty ? LOOP_STATE::DONE : LOOP_STATE::CLEANUP;
    }

    // load current row into buffer
    const bool load_row = !all_rows_complete && !row_loaded();
    if(load_row){
      const uint64_t curr_row_mem_addr = rand_base_addr + (curr_row() * sizeof(uint64_t));
      parent->read(curr_row_mem_addr, sizeof(uint64_t), curr_row_buffer);
      row_loaded = true;
    }

    // start column cycle
    if(row_loaded()){
      loop_state = LOOP_STATE::CYCLE;
      curr_col = 0;
      col_loaded = false;
    }

    return false;
  }
  if(loop_state() == LOOP_STATE::CYCLE) {
    parent->output->verbose(CALL_INFO,3,0,"SymmetricDistanceMatrix Cycle\n");

    // shift random values into sram if curr_col is out of bounds
    const bool load_col = !col_loaded();
    if(load_col) {
      const uint64_t curr_col_mem_addr = rand_base_addr + (curr_col() * sizeof(uint64_t));
      parent->read(curr_col_mem_addr,sizeof(uint64_t), curr_col_buffer);
      col_loaded = true;
    }

    // do calculation once column is loaded
    const bool buffer_full = buffer_head() == bytes;
    const bool do_calculation = col_loaded() && !buffer_full;
    if(do_calculation){
      uint64_t distance = 0;
      if(curr_row() != curr_col()) {
        // aggregate buffers into double word scalars
        uint64_t col;
        uint64_t row;
        uint8_t * col_p = (uint8_t*) &col;
        uint8_t * row_p = (uint8_t*) &row;
        for(uint64_t i = 0;i<sizeof(uint64_t);i++){
          row_p[i] = curr_row_buffer[i];
          col_p[i] = curr_col_buffer[i];
        }
        // const uint64_t curr_col_mem_addr = rand_base_addr + (curr_col() * sizeof(uint64_t));
        // const uint64_t curr_row_mem_addr = rand_base_addr + (curr_row() * sizeof(uint64_t));

        // parent->output->verbose(CALL_INFO,3,0,"SymmetricDistanceMatrixHW calculating dist for r=%u c=%u addr_r=0x%llx addr_r=0x%llx rand_r=%llu rand_c=%llu \n",
        //   curr_row(),curr_col(),curr_row_mem_addr, curr_col_mem_addr, row,col);
        
        // copied from software function
        const uint64_t xor_rand = row ^ col;
        const uint32_t xor_rand_msb = xor_rand >> 32;
        const uint32_t xor_rand_lsb = xor_rand & UINT32_MAX;
        const bool setEdge = (xor_rand_msb & prob_mask) == prob_mask;
        distance = setEdge ? xor_rand_lsb & dist_mask : UINT64_MAX;

      }

      // copy distance for edge (r,c) into buffer
      const uint8_t * p = (uint8_t*) &distance;
      for(unsigned i = 0; i < sizeof(uint64_t); i++){
        parent->buffer[buffer_head()+i] = p[i];
      }

      // set up for next column
      // buffer head may reset to 0 in same cycle. cannot drive here.
      curr_col = curr_col() + 1;
      col_loaded = false;
    }
    
    // empty buffer if full or row_complete
    const bool buffer_empty = buffer_head() == 0;
    const bool dump_buffer = (buffer_full) && (dma_state() == DMA_STATE::IDLE);
    const unsigned value_bytes_in_buffer = do_calculation ? buffer_head() + sizeof(uint64_t) : buffer_head();
    buffer_head = dump_buffer ? 0 : value_bytes_in_buffer;
    if(dump_buffer) {
      dma_state = DMA_STATE::WRITE;
      matrix_head = matrix_head() + value_bytes_in_buffer;
      parent->m_issueDRAMRequest(
        matrix_base_addr + matrix_head(), 
        &parent->buffer, 
        WRITE,
        [this](const MemEventBase::dataVec& d){
          dma_state = DMA_STATE::IDLE;
      });
    }

    // set up for next row
    const bool all_cols_complete = (curr_col() == vertices - 1) && do_calculation;
    if(all_cols_complete){
      curr_row = curr_row() + 1;
      row_loaded = false;
      loop_state = LOOP_STATE::INIT_ROW;
    }
  
    return false;
  }
  if(loop_state() == LOOP_STATE::CLEANUP) {
    parent->output->verbose(CALL_INFO,3,0,"SymmetricDistanceMatrix Init\n");

    // dump buffer before exiting
    const bool buffer_empty = buffer_head() == 0;
    const bool dump_buffer = !buffer_empty && (dma_state() == DMA_STATE::IDLE);
    if(dump_buffer) {
      dma_state = DMA_STATE::WRITE;
      matrix_head = matrix_head() + buffer_head();
      buffer_head = 0;
      parent->m_issueDRAMRequest(
        matrix_base_addr + matrix_head(), 
        &parent->buffer, 
        WRITE,
        [this](const MemEventBase::dataVec& d){
          dma_state = DMA_STATE::IDLE;
      });
    }

    // exit after dma is finished
    const bool done = buffer_empty && (dma_state() == DMA_STATE::IDLE);
    if(done){
      loop_state = LOOP_STATE::DONE;
    }

    return false;
  }
  if(loop_state() == LOOP_STATE::DONE) {
    parent->output->verbose(CALL_INFO,3,0,"SymmetricDistanceMatrix Done\n");
    loop_state = LOOP_STATE::IDLE;
    return true;
  }
}
} // namespace
