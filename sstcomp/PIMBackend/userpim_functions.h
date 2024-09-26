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
  enum class LOOP_STATE { IDLE, INIT, CYCLING, DONE };
  enum class DMA_STATE { IDLE, WRITE, WAITING };
  DMA_STATE  dma_state    = DMA_STATE::IDLE;
  LOOP_STATE loop_state   = LOOP_STATE::IDLE;
  uint64_t   dst          = 0;
  uint64_t   seed         = 0;
  uint64_t   len          = 0;
  uint64_t   lfsr;
  uint64_t   samples;
  uint64_t   cycles;
  uint64_t   buffer_head;
};  //class LFSR

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

  T operator()(){ return q; }

  void clock(){
    q = d;
    driven = false;
  }
};

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

} // namespace SST::PIM


#endif //_SST_PIMBACKEND_USER_PIM_FUNCTIONS_