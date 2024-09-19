/*
 * userfunc7.cpp
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

#include "userfunc7.h"

// Select one and only one
#define DO_LOOP 1
// #define DO_PIM 1

#define N_VERTICES 16
#define DIST_MASK 0xFu

// Globals
const unsigned vertices = N_VERTICES;
const uint64_t dist_mask = DIST_MASK;
static_assert(vertices<=RANDOM_ARRAY_LEN);
volatile uint64_t check_data[vertices*vertices];
const uint64_t total_dram_size = ((vertices*vertices)+vertices)*sizeof(uint64_t);
volatile uint8_t pim_dram[total_dram_size] __attribute__((section(".pimdram")));
volatile uint64_t * const distance_matrix = (uint64_t *) &pim_dram;
volatile uint64_t * const rand_src        = (uint64_t *)(distance_matrix + (vertices*vertices));

uint32_t clz(uint32_t src){
  uint32_t mask = INT32_MIN;
  unsigned zeroes = 0;
  while((src & (mask >> zeroes)) == 0 && zeroes != 32) ++zeroes;
  return zeroes;
}

/**
 * Creates a symmetric distance matrix.
 * 
 * @param out       output array
 * @param rand_src  random source array (must have length n)
 * @param dist_mask randomized distances are masked by this value before being added to the matrix.
 * @param n         number of vertices 
 */
void symmetricDistanceMatrix(uint64_t * const out, const uint64_t * const rand_src, const uint64_t dist_mask, const uint64_t n){
  assert(dist_mask <= UINT32_MAX);
  assert(n > 0);

  const uint32_t prob_mask = (1 << (32-clz(n >> 2)))-1;

  for (uint64_t r = 0; r<n; r++){
    out[(r*n)+r] = 0;

    for(uint64_t c = r+1; c<n; c++){
      const uint64_t xor_rand = rand_src[r] ^ rand_src[c];
      const uint32_t xor_rand_msb = xor_rand >> 32;
      const uint32_t xor_rand_lsb = xor_rand & UINT32_MAX;

      const bool doEdge = (xor_rand_msb & prob_mask) == prob_mask;
      const uint32_t dist = xor_rand_lsb & dist_mask;
      // if(doEdge)
      //   printf("writing to edge=%u dist=%u xor_rand_lsb=0x%lx dist_mask=0x%lx\n", (r*n) + c, doEdge ? dist : UINT64_MAX,xor_rand_lsb,dist_mask);
      out[(r*n) + c] = doEdge ? dist : UINT64_MAX;
      out[(c*n) + r] = doEdge ? dist : UINT64_MAX;
    }
  }
}

int configure() {
  size_t time1, time2;
  REV_TIME( time1 );
  // Generate check data
  for(unsigned i = 0; i<vertices; i++){
    rand_src[i] = RANDOM_ARRAY[i];
  }
  symmetricDistanceMatrix((uint64_t*)check_data,(uint64_t*)rand_src,dist_mask,vertices);
  REV_TIME( time2 );
  return time2 - time1; 
}

#if DO_LOOP
size_t theApp() {
  size_t time1, time2;
  REV_TIME( time1 );
  symmetricDistanceMatrix((uint64_t*)distance_matrix,(uint64_t*)rand_src,dist_mask,vertices);
  REV_TIME( time2 );
  return time2 - time1;
}
#endif

// #if DO_PIM
// size_t theApp() {
//   size_t time1, time2;
//   REV_TIME( time1 );
//   revpim::init(PIM::FUNC_NUM::U6, dram_dst, dram_src0, dram_src1, xfr_size*sizeof(uint64_t));
//   revpim::run(PIM::FUNC_NUM::U6);
//   revpim::finish(PIM::FUNC_NUM::U6); // blocking polling loop :(
//   REV_TIME( time2 );
//   return time2 - time1;
// }
// #endif


size_t check() {
  size_t time1, time2;
  REV_TIME( time1 );
  for (unsigned i=0; i<(vertices*vertices); i++) {
    if (check_data[i] != distance_matrix[i]) {
      printf("Failed: check_data[%u]=%lu distance_matrix[%d]=%lu\n",
              i, check_data[i], i,distance_matrix[i]);
      assert(false);
    }
  }

  for (unsigned i=0;i<vertices;i++) {
    if(RANDOM_ARRAY[i] != rand_src[i]) {
      printf("Failed: RANDOM_ARRAY[%u]=%lu rand_src[%u]=%lu\n",
              i, check_data[i], i,rand_src[i]);
      assert(false);
    }
  }

  REV_TIME( time2 );
  return time2 - time1;
}

int main( int argc, char** argv ) {
  printf("Starting userfunc7\n");
  size_t time_config, time_exec, time_check;

  printf("\ncheck_data=0x%lx\nrand_src=0x%lx\ndistance_matrix=0x%lx\ndist_mask=0x%lx\nvertices=%lu\n",
    reinterpret_cast<uint64_t>(check_data), 
    reinterpret_cast<uint64_t>(RANDOM_ARRAY), 
    reinterpret_cast<uint64_t>(distance_matrix), 
    reinterpret_cast<uint64_t>(dist_mask), 
    vertices
  );

  printf("Configuring...\n");
  time_config = configure();
  printf("Executing...\n");
  time_exec = theApp(); 
  printf("Checking...\n");
  time_check = check();

  printf("Results:\n");
  printf("cycles: config=%d, exec=%d, check=%d\n", time_config, time_exec, time_check);
  printf("userfunc7 completed normally\n");
  return 0;
}
