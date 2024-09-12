//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
// See LICENSE in the top level directory for licensing details
//

#include "tclpim.h"
#include "tclpim_functions.h"

namespace SST::PIM {

TCLPIM::TCLPIM( uint64_t node, SST::Output* o ) : PIM( o ) {
  // simulator defined identifier
  id          = ( uint64_t( PIM_TYPE_TEST ) << 56 ) | ( node << 12 );
  spdArray[0] = id;
  output->verbose( CALL_INFO, 1, 0, "Creating TCLPIM node=%" PRId64 " id=0x%" PRIx64 "\n", node, id );
  // mmio decoder
  pimDecoder = new PIMDecoder( node );
  // Built-in functions
  funcState.resize(NUM_FUNCS);
  for (unsigned i=0; i<NUM_FUNCS; i++ )
    funcState[i] = std::make_unique<FuncState>(this, i);
}

TCLPIM::~TCLPIM() {
  if( pimDecoder )
    delete pimDecoder;
}

bool TCLPIM::clock( SST::Cycle_t cycle ) {
  this->cycle = cycle;

  // if( dma->active() ) {
  //   uint64_t done = dma->clock();
  //   spdArray[0]   = done;
  // }
  
  for ( int fnum=0; fnum<16; fnum++) {
    if (funcState[fnum]->running()) {
      uint64_t done = funcState[fnum]->exec->clock();
      if (done)
        funcState[fnum]->writeFSM(FUNC_CMD::FINISH);
    }
  }

}

uint64_t TCLPIM::getCycle() {
  return cycle;
}

bool TCLPIM::isMMIO( uint64_t addr ) {
  PIMDecodeInfo inf = pimDecoder->decode( addr );
  return inf.isIO;
}

uint64_t TCLPIM::decodeFuncNum(uint64_t a, unsigned numBytes)
{
    // 16 functions
    unsigned n = ( a >> 3 ) & 0xf;
    assert(n < SST::PIM::FUNC_SIZE );
    assert( (a & 0x7UL) == 0 ); // byte aligned
    assert( numBytes == 8 );
    return n;
}

void TCLPIM::read( Addr addr, uint64_t numBytes, std::vector<uint8_t>& payload ) {
  PIMDecodeInfo info = pimDecoder->decode( addr );
  if( info.pimAccType == PIM_ACCESS_TYPE::SRAM ) {
    unsigned spdIndex = ( addr & 0x38ULL ) >> 3;
    unsigned byte     = ( addr & 0x7 );
    assert( ( byte + numBytes ) <= 8 );  // 8 byte aligned only
    uint8_t* p = (uint8_t*) ( &( spdArray[spdIndex] ) );
    for( unsigned i = 0; i < numBytes; i++ ) {
      payload[i] = p[byte + i];
    }
    output->verbose(
      CALL_INFO, 3, 0, "PIM 0x%" PRIx64 " IO READ SRAM A=0x%" PRIx64 " D=0x%" PRIx64 "\n", id, addr, spdArray[spdIndex]
    );
  } else {
    unsigned fnum = decodeFuncNum(addr, numBytes);
    output->verbose( CALL_INFO, 3, 0, "PIM 0x%" PRIx64 " IO READ FUNC[%d]\n", id, fnum );
    uint64_t d = funcState[fnum]->readFSM();
    uint8_t* p = (uint8_t*) ( &d );
    for( unsigned i = 0; i < numBytes; i++ ) {
      payload[i] = p[i];
    }
    output->verbose(
      CALL_INFO, 3, 0, "PIM 0x%" PRIx64 " IO READ FUNC A=0x%" PRIx64 " D=0x%" PRIx64 "\n", id, addr, d
    );
  }
}

void TCLPIM::write( Addr addr, uint64_t numBytes, std::vector<uint8_t>* payload ) {
  
  output->verbose(CALL_INFO, 3, 0, "PIM 0x%" PRIx64 " IO WRITE A=0x%" PRIx64 "\n", id, addr);

  // TODO: Fix elf / linker to not load these
  if (numBytes !=8 ) {
    output->verbose(CALL_INFO, 3, 0, "Warning: Dropping MMIO write to function handler with numBytes=%" PRIx64 "\n", numBytes );
    return;
  }
  PIMDecodeInfo info = pimDecoder->decode( addr );
  if( info.pimAccType == PIM_ACCESS_TYPE::SRAM ) {
    // Eight 8-byte entries. Byte Addressable (memcpy -O0 does byte copy).
    unsigned offset = ( addr & 0x38ULL ) >> 3;
    unsigned byte   = ( addr & 0x7 );
    assert( ( byte + numBytes ) <= 8 );  // 8 byte aligned only
    uint8_t* p = (uint8_t*) ( &( spdArray[offset] ) );
    for( int i = 0; i < numBytes; i++ ) {
      p[byte + i] = payload->at( i );
    }
    output->verbose(
      CALL_INFO, 3, 0, "PIM 0x%" PRIx64 " IO WRITE SRAM A=0x%" PRIx64 " D=0x%" PRIx64 "\n", id, addr, spdArray[offset]
    );
  } else if( info.pimAccType == PIM_ACCESS_TYPE::FUNC ) {
    // Decode function number and grab the payload
    unsigned fnum = decodeFuncNum(addr, numBytes);
    uint64_t data = 0;
    uint8_t* p = (uint8_t*) ( &data );
    for( unsigned i = 0; i < numBytes; i++ )
      p[i] = payload->at( i );

    output->verbose( CALL_INFO, 3, 0, "PIM 0x%" PRIx64 " IO WRITE FUNC[%d] D=0x%" PRIx64 "\n", id, fnum, data );
    funcState[fnum]->writeFSM(data);
  } else {
    assert( false );
  }
}

TCLPIM::FuncState::FuncState(TCLPIM *p, unsigned n) : parent(p), fnum(n)
{
  exec = make_unique<MemCopy>(p);
}

void TCLPIM::FuncState::writeFSM(uint64_t d)
{
  switch (fstate) {
    case FSTATE::DONE:
    case FSTATE::INVALID:
      if (static_cast<FUNC_CMD>(d) == FUNC_CMD::INIT) {
        fstate = FSTATE::INITIALIZING;
        counter = 0;
      } else  { 
        assert(false);
      }
      break;
    case FSTATE::INITIALIZING:
      params[counter++] = d;
      if (counter == NUM_FUNC_PARAMS)
        fstate = FSTATE::READY;
      break;
    case FSTATE::READY:
      if (static_cast<FUNC_CMD>(d) == FUNC_CMD::RUN) {
        fstate = FSTATE::RUNNING;
        counter = 0;
        parent->output->verbose(
          CALL_INFO, 3, 0, 
          "Starting Function[%d]( 0x%" PRIx64 " 0x%" PRIx64 " 0x%" PRIx64 " )\n",
          fnum, params[0], params[1], params[2]);
        exec->start(params[0], params[1], params[2]);
      } else {
        assert(false);
      }
      break;
    case FSTATE::RUNNING:
        fstate = FSTATE::DONE;
        counter = 0;
        parent->output->verbose( CALL_INFO, 3, 0, "Function[%d] Done\n", fnum );
      break;
  }
}

void TCLPIM::FuncState::writeFSM(FUNC_CMD cmd) {
  writeFSM( static_cast<uint64_t>(cmd) );
}


uint64_t TCLPIM::FuncState::readFSM()
{
    return static_cast<uint64_t>(fstate);
}

bool TCLPIM::FuncState::running()
{
    return fstate==FSTATE::RUNNING;
}


} // namespace