/*
 * Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 */

#ifndef _H_SST_MIRANDA_APP_
#define _H_SST_MIRANDA_APP_

#include "appLink.h"
#include <sst/core/output.h>

namespace SST::AppGen {

class App {
public:
  App( AppLink* _link );
  virtual ~App();
  void         setOutput( Output* out );
  int          spawn();
  virtual void theApp(){};

  // Syncrhonized access functions invoked by the derived application.
  // Making these public to allow runtime to access them using App* pointer.
  void send( PIM::Cmd cmd, uint64_t address, uint64_t data );
  int  receive( uint64_t& data );

private:
  void        worker_thread();
  std::string name = "";

protected:
  bool     _running = false;
  AppLink* appLink  = nullptr;
  Output*  out;

  void setName( const std::string& _name ) { name = _name; }

  const std::string& getName() { return name; }
};

}  // namespace SST::AppGen

#endif  //_H_SST_MIRANDA_APP_
