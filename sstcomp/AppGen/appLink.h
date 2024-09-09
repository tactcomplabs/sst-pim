/*
 * Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 */

#ifndef _SST_APPGEN_APP_LINK_H
#define _SST_APPGEN_APP_LINK_H

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include "pimdef.h"

using namespace SST::PIM;

namespace SST::AppGen {

struct AppEvent {
  SRAM_CMD   cmd;
  uint64_t address;
  uint64_t data;

  AppEvent( SRAM_CMD _cmd, uint64_t _address, uint64_t _data ) : cmd( _cmd ), address( _address ), data( _data ) {}

  friend std::ostream& operator<<( std::ostream& os, const AppEvent& e ) {
    std::string c;
    c = e.cmd == SRAM_CMD::NOP   ? "NOP" :
        e.cmd == SRAM_CMD::READ  ? "READ" :
        e.cmd == SRAM_CMD::WRITE ? "WRITE" :
        e.cmd == SRAM_CMD::DONE  ? "DONE" :
                                 "?";
    os << c << " 0x" << std::hex << e.address;
    if( e.cmd == SRAM_CMD::WRITE )
      os << " 0x" << std::hex << e.data;
    return os;
  }
};

class AppLink {
public:
  AppLink(){};
  ~AppLink(){};
  std::mutex               m;
  std::condition_variable  cv;
  std::thread*             thr = nullptr;
  std::queue<AppEvent>     q;
  std::queue<uint64_t>     loadQ;
  bool                     done = false;
  std::vector<std::string> args;
};
}  // namespace AppGen::SST
#endif  //_SST_APPGEN_APP_LINK_H
