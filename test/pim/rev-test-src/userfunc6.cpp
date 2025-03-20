/*
 * userfunc6.cpp
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
#include "revpim.h"

// Select one and only one
// #define DO_LOOP 1
#define DO_PIM 1

#define LFSR_SEED 0x1010101010101010ul
#define N 64

// Globals
const uint64_t lfsr_seed = LFSR_SEED;
const uint64_t lfsr_out_len = N;
volatile uint64_t lfsr_out[lfsr_out_len] __attribute__((section(".pimsram")));
uint64_t check_data[lfsr_out_len];

/**
 * 64 bit linear feedback shift register. 
 *
 * x^59 + x^56 + x^54 + x^44 + x^37 + x^36 + x^30 + x^28 + x^23 + x^10 + x^8 + x^5 + 1
 * samples state every 13 cycles
 * 
 * @param out  output array
 * @param len  number of pseudo-random numbers to generate
 * @param seed initial LFSR state
 */

void lfsr_fib(uint64_t * const out, const unsigned len, const uint64_t seed){
  if(seed == 0 || len == 0) return;

  uint64_t lfsr = seed;
  unsigned cycles = 0;

  for(unsigned i = 0; i < len;){
    const uint64_t bit = ((lfsr >> 5) ^ (lfsr >> 8)  ^ (lfsr >> 10) ^ (lfsr >> 20) ^ 
          (lfsr >> 27) ^ (lfsr >> 28) ^ (lfsr >> 34) ^ (lfsr >> 39) ^ 
          (lfsr >> 54) ^ (lfsr >> 56) ^ (lfsr >> 59) ) & 1;
    lfsr = (lfsr >> 1) | (bit << 63);
  
    if(cycles++ == (13-1) ){
      out[i++] = lfsr;
      cycles = 0;
    }
  }
  return;
}

int configure() {
  size_t time1, time2;
  REV_TIME( time1 );
  // Generate check data
  lfsr_fib(check_data, lfsr_out_len, lfsr_seed);
  REV_TIME( time2 );
  return time2 - time1;
}

#if DO_LOOP
size_t theApp() {
  size_t time1, time2;
  REV_TIME( time1 );
  lfsr_fib((uint64_t*)lfsr_out, lfsr_out_len, lfsr_seed);
  REV_TIME( time2 );
  return time2 - time1;
}
#endif

#if DO_PIM
size_t theApp() {
  size_t time1, time2;
  REV_TIME( time1 );
  revpim::init(PIM::FUNC_NUM::U6, (uint64_t) lfsr_out, lfsr_seed, lfsr_out_len);
  revpim::run(PIM::FUNC_NUM::U6);
  revpim::finish(PIM::FUNC_NUM::U6); // blocking polling loop :(
  REV_TIME( time2 );
  return time2 - time1;
}
#endif


size_t check() {
  size_t time1, time2;
  REV_TIME( time1 );
  for (unsigned i=0; i<lfsr_out_len; i++) {
    if (check_data[i] != lfsr_out[i]) {
      printf("Failed: check_data[%d]=0x%llx lfsr_out[%d]=0x%llx\n",
              i, check_data[i], i, lfsr_out[i]);
      assert(false);
    }

  }
  REV_TIME( time2 );
  return time2 - time1;
}

int main( int argc, char** argv ) {
  printf("Starting userfunc6\n");
  size_t time_config, time_exec, time_check;

  printf("\nlfsr_out=0x%lx\nlfsr_seed=0x%lx\nlfsr_out_len=%lu\n", reinterpret_cast<uint64_t>(lfsr_out), lfsr_seed, lfsr_out_len);

  printf("Configuring...\n");
  time_config = configure();
  printf("Executing...\n");
  time_exec = theApp(); 
  printf("Checking...\n");
  time_check = check();

  printf("Results:\n");
  printf("cycles: config=%d, exec=%d, check=%d\n", time_config, time_exec, time_check);
  printf("userfunc6 completed normally\n");
  return 0;
}
