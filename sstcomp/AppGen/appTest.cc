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

using namespace SST;
using namespace SST::AppGen;

AppTest::AppTest( AppLink* _appLink, uint64_t pimSramBaseAddr ) : App( _appLink ), pimSramBaseAddr(pimSramBaseAddr) {};

void AppTest::theApp() {
  int      rc      = 0;
  uint64_t readBuf = 0;
  // read PIM_ID
  send( SRAM_CMD::READ, pimSramBaseAddr, 0 );
  rc = receive( readBuf );
  if( rc )
    out->fatal( CALL_INFO, -1, "receiving PIM ID read failed\n" );
  out->verbose( CALL_INFO, 1, 0, "PIM ID read as 0x%" PRIx64 "\n", readBuf );

  // write/read some other memory
  for( uint64_t addr = 0x20000; addr < 0x20080; addr += 8 )
    send( SRAM_CMD::WRITE, addr, ( addr << 16 ) + 0xaced );
  for( uint64_t addr = 0x20000; addr < 0x20080; addr += 8 )
    send( SRAM_CMD::READ, addr, 0 );
}
