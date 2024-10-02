//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
// See LICENSE in the top level directory for licensing details
//

#include "pim.h"

namespace SST::PIM {

PIMReq_t::PIMReq_t( MemBackend::ReqId _id, Addr _addr, bool _isWrite, unsigned _numBytes, MemEventBase::dataVec _payload )
  : id( _id ), addr( _addr ), isWrite( _isWrite ), numBytes( _numBytes ), payload( _payload ) {};

PIMReq_t::PIMReq_t( MemBackend::ReqId _id, Addr _addr, bool _isWrite, unsigned _numBytes, uint64_t data )
  : id( _id ), addr( _addr ), isWrite( _isWrite ), numBytes( _numBytes ) {
  uint8_t* p = (uint8_t*) ( &data );
  setPayload( p, numBytes );
};

void PIMReq_t::setPayload( uint8_t* data, unsigned bytes ) {
  if( bytes != payload.size() )
    payload.resize( bytes );
  for( unsigned int i = 0; i < bytes; i++ )
    payload.at( i ) = data[i];
};

uint64_t PIMReq_t::getPayload(std::vector<uint8_t> payload)
{
    uint64_t data = 0;
    uint8_t* p = (uint8_t*) ( &data );
    for( unsigned i = 0; i < sizeof(uint64_t); i++ )
      p[i] = payload[i];
    return data;
}

} // namespace SST::PIM

// EOF
