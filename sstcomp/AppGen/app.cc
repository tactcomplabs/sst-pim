/*
 * Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 */

#include <assert.h>
#include <iostream>
#include <sstream>

#include "app.h"

using namespace SST::AppGen;

App::App( AppLink* _appLink ) : appLink( _appLink ){};

App::~App() {}

void SST::AppGen::App::setOutput( Output* _out ) {
  assert( _out );
  out = _out;
};

void App::worker_thread() {
  theApp();
  // Finished
  send( SRAM_CMD::DONE, 0, 0 );
}

void SST::AppGen::App::send( SRAM_CMD cmd, uint64_t address, uint64_t data ) {
  std::unique_lock lk( appLink->m );
  appLink->cv.wait( lk, [this] { return appLink->q.empty(); } );
  assert( appLink->q.size() == 0 );
  AppEvent e( cmd, address, data );
  appLink->q.push( e );
  std::stringstream s;
  s << e;
  if( e.cmd != SRAM_CMD::NOP ) {
    out->verbose( CALL_INFO, 3, 0, "App thread sending %s\n", s.str().c_str() );
  }
  lk.unlock();
  //std::this_thread::sleep_for(std::chrono::milliseconds(100));
  appLink->cv.notify_one();
}

int SST::AppGen::App::receive( uint64_t& data ) {
  // Currently requires loads to return in order.
  // TODO throttle generate to not be called when there are outstanding loads (use dependencies?)
  int timeout = 0x100000;
  do {
    send( SRAM_CMD::NOP, timeout, 0 );
  } while( --timeout > 0 && appLink->loadQ.size() == 0 );

  if( appLink->loadQ.size() == 0 )
    return -1;
  if( appLink->loadQ.size() > 1 )
    return appLink->loadQ.size();
  data = appLink->loadQ.front();
  appLink->loadQ.pop();
  return 0;
}

int App::spawn() {
  assert( appLink->thr == nullptr );
  assert( !_running );
  try {
    appLink->thr = new std::thread( [this] { this->worker_thread(); } );
    _running     = true;
    return 0;
  } catch( const std::system_error& e ) {
    std::cerr << "Exception: " << e.what() << std::endl;
    assert( false );
  }
  return 1;
}
