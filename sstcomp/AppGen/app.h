/*
 * Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 */

#ifndef _H_SST_MIRANDA_APP_
#define _H_SST_MIRANDA_APP_

// clang-format off
#include "sst_app.h"
#include "appLink.h"
// clang-format on

namespace SST::AppGen {

class App {
public:
  App(AppLink *_link);
  virtual ~App();
  void setOutput(Output *out);
  int spawn();
  virtual void theApp() = 0;

  // Syncrhonized access functions invoked by the derived application.
  // Making these public to allow runtime to access them using App* pointer.
  void send(PIM::SRAM_CMD cmd, uint64_t address, uint64_t data);
  size_t receive(uint64_t &data);

private:
  void worker_thread();
  std::string name = "";

protected:
  bool _running = false;
  AppLink *appLink = nullptr;
  Output *out;

  void setName(const std::string &_name) { name = _name; }

  const std::string &getName() { return name; }
};

} // namespace SST::AppGen

#endif //_H_SST_MIRANDA_APP_
