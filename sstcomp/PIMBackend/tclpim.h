//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_PIMBACKEND_TCLPIM_
#define _SST_PIMBACKEND_TCLPIM_

#include "pim.h"

namespace SST::PIM {

class TCLPIM : public PIM {
public:
  TCLPIM( uint64_t node, SST::Output* o );
  virtual ~TCLPIM();
  void     setup() override {};
  bool     clock( SST::Cycle_t ) override;
  uint64_t getCycle() override;
  bool     isMMIO( uint64_t addr ) override;
  // IO access functions
  void read( Addr, uint64_t numBytes, std::vector<uint8_t>& ) override;
  void write( Addr, uint64_t numBytes, std::vector<uint8_t>* ) override;

private:
  uint64_t   id;
  PIMDecoder* pimDecoder;
  uint64_t   cycle = 0;

  // memory mapped IO
  std::vector<std::shared_ptr<PIMMemSegment>> PIMSegs;
  uint64_t                                   spdArray[8] = { 0 };
  uint64_t                                   ctl_buf     = 0;
  uint64_t                                   ctl_event   = 0;
  std::deque<uint64_t>                       ctl_ops;
  bool                                       ctl_exec   = false;
  bool                                       ctl_locked = false;
  void                                       function_write( unsigned offset );

  // dma sequencer
  class SimpleDMA {
  public:
    SimpleDMA( TCLPIM* p );
    virtual ~SimpleDMA() {};
    void     start( uint64_t src, uint64_t dst, uint64_t numWords );
    unsigned clock();  // return 1 when busy, 0 when idle

    inline bool active() { return ( dma_state != DMA_STATE::IDLE ); };

  private:
    TCLPIM* parent;

    enum DMA_STATE { IDLE, READ, WRITE, WAITING, DONE };

    DMA_STATE dma_state    = DMA_STATE::IDLE;
    uint64_t  total_words  = 0;
    uint64_t  word_counter = 0;
    uint64_t  src          = 0;
    uint64_t  dst          = 0;
  };  //class SimpleDMA

  SimpleDMA* dma;

};  //class TCLPIM


} // namespace SST::PIM


#endif