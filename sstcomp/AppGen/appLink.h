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

namespace SST {
namespace AppGen {

enum class AppCmd { NOP, READ, WRITE, DONE };

struct AppEvent {
  AppCmd   cmd;
  uint64_t address;
  uint64_t data;

  AppEvent( AppCmd _cmd, uint64_t _address, uint64_t _data ) : cmd( _cmd ), address( _address ), data( _data ) {}

  friend std::ostream& operator<<( std::ostream& os, const AppEvent& e ) {
    std::string c;
    c = e.cmd == AppCmd::NOP   ? "NOP" :
        e.cmd == AppCmd::READ  ? "READ" :
        e.cmd == AppCmd::WRITE ? "WRITE" :
        e.cmd == AppCmd::DONE  ? "DONE" :
                                 "?";
    os << c << " 0x" << std::hex << e.address;
    if( e.cmd == AppCmd::WRITE )
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
}  // namespace AppGen
}  // namespace SST
#endif  //_SST_APPGEN_APP_LINK_H
