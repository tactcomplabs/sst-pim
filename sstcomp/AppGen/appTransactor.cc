/*
 * Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 * See LICENSE in the top level directory for licensing details
 */

// clang-format off
// order dependent includes
#include <sst/core/sst_config.h>
#include <sst/core/params.h>
#include "appTransactor.h"
// clang-format on

using namespace SST::AppGen;

AppTransactor::AppTransactor( ComponentId_t id, Params& params ) : RequestGenerator( id, params ) {
  // output stream
  const uint32_t verbose = params.find<uint32_t>( "verbose", 0 );
  out                    = new Output( "AppTransactor[@p:@l]: ", verbose, 0, Output::STDOUT );
}

AppTransactor::~AppTransactor() {
  if( out )
    delete out;
  if( app )
    delete app;
}

void AppTransactor::generate( MirandaRequestQueue<GeneratorRequest*>* q ) {
  assert( appLink );

  // OK q available. Step the application to next data access.
  std::unique_lock lk( appLink->m );
  appLink->cv.wait( lk, [this] { return !appLink->q.empty(); } );

  assert( appLink->q.size() == 1 );
  AppEvent e = appLink->q.front();
  appLink->q.pop();
  if( e.cmd != SRAM_CMD::NOP ) {  // TODO there should be no NOPs
    std::stringstream s;
    s << e;
    out->verbose( CALL_INFO, 10, 0, "Processing %s\n", s.str().c_str() );
  }
  switch( e.cmd ) {
  case SRAM_CMD::DONE: appLink->done = true; break;
  case SRAM_CMD::READ: q->push_back( new MemoryOpRequest( e.address, 8, ReqOperation::READ ) ); break;
  case SRAM_CMD::WRITE: q->push_back( new MemoryOpRequest( e.address, 8, ReqOperation::WRITE, e.data ) ); break;
  case SRAM_CMD::NOP: break;
  default: out->fatal( CALL_INFO, -1, "Undefined Cmd %d", static_cast<int>( e.cmd ) ); break;
  }

  //std::this_thread::sleep_for(std::chrono::milliseconds(50));
  lk.unlock();
  appLink->cv.notify_one();
}

bool AppTransactor::isFinished() {
  return appLink->done;
}

void AppTransactor::completed() {}

void SST::AppGen::AppTransactor::handleLoadResponse( uint64_t data ) {
  out->verbose( CALL_INFO, 10, 0, "Receiving load response 0x%" PRIx64 "\n", data );
  appLink->loadQ.push( data );
}
