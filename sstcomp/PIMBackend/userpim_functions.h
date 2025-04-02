//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
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
  MulVecByScalar(TCLPIM *p);
  void start(uint64_t params[NUM_FUNC_PARAMS]) override;
  bool clock() override;

private:
  enum DMA_STATE { IDLE, READ, WRITE, WAITING, DONE };
  DMA_STATE dma_state = DMA_STATE::IDLE;
  unsigned total_words = 0;
  unsigned word_counter = 0;
  uint64_t src = 0;
  uint64_t dst = 0;
  uint64_t scalar = 0;
}; // class MulVecByScalar

class DotProduct : public FSM {
public:
  DotProduct(TCLPIM *p);
  void start(uint64_t params[NUM_FUNC_PARAMS]) override;
  bool clock() override;

private:
  enum DMA_STATE { IDLE, READ1, READ2, WRITE, WAITING, DONE };
  DMA_STATE dma_state = DMA_STATE::IDLE;
  unsigned total_words = 0;
  unsigned word_counter = 0;
  uint64_t sum = 0;
  uint64_t src1 = 0;
  uint64_t src2 = 0;
  uint64_t dst = 0;
  // below used for task time statistic
  uint64_t task_start;
  // uint64_t task_end;
}; // class DotProduct

} // namespace SST::PIM

#endif //_SST_PIMBACKEND_USER_PIM_FUNCTIONS_
