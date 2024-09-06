/*
 * Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 */

#ifndef _H_SST_APP_TRANSACTOR_
#define _H_SST_APP_TRANSACTOR_

// clang-format off
#include "mirandaGenerator_kg.h"
#include "sst/core/output.h"
#include "appLink.h"
#include "app.h"

// clang-format on

namespace SST::AppGen {

class AppTransactor : public RequestGenerator {

public:
  AppTransactor( ComponentId_t id, Params& params );
  ~AppTransactor();

  virtual void spawnApp( AppLink* _appLink ){};

  void generate( MirandaRequestQueue<GeneratorRequest*>* q );
  bool isFinished();
  void completed();
  void handleLoadResponse( uint64_t data );

  SST_ELI_REGISTER_SUBCOMPONENT(
    AppTransactor,
    "AppGen",
    "AppTransactor",
    SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
    "Miranda AppTransactor Generator",
    SST::AppGen::RequestGenerator
  )

  SST_ELI_DOCUMENT_PARAMS( { "verbose", "Sets the verbosity of the output", "0" } )

protected:
  Output*                  out     = nullptr;
  AppLink*                 appLink = nullptr;
  App*                     app     = nullptr;
  std::vector<std::string> args;
};

}  // namespace SST::AppGen

#endif  //_H_SST_APP_TRANSACTOR_
