/*
 * Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 */

#ifndef _H_SST_MIRANDA_APP_TEST_
#define _H_SST_MIRANDA_APP_TEST_

#include "app.h"
#include "appTransactor.h"

namespace SST::AppGen {

class AppTest : public App {
public:
  AppTest( AppLink* _link );
  virtual void theApp() override;
};  //AppTest

class AppxTest : public AppTransactor {

public:
  AppxTest( ComponentId_t id, Params& params ) : AppTransactor( id, params ) {}

  SST_ELI_REGISTER_SUBCOMPONENT(
    AppxTest,
    "AppGen",
    "AppxTest",
    SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
    "Test Miranda AppTransactor Generator",
    SST::AppGen::AppTransactor
  )

  SST_ELI_DOCUMENT_PARAMS( { "verbose", "Sets the verbosity of the output", "0" } )

  void spawnApp( AppLink* _appLink ) override {
    assert( appLink == nullptr );
    appLink = _appLink;
    app     = new AppTest( appLink );
    app->setOutput( out );
    int rc = app->spawn();
    if( rc ) {
      out->fatal( CALL_INFO, -1, "Could not spawn thread\n" );
    } else {
      out->verbose( CALL_INFO, 1, 0, "Successfully spawned application thread\n" );
    }
  }

};  //AppxTest

}  // namespace SST::AppGen

#endif  //_H_SST_MIRANDA_APP_TEST_
