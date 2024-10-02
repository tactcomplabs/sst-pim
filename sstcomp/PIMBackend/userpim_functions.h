//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_PIMBACKEND_USER_PIM_FUNCTIONS_
#define _SST_PIMBACKEND_USER_PIM_FUNCTIONS_

#include "tclpim.h"

namespace SST::PIM {

template<typename T>
struct SequentialLogic {
  T d;
  T q;
  bool driven = false;

  SequentialLogic(T init) : q(init) {}
  SequentialLogic(){}

  void operator=(T in){
    assert(!driven);
    d = in;
    driven = true;
  }

  inline T operator()(){ return q; }

  void clock(){
    q = d;
    driven = false;
  }
};


class MulVecByScalar : public FSM {
public:
  MulVecByScalar( TCLPIM* p );
  void start( uint64_t params[NUM_FUNC_PARAMS] ) override;
  bool clock() override;
private:
  enum DMA_STATE { IDLE, READ, WRITE, WAITING, DONE };
  DMA_STATE dma_state    = DMA_STATE::IDLE;
  uint64_t  total_words  = 0;
  uint64_t  word_counter = 0;
  uint64_t  src          = 0;
  uint64_t  dst          = 0;
  uint64_t  scalar       = 0;
};  //class MulVecByScalar

class LFSR : public FSM {
public:
  LFSR( TCLPIM* p );
  void start( uint64_t params[NUM_FUNC_PARAMS] ) override;
  bool clock() override;
private:
  void flipFlop();
  enum class LOOP_STATE { IDLE, CYCLING, DONE };
  LOOP_STATE loop_state   = LOOP_STATE::IDLE;
  uint64_t   rand_base_addr = 0;
  uint64_t   seed           = 0;
  uint64_t   len            = 0;
  SequentialLogic<uint64_t> lfsr;
  SequentialLogic<unsigned> samples;
  SequentialLogic<unsigned> cycles;
};  //class LFSR

class SymmetricDistanceMatrix : public FSM {
public:
  SymmetricDistanceMatrix( TCLPIM* p );
  void start( uint64_t params[NUM_FUNC_PARAMS] ) override;
  bool clock() override;
private:
  enum class LOOP_STATE { IDLE, INIT_ROW, CYCLE, CLEANUP, DONE };
  enum class DMA_STATE { IDLE, WRITE, WAIT };
  SequentialLogic<DMA_STATE>  dma_state    = DMA_STATE::IDLE;
  SequentialLogic<LOOP_STATE> loop_state   = LOOP_STATE::IDLE;
  uint64_t   matrix_base_addr;
  uint64_t   rand_base_addr;
  uint64_t   dist_mask;
  uint64_t   vertices;
  uint32_t   prob_mask;
  SequentialLogic<uint64_t> curr_row;
  SequentialLogic<uint64_t> curr_col;
  SequentialLogic<bool> row_loaded;
  SequentialLogic<bool> col_loaded;
  SequentialLogic<uint64_t> buffer_head;
  SequentialLogic<uint64_t> matrix_head;
  MemEventBase::dataVec curr_row_buffer = {0,0,0,0,0,0,0,0};
  MemEventBase::dataVec curr_col_buffer = {0,0,0,0,0,0,0,0};
};  //class SymmetricDistanceMatrix

class AStar : public FSM {
public:
  AStar( TCLPIM* p);
  void start (uint64_t params[NUM_FUNC_PARAMS] ) override;
  bool clock() override;
private:
  enum class LOOP_STATE { IDLE, INIT, POP_OPEN_SET, CYCLE, CLEANUP, DONE };
  enum class DMA_STATE { IDLE, BUSY };
  const bool DMA_WRITE = true;
  const bool DMA_READ = false;
  SequentialLogic<DMA_STATE>  dma_state;
  SequentialLogic<LOOP_STATE> loop_state;
  uint64_t came_from_base_addr;
  uint64_t matrix_base_addr;
  uint64_t ret_code_addr;
  uint64_t src;
  uint64_t target;
  unsigned vertices;
  SequentialLogic<unsigned> open_set_index;
  SequentialLogic<unsigned> lowest_fscore;
  SequentialLogic<unsigned> lowest_fscore_index;
  SequentialLogic<unsigned> neighbor;
  SequentialLogic<unsigned> buffer_head;
  SequentialLogic<unsigned> buffer_tail;
  SequentialLogic<bool> curr_fscore_loaded;
  SequentialLogic<bool> curr_gscore_loaded;
  SequentialLogic<bool> neighbor_gscore_loaded;
  SequentialLogic<bool> path_found;
  MemEventBase::dataVec uint32_max_vec = MemEventBase::dataVec(8);
  MemEventBase::dataVec zero_vec = MemEventBase::dataVec(8,0);
  MemEventBase::dataVec curr_fscore_buffer = MemEventBase::dataVec(8);
  MemEventBase::dataVec curr_gscore_buffer = MemEventBase::dataVec(8);
  MemEventBase::dataVec neighbor_gscore_buffer = MemEventBase::dataVec(8);
  uint64_t cycle = 0;
};

} // namespace SST::PIM


#endif //_SST_PIMBACKEND_USER_PIM_FUNCTIONS_