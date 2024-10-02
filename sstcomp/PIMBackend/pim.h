//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
// See LICENSE in the top level directory for licensing details
//

#ifndef _H_SST_MEMH_PIM_
#define _H_SST_MEMH_PIM_

#include <queue>
#include <vector>

#include "PIMBackend.h"
#include "PIMDecoder.h"
#include "pimdef.h"
#include "kgdbg.h"

using namespace SST::MemHierarchy;

namespace SST::PIM {

// TODO static uint64_t getPayload( std::vector<uint8_t> payload );

struct PIMReq_t {
  PIMReq_t( MemBackend::ReqId _id, Addr _addr, bool _isWrite, unsigned _numBytes, MemEventBase::dataVec _payload );
  PIMReq_t( MemBackend::ReqId _id, Addr _addr, bool _isWrite, unsigned _numBytes, uint64_t data = 0 );
  MemBackend::ReqId     id;
  Addr                  addr;
  bool                  isWrite;
  unsigned              numBytes;
  MemEventBase::dataVec payload;
  void                  setPayload( uint8_t* data, unsigned bytes );
  uint64_t              getPayload(std::vector<uint8_t> payload);

  friend std::ostream& operator<<( std::ostream& os, const PIMReq_t r ) {
    return os << hex << "id=0x" << r.id << " addr=0x" << r.addr << " isWrite=" << r.isWrite << " numBytes=" << r.numBytes << endl;
  }
};

class PIM {
public:
  PIM( SST::Output* o ) : output( o ) {
    kgdbg::spinner("PIM_SPINNER");
  };

  virtual ~PIM() {}

  virtual void     setup()                                             = 0;
  virtual bool     clock( SST::Cycle_t )                               = 0;
  virtual uint64_t getCycle()                                          = 0;
  // IO access functions
  virtual bool isMMIO( uint64_t addr )                                 = 0;
  virtual void read( Addr, uint64_t numBytes, std::vector<uint8_t>& )  = 0;
  virtual void write( Addr, uint64_t numBytes, std::vector<uint8_t>* ) = 0;
  // DRAM request callback (uint64_t a, MemEventBase::dataVec* d, unsigned bytes, bool isWrite, std::function<void(const uint64_t&)> completion)
  std::function<void( uint64_t, MemEventBase::dataVec*, bool, std::function<void( const MemEventBase::dataVec& )> )>
    m_issueDRAMRequest;

  void setCallback(
    std::function<void( uint64_t, MemEventBase::dataVec*, bool, std::function<void( const MemEventBase::dataVec& )> )> handler
  ) {
    m_issueDRAMRequest = handler;
  }

  //TODO protect and friend FSM
  SST::Output*          output;
  MemEventBase::dataVec buffer;

};  // class PIMifc

}  // namespace SST::PIM

#endif  // _H_SST_MEMH_PIM_
