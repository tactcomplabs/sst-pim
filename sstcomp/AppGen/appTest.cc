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

AppTest::AppTest( AppLink* _appLink ) : App( _appLink ){};

void AppTest::theApp() {
  int      rc      = 0;
  uint64_t readBuf = 0;
  // read PIM_ID
  uint64_t pimBase =  static_cast<uint64_t>(PIM::SRAM_BASE);
  send( SRAM_CMD::READ, pimBase, 0 );
  rc = receive( readBuf );
  if( rc )
    out->fatal( CALL_INFO, -1, "receiving PIM ID read failed\n" );
  out->verbose( CALL_INFO, 1, 0, "PIM ID read as 0x%" PRIx64 "\n", readBuf );

  uint64_t expected = ((uint64_t)PIM_TYPE_TCL << 56);
  if (readBuf != expected ) {
    out->fatal(CALL_INFO, -1, "Expected PIM ID=0x%" PRIx64 "\n", expected);
  }

  // write/read some other memory
  for( uint64_t addr = 0x20000; addr < 0x20080; addr += 8 )
    send( SRAM_CMD::WRITE, addr, ( addr << 16 ) + 0xaced );
  for( uint64_t addr = 0x20000; addr < 0x20080; addr += 8 ) {
    send( SRAM_CMD::READ, addr, 0 );
    rc = receive( readBuf );
    expected = (addr << 16) + 0xaced;
    if (readBuf !=  expected ) {
      out->fatal(CALL_INFO, -1, 
        "For address 0x%" PRIx64 ": Expected 0x%" PRIx64 "  but acquired 0x%" PRIx64 "\n", 
        addr, expected, readBuf);
    }
  }
}
