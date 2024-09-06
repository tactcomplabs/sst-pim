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

// REV includes
#include "rev-macros.h"
#include "syscalls.h"

// PIM definitions
#include "pimdef.h"

#undef assert
#define assert TRACE_ASSERT

// rev_fast_print is limited to 6 simple numeric parameters
#define printf rev_fast_print

void theApp() {
  rev_fast_print("Tada!\n");
}

int main( int argc, char** argv ) {
  theApp();
  return 0;
}
