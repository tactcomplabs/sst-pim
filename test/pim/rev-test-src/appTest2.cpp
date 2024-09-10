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
#include <cstring>

// PIM definitions
#include "pimdef.h"

// REV includes
#include "rev-macros.h"
#include "syscalls.h"

#undef assert
#define assert TRACE_ASSERT

//rev_fast_print limited to a format string, 6 simple numeric parameters, 1024 char max output
#define printf rev_fast_printf
//#define printf(...)

using namespace SST;

const int xfr_size = 256;  // dma transfer size in dwords

uint64_t check_data[xfr_size];
#if 1
uint64_t sram[64] __attribute__((section(".pimsram")));
uint64_t dram_src[xfr_size] __attribute__((section(".pimdram")));
uint64_t dram_dst[xfr_size] __attribute__((section(".pimdram")));
#else
uint64_t sram[64];
uint64_t dram_src[xfr_size];
uint64_t dram_dst[xfr_size];
#endif

int configure() {
  // Generate source and check data
  for (int i=0; i<xfr_size ;i++) {
    uint64_t d = (0xaced << 16) | i;
    check_data[i] = d;
    dram_src[i] = d;
  }
  return 0;
}

int theApp() {
  memcpy(dram_dst, dram_src, xfr_size*sizeof(uint64_t));
  return 0;
}

int check() {
  for (int i=0; i<xfr_size; i++) {
    if (check_data[i] != dram_src[i]) {
      printf("Failed: check_data[%d]=0x%lx dram_src[%d]=0x%lx\n",
              i, check_data[i], i, dram_src[i]);
      return 1;
    }
    assert(check_data[i] == dram_dst[i]); 
  }
  return 0;
}

int main( int argc, char** argv ) {
  printf("Starting appTest2\n");

  int rc = configure();
  assert(rc==0);

  rc = theApp();
  assert(rc==0);

  rc = check();
  assert(rc==0);

  printf("appTest2 completed normally\n");
  return rc;
}
