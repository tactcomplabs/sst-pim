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
    parent->output->verbose(CALL_INFO,3,0,"SymmetricDistanceMatrix Cleanup\n");

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

AStar::AStar( TCLPIM* p) : FSM( p ) {
  const uint64_t uint32_max = UINT32_MAX;
  std::memcpy(uint32_max_vec.data(), &uint32_max, sizeof(uint32_max));
};

void AStar::start( uint64_t params [NUM_FUNC_PARAMS] ) {
  this->ret_code_addr       = params[0];
  this->came_from_base_addr = params[1];
  this->matrix_base_addr    = params[2];
  this->src                 = params[3];
  this->target              = params[4];
  this->vertices            = params[5];

  // check address are in correct memory regions
  assert(PIMDecoder::decode(ret_code_addr).pimAccType == PIM_ACCESS_TYPE::SRAM);
  assert(PIMDecoder::decode(came_from_base_addr).isDRAM);
  assert(PIMDecoder::decode(matrix_base_addr).isDRAM);

  // preset state values and sequential logic
  dma_state = DMA_STATE::IDLE;
  loop_state = LOOP_STATE::INIT;
  open_set_index = 0;
  buffer_head = 0;
  buffer_tail = 0;

  // optimize buffer size if small graph
  const unsigned graph_size = (vertices*vertices*sizeof(uint64_t));
  const size_t buffer_size = graph_size >= 64 ? 64 : graph_size;
  parent->buffer.resize(buffer_size);
}

