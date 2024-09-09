/*
 * apptest.cpp
 *
 * Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 */

// Standard includes
#include <cinttypes>
#include <cstdlib>

// PIM definitions
#include "pimdef.h"

// REV includes
#include "rev-macros.h"
#include "syscalls.h"

#undef assert
#define assert TRACE_ASSERT

// rev_fast_print is limited to a format string and 6 simple numeric parameters
#define printf rev_fast_print

using namespace SST;

void send( PIM::SRAM_CMD cmd, uint64_t address, uint64_t data ) {
}

int receive( uint64_t& data ) {
  return 0;
}

int theApp() {
  int      rc      = 0;
  uint64_t readBuf = 0;
  // read PIM_ID
  uint64_t pimBase = PIM::SRAM_BASE;
  send( PIM::SRAM_CMD::READ, pimBase, 0 );
  rc = receive( readBuf );
  if( rc ) {
    rev_fast_print("PIM ID read failed\n");
    return 1;
  }
  rev_fast_print("PIM ID read as 0x%" PRIx64 "\n", readBuf );

  // write/read some other memory
  for( uint64_t addr = 0x20000; addr < 0x20080; addr += 8 )
    send( PIM::SRAM_CMD::WRITE, addr, ( addr << 16 ) + 0xaced );
  for( uint64_t addr = 0x20000; addr < 0x20080; addr += 8 )
    send( PIM::SRAM_CMD::READ, addr, 0 );

  rev_fast_print("Tada!\n");
  return 0;
}

int main( int argc, char** argv ) {
  int rc = theApp();
  return rc;
}
