//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
// See LICENSE in the top level directory for licensing details
//

#include "tclpim.h"
#include "tclpim_functions.h"
#include "userpim_functions.h"

namespace SST::PIM {

TCLPIM::TCLPIM( uint64_t node, SST::Output* o ) : PIM( o ) {
  // simulator defined identifier
  id          = ( uint64_t( PIM_TYPE_TEST ) << 56 ) | ( node << 12 );
  const uint8_t * p = (uint8_t*)&id;
  for(unsigned i = 0; i < sizeof(uint64_t); i++){
    spdArray[i] = p[i];
  } 

  output->verbose( CALL_INFO, 1, 0, "Creating TCLPIM node=%" PRId64 " id=0x%" PRIx64 "\n", node, id );
  // mmio decoder
  pimDecoder = new PIMDecoder( node );

  // PIM FSM Assignments
  // Built-in function 1: MemCopy
  funcState[FUNC_NUM::F1] = std::make_unique<FuncState>(this, FUNC_NUM::F1, std::make_unique<MemCopy>(this));
  // User function 5: MulVectByScalar
  funcState[FUNC_NUM::U5] = std::make_unique<FuncState>(this, FUNC_NUM::U5, std::make_unique<MulVecByScalar>(this));
  funcState[FUNC_NUM::U6] = std::make_unique<FuncState>(this, FUNC_NUM::U6, std::make_unique<LFSR>(this));
  funcState[FUNC_NUM::U7] = std::make_unique<FuncState>(this, FUNC_NUM::U7, std::make_unique<SymmetricDistanceMatrix>(this));
  funcState[FUNC_NUM::U8] = std::make_unique<FuncState>(this, FUNC_NUM::U8, std::make_unique<AStar>(this));

}

TCLPIM::~TCLPIM() {
  if( pimDecoder )
    delete pimDecoder;
}

bool TCLPIM::clock( SST::Cycle_t cycle ) {
  this->cycle = cycle;
  for (auto func : funcState ) {
    if (func.second->running()) {
      uint64_t done = func.second->exec()->clock();
      if (done) {
        func.second->writeFSM(FUNC_CMD::FINISH);
        return true;
      }
    }
  }
  return false;
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
    // 32 functions
    unsigned n = ( a >> 3 ) & SST::PIM::FUNC_LEN-1;
    assert(n < SST::PIM::FUNC_LEN );
    assert( (a & 0x7UL) == 0 ); // byte aligned
    assert( numBytes == 8 );
    return n;
}

void TCLPIM::read( Addr addr, uint64_t numBytes, std::vector<uint8_t>& payload ) {
  PIMDecodeInfo info = pimDecoder->decode( addr );
  if( info.pimAccType == PIM_ACCESS_TYPE::SRAM ) {
    unsigned sram_index = addr - PIMDecoder::getSramBaseAddr();
    assert((addr & 0x7) == 0); // 8 byte aligned only
    uint64_t data; // for debug only
    uint8_t * p = (uint8_t*) &data;
    for( unsigned i = 0; i < numBytes; i++ ) {
      payload[i] = spdArray[sram_index + i];
      p[i] = spdArray[sram_index + i];
    }
    output->verbose(
      CALL_INFO, 3, 0, "PIM 0x%" PRIx64 " IO READ SRAM A=0x%" PRIx64 " D=0x%" PRIx64 "\n", id, addr, data
    );
  } else {
    unsigned fnum = decodeFuncNum(addr, numBytes);
    output->verbose( CALL_INFO, 3, 0, "PIM 0x%" PRIx64 " IO READ FUNC[%d]\n", id, fnum );

    uint64_t d;
    if(const auto func = funcState.find(static_cast<FUNC_NUM>(fnum)); func != funcState.end()){
      d = func->second->readFSM();
    }else {
      output->verbose( CALL_INFO, 3, 0, "Warning: MMIO read from non-existent function handler fnum=%" PRIx32 "\n", fnum );
      d = static_cast<uint64_t>(FSTATE::INVALID);
    }

    uint8_t* p = (uint8_t*) ( &d );
    for( unsigned i = 0; i < numBytes; i++ ) {
      payload[i] = p[i];
    }
    output->verbose(
      CALL_INFO, 3, 0, "PIM 0x%" PRIx64 " IO READ FUNC A=0x%" PRIx64 " D=0x%" PRIx64 "\n", id, addr, d
    );
  }
}

void TCLPIM::write( Addr addr, uint64_t numBytes, const std::vector<uint8_t>* payload ) {
  
  output->verbose(CALL_INFO, 3, 0, "PIM 0x%" PRIx64 " IO WRITE A=0x%" PRIx64 "\n", id, addr);

  // TODO: Fix elf / linker to not load these
  if (numBytes !=8 ) {
    output->verbose(CALL_INFO, 3, 0, "Warning: Dropping MMIO write to function handler with numBytes=%" PRIx64 "\n", numBytes );
    return;
  }
  PIMDecodeInfo info = pimDecoder->decode( addr );
  if( info.pimAccType == PIM_ACCESS_TYPE::SRAM ) {
    // Eight 8-byte entries. Byte Addressable (memcpy -O0 does byte copy).
    unsigned sram_index = addr - PIMDecoder::getSramBaseAddr();
    assert((addr & 0x7) == 0); // 8 byte aligned only
    uint64_t data; // for debug only
    uint8_t * p = (uint8_t*) &data;
    for( unsigned i = 0; i < numBytes; i++ ) {
      spdArray[sram_index + i] = payload->at(i);
      p[i] = payload->at(i);
    }
    output->verbose(
      CALL_INFO, 3, 0, "PIM 0x%" PRIx64 " IO WRITE SRAM A=0x%" PRIx64 " D=0x%" PRIx64 "\n", id, addr, data
    );
  } else if( info.pimAccType == PIM_ACCESS_TYPE::FUNC ) {
    // Decode function number and grab the payload
    const unsigned fnum = decodeFuncNum(addr, numBytes);
    uint64_t data = 0;
    uint8_t* p = (uint8_t*) ( &data );
    for( unsigned i = 0; i < numBytes; i++ )
      p[i] = payload->at( i );

    output->verbose( CALL_INFO, 3, 0, "PIM 0x%" PRIx64 " IO WRITE FUNC[%d] D=0x%" PRIx64 "\n", id, fnum, data );
    if(const auto func = funcState.find(static_cast<FUNC_NUM>(fnum)); func != funcState.end()){
      func->second->writeFSM(data);
    }else {
      output->verbose( CALL_INFO, 3, 0, "Warning: Dropping MMIO write to non-existent function handler fnum=%" PRIx32 "\n", fnum );
    }
  } else {
    assert( false );
  }
}

TCLPIM::FuncState::FuncState(TCLPIM *p, FUNC_NUM fnum, std::shared_ptr<FSM> fsm) 
: parent(p), fnum(fnum), exec_(fsm)
{}

void TCLPIM::FuncState::writeFSM(uint64_t d)
{
  switch (fstate) {
    case FSTATE::DONE:
    case FSTATE::INVALID:
      if (static_cast<FUNC_CMD>(d) == FUNC_CMD::INIT) {
        parent->output->verbose(CALL_INFO, 3, 0, "Initializing Function[%u]\n",fnum);
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
          "Starting Function[%" PRId32 "]( 0x%" PRIx64 " 0x%" PRIx64 " 0x%" PRIx64 "0x%" PRIx64 " 0x%" PRIx64 " 0x%" PRIx64 " 0x%" PRIx64 " 0x%" PRIx64 " )\n",
          static_cast<int>(fnum), 
          params[0], params[1], params[2], params[3],
          params[4], params[5], params[6], params[7]
        );
        exec()->start(params);
      } else {
        assert(false);
      }
      break;
    case FSTATE::RUNNING:
        fstate = FSTATE::DONE;
        counter = 0;
        parent->output->verbose( CALL_INFO, 3, 0, "Function[%" PRId32 "] Done\n", static_cast<int>(fnum) );
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

std::shared_ptr<FSM> TCLPIM::FuncState::exec()
{
    assert(exec_);
    return exec_;
}

void TCLPIM::FuncState::setFSM(std::shared_ptr<FSM> fsm)
{
  exec_ = fsm;
}

} // namespace