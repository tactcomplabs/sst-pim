/*
 * Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 */

#include <assert.h>
#include <iostream>

#include "appTest.h"
#include "pimdef.h"

using namespace SST::AppGen;

AppTest::AppTest( AppLink* _appLink ) : App( _appLink ){};

void AppTest::theApp() {
  int      rc      = 0;
  uint64_t readBuf = 0;
  // read PIM_ID
  uint64_t pimBase = PIM_SRAM;
  send( AppCmd::READ, pimBase, 0 );
  rc = receive( readBuf );
  if( rc )
    out->fatal( CALL_INFO, -1, "receiving PIM ID read failed\n" );
  out->verbose( CALL_INFO, 1, 0, "PIM ID read as 0x%" PRIx64 "\n", readBuf );

  // write/read some other memory
  for( uint64_t addr = 0x20000; addr < 0x20080; addr += 8 )
    send( AppCmd::WRITE, addr, ( addr << 16 ) + 0xaced );
  for( uint64_t addr = 0x20000; addr < 0x20080; addr += 8 )
    send( AppCmd::READ, addr, 0 );
}