bool AStar::clock() {
  dma_state.clock();
  loop_state.clock();
  open_set_index.clock();
  lowest_fscore.clock();
  lowest_fscore_index.clock();
  neighbor.clock();
  buffer_head.clock();
  buffer_tail.clock();
  curr_fscore_loaded.clock();
  curr_gscore_loaded.clock();
  neighbor_gscore_loaded.clock();
  path_found.clock();

  if(loop_state() == LOOP_STATE::INIT) {
    parent->output->verbose(CALL_INFO,3,0,"AStar Init\n");
    
    //write 0 to openset[i], infinity to fscore[i] except 0 to fscore[src]
    const uint64_t write_addr = PIMDecoder::getSramBaseAddr() + (open_set_index() * sizeof(uint64_t));
    const bool write_open_set_src = (open_set_index() == src) && (dma_state() == DMA_STATE::IDLE);
    const bool write_open_set_nonsrc = (open_set_index() != src);
    if(write_open_set_src) {
      parent->write(write_addr,sizeof(uint64_t),&zero_vec);
      MemEventBase::dataVec src_vec(sizeof(src));
      std::memcpy(src_vec.data(),&src,sizeof(src));

      // init came_from[src] = src
      dma_state = DMA_STATE::BUSY;
      const uint64_t src_came_from_addr = came_from_base_addr + (src * sizeof(uint64_t));
      parent->m_issueDRAMRequest(src_came_from_addr,&src_vec,DMA_WRITE,[this]( const MemEventBase::dataVec& d ) {
        parent->output->verbose(CALL_INFO,3,0,"came_from[src] = src returned\n");
        dma_state = DMA_STATE::IDLE;
      });
      open_set_index = open_set_index() + 1;
    }
    if(write_open_set_nonsrc) {
      parent->write(write_addr,sizeof(uint64_t),&uint32_max_vec);
      open_set_index = open_set_index() + 1;
    }

    //exit after open_set is filled, skip POP_OPEN_SET, openset[src] already "popped"
    const bool all_open_set_complete = (open_set_index() == vertices - 1) && (write_open_set_nonsrc || write_open_set_src);
    if(all_open_set_complete){
      loop_state = LOOP_STATE::CYCLE;
      lowest_fscore_index = src;
      curr_gscore_loaded = false;
      neighbor_gscore_loaded = false;
      neighbor = 0;
    }
  }
  if(loop_state() == LOOP_STATE::POP_OPEN_SET){
    parent->output->verbose(CALL_INFO,3,0,"AStar Pop Open Set\n");

    // load openSet[i] and fScore[i]. reuse gscore buffers
    const uint64_t open_set_index_addr = PIMDecoder::getSramBaseAddr() + (open_set_index() * sizeof(uint64_t));
    if(!curr_fscore_loaded()){
      parent->read(open_set_index_addr,sizeof(uint64_t),curr_fscore_buffer);
      curr_fscore_loaded = true;
    }

    // check if vertex is in openst and if fscore less than lowest_fscore
    bool found_lower_fscore;
    if(curr_fscore_loaded()){
      uint64_t curr_open_set_entry;
      std::memcpy(&curr_open_set_entry,curr_fscore_buffer.data(),sizeof(curr_open_set_entry));
      bool in_open_set = (curr_open_set_entry & INT64_MIN) != 0;
      uint32_t curr_fscore = curr_open_set_entry & UINT32_MAX;
      found_lower_fscore = in_open_set && (curr_fscore < lowest_fscore());
      if(found_lower_fscore){
        lowest_fscore = curr_fscore;
        lowest_fscore_index = open_set_index();
      }
      
      curr_fscore_loaded = false;
      open_set_index = open_set_index() + 1;
    }

    // exit loop after all vertices in open set are checked
    const bool all_open_set_checked = (open_set_index() == vertices - 1) && curr_fscore_loaded();
    if(all_open_set_checked){
      loop_state = LOOP_STATE::CYCLE;
      curr_gscore_loaded = false;
      neighbor_gscore_loaded = false;
      neighbor = 0;
    }

    // pop open_set[i] if any found
    const unsigned pop_open_set_index = found_lower_fscore ? open_set_index() : lowest_fscore_index();
    const bool pop_open_set = all_open_set_checked && (pop_open_set_index != UINT32_MAX);
    if(pop_open_set){
      const uint64_t lowest_fscore_64 = lowest_fscore();
      MemEventBase::dataVec lowest_fscore_vec = MemEventBase::dataVec(sizeof(lowest_fscore_64));
      std::memcpy(lowest_fscore_vec.data(),&lowest_fscore_64,sizeof(lowest_fscore_64));
      const uint64_t lowest_fscore_index_addr = PIMDecoder::getSramBaseAddr() + (pop_open_set_index * sizeof(uint64_t));
      parent->write(lowest_fscore_index_addr,sizeof(uint64_t),&lowest_fscore_vec);
    }
   
  }
  if(loop_state() == LOOP_STATE::CYCLE) {
    parent->output->verbose(CALL_INFO,3,0,"AStar Cycle lowestfscoreindex=%u target=%u\n",lowest_fscore_index(),target);
    // return success if target is reached
    const bool curr_is_target = lowest_fscore_index() == target;
    if(curr_is_target) {
      loop_state = LOOP_STATE::CLEANUP;
      path_found = true;
    }

    // return fail if target is not reached
    const bool open_set_is_empty = lowest_fscore_index() == UINT32_MAX;
    if(open_set_is_empty) {
      loop_state = LOOP_STATE::CLEANUP;
      path_found = false;
    }

    // load neighbor into buffer if not loaded (sync load)
    const unsigned neighbor_edge_index = (lowest_fscore_index() * vertices) + neighbor();
    const bool neighbor_edge_loaded = (buffer_tail() <= neighbor_edge_index) && (neighbor_edge_index < buffer_head());
    const bool roll_matrix_window = !neighbor_edge_loaded && dma_state() == DMA_STATE::IDLE;
    if(roll_matrix_window) {
      dma_state = DMA_STATE::BUSY;
      const uint64_t neighbor_edge_addr = matrix_base_addr + (neighbor_edge_index * sizeof(uint64_t));
      parent->m_issueDRAMRequest(
        neighbor_edge_addr,
        &parent->buffer,
        DMA_READ,
        [this](const MemEventBase::dataVec & d){
          assert(parent->buffer.size() == d.size());
          std::memcpy(parent->buffer.data(), d.data(), d.size());
          const uint64_t * p = (uint64_t*)parent->buffer.data();
          const uint64_t * p2 = (uint64_t*)d.data();
          for(unsigned i = 0; i < 8; i++){
            parent->output->verbose(CALL_INFO,3,0,"d[%u]=%llu buffer[%u]=%llu\n",i,p2[i],i,p[i]);
          }
          dma_state = DMA_STATE::IDLE;
          const unsigned neighbor_edge_index = (lowest_fscore_index() * vertices) + neighbor();
          buffer_head = neighbor_edge_index + (d.size() / sizeof(uint64_t));
          buffer_tail = neighbor_edge_index;
          parent->output->verbose(CALL_INFO,3,0,"buffer = rolling widnow returned\n");
        }
      );
    }

    // load g scores if neighbor has an edge
    uint64_t neighbor_distance;
    bool neighbor_has_edge;
    if(neighbor_edge_loaded){
      const uint64_t neighbor_buffer_index = neighbor_edge_index - buffer_tail();
      std::memcpy(&neighbor_distance, ((uint64_t*)parent->buffer.data()) + neighbor_buffer_index,sizeof(uint64_t));
      neighbor_has_edge = neighbor_distance < UINT64_MAX;
      const bool load_curr_gscore = neighbor_has_edge && neighbor_edge_loaded && !curr_gscore_loaded();
      if(load_curr_gscore){
        const uint64_t curr_gscore_addr = PIMDecoder::getSramBaseAddr() + (lowest_fscore_index() * sizeof(uint64_t));
        parent->read(curr_gscore_addr, sizeof(uint64_t), curr_gscore_buffer);
        curr_gscore_loaded = true;
      }
    }

    const uint64_t neighbor_gscore_addr = PIMDecoder::getSramBaseAddr() + (neighbor() * sizeof(uint64_t));
    const bool load_neighbor_gscore = curr_gscore_loaded() && !neighbor_gscore_loaded() && neighbor_has_edge;
    if(load_neighbor_gscore){
      parent->read(neighbor_gscore_addr, sizeof(uint64_t), neighbor_gscore_buffer);
      neighbor_gscore_loaded = true;
    }

    // compare g scores
    uint64_t tentative_gscore; //treated as const, simulation optimization
    bool shorter_path_found; //treated as const, simulation optimization
    const bool compare_gscores = neighbor_has_edge && curr_gscore_loaded() && neighbor_gscore_loaded();
    if(compare_gscores) {
      uint64_t curr_gscore;
      uint64_t neighbor_gscore;
      std::memcpy(&curr_gscore,curr_gscore_buffer.data(),sizeof(curr_gscore));
      std::memcpy(&neighbor_gscore,neighbor_gscore_buffer.data(),sizeof(neighbor_gscore));
      tentative_gscore = (curr_gscore & UINT32_MAX) + neighbor_distance;
      shorter_path_found = tentative_gscore < (neighbor_gscore & UINT32_MAX);
      parent->output->verbose(CALL_INFO,3,0,"curr_gscore=%u dist=%u neighbor_gscore=%u shorter_path_found=%u\n",curr_gscore,neighbor_distance,(neighbor_gscore & UINT32_MAX),shorter_path_found);
    }

    const bool do_update = shorter_path_found && (dma_state() == DMA_STATE::IDLE);
    if(do_update){
      parent->output->verbose(CALL_INFO,3,0,"found shorter path between curr=%u and neighbor=%u tentative_gscore=%llu\n",lowest_fscore_index(),neighbor(),tentative_gscore);
      //update came_from[neighbor]
      const uint64_t neighbor_came_from_addr = came_from_base_addr + (neighbor() * sizeof(uint64_t));
      const uint64_t curr = lowest_fscore_index();
      MemEventBase::dataVec curr_vec(sizeof(curr));
      std::memcpy(curr_vec.data(), &curr, sizeof(curr));
      dma_state = DMA_STATE::BUSY;
      parent->m_issueDRAMRequest(
        neighbor_came_from_addr,
        &curr_vec,
        DMA_WRITE,
        [this](const MemEventBase::dataVec & d){
          dma_state = DMA_STATE::IDLE;
      });

      //update {open_set,gscore,fscore}[neighbor]
      const uint64_t open_set_entry = tentative_gscore | INT64_MIN;
      MemEventBase::dataVec open_set_entry_vec(sizeof(open_set_entry));
      std::memcpy(open_set_entry_vec.data(),&open_set_entry,sizeof(open_set_entry));
      parent->write(neighbor_gscore_addr, sizeof(open_set_entry), &open_set_entry_vec);
    }

    // setup for next neighbor if this one does not has an edge or has been updated
    const bool neighbor_checked = (neighbor_edge_loaded && !neighbor_has_edge) || ((curr_gscore_loaded() && neighbor_gscore_loaded()) && (do_update || !shorter_path_found));
    if(neighbor_checked){
      parent->output->verbose(CALL_INFO,3,0,"do_next_neighbor\n");
      neighbor = neighbor() + 1;
      neighbor_gscore_loaded = false;
    }
    
    // pop next vertex in open set if all neighbors evaluated
    const bool all_neighbors_checked = (neighbor() == (vertices - 1)) && neighbor_checked;
    if(all_neighbors_checked) {
      loop_state = LOOP_STATE::POP_OPEN_SET;
      lowest_fscore = UINT32_MAX;
      lowest_fscore_index = UINT32_MAX;
      curr_fscore_loaded = false;
      open_set_index = 0;
    }
  }
  if(loop_state() == LOOP_STATE::CLEANUP) {
    parent->output->verbose(CALL_INFO,3,0,"AStar Cleanup\n");

    const bool wait = dma_state() == DMA_STATE::BUSY;
    if(!wait){
      loop_state = LOOP_STATE::DONE;
    }
  }
  if(loop_state() == LOOP_STATE::DONE) {
    parent->output->verbose(CALL_INFO,3,0,"AStar Done\n");

    // return 0 success, else 0
    if(path_found()){
      const uint64_t zero = 0;
      MemEventBase::dataVec zero_vec = MemEventBase::dataVec(8);
      std::memcpy(zero_vec.data(), &zero, sizeof(zero));
      parent->write(ret_code_addr,sizeof(uint64_t),&zero_vec);
    }else{
      const uint64_t one = 1;
      MemEventBase::dataVec one_vec = MemEventBase::dataVec(8);
      std::memcpy(one_vec.data(), &one, sizeof(one));
      parent->write(ret_code_addr,sizeof(uint64_t),&one_vec);
    }

    return true;
  }
  return false;
}

} // namespace
