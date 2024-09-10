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

#define REV_TIME(X) do { asm volatile( " rdtime %0" : "=r"( X ) ); } while (0)

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
  size_t time1, time2;
  REV_TIME( time1 );
  // Generate source and check data
  for (int i=0; i<xfr_size ;i++) {
    uint64_t d = (0xaced << 16) | i;
    check_data[i] = d;
    dram_src[i] = d;
  }
  REV_TIME( time2 );
  return time2 - time1;
}

//#define DO_LOOP 1
#if DO_LOOP
size_t theApp() {
  size_t time1, time2;
  REV_TIME( time1 );
  for (int i=0; i<xfr_size; i++) 
    dram_dst[i] = dram_src[i];
  REV_TIME( time2 );
  return time2 - time1;
}
#endif

#define DO_MEMCPY 1
#if DO_MEMCPY
size_t theApp() {
  size_t time1, time2;
  REV_TIME( time1 );
  memcpy(dram_dst, dram_src, xfr_size*sizeof(uint64_t));
  REV_TIME( time2 );
  return time2 - time1;
}
#endif


int check() {
  size_t time1, time2;
  REV_TIME( time1 );
  for (int i=0; i<xfr_size; i++) {
    if (check_data[i] != dram_src[i]) {
      printf("Failed: check_data[%d]=0x%lx dram_src[%d]=0x%lx\n",
              i, check_data[i], i, dram_src[i]);
      return 1;
    }
    assert(check_data[i] == dram_dst[i]); 
  }
  REV_TIME( time2 );
  return time2 - time1;
}

int main( int argc, char** argv ) {

  printf("Starting appTest2\n");
  size_t time_config, time_exec, time_check;

  time_config = configure();
  time_exec = theApp(); 
  time_check = check();

  printf("cycles: config=%d, exec=%d, check=%d", time_config, time_exec, time_check);
  printf("appTest2 completed normally\n");
  return 0;
}
