/*
 * userfunc8.cpp
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

#include "userfunc8.h"

// Select one and only one
// #define DO_LOOP 1
#define DO_PIM 1

// Globals
const unsigned vertices = 512;
const uint64_t src = 21;
const uint64_t target = 509;
const uint64_t * const test_data = large_matrix_512;
uint64_t check_data[vertices*vertices];
uint64_t check_ret_code;
// total_dram_size = distance_matrix + came_from
const size_t total_dram_size = ((vertices * vertices) + vertices) * sizeof(uint64_t);
static_assert(total_dram_size <= (SST::PIM::DEFAULT_REG_BOUND_ADDR - SST::PIM::DEFAULT_DRAM_BASE_ADDR));
volatile uint8_t pim_dram[total_dram_size] __attribute__((section(".pimdram")));
volatile uint64_t * const distance_matrix = (uint64_t *) &pim_dram;
volatile uint64_t * const came_from       = distance_matrix + (vertices*vertices);
volatile uint64_t ret_code __attribute__((section(".pimsram")));

/**
 * admissable and consistent heuristic function
 * 
 * astar helper function
 */
uint64_t h(const uint64_t src, const uint64_t target) {
  return 0;
}

/**
 * returns the index with the lowest fscore in openSet
 * 
 * astar helper function
 */
uint64_t openSetTop(const uint64_t * const openSet, const uint64_t * const fscore, const unsigned vertices) {
  unsigned lowestFScore = UINT32_MAX;
  unsigned lowestFScoreIdx = UINT32_MAX;
  for(unsigned i = 0; i < vertices; i++){
    if(openSet[i] == 1 && (fscore[i] < lowestFScore)){
      lowestFScore = fscore[i];
      lowestFScoreIdx = i;
    }
  }
  return lowestFScoreIdx;
}

/**
 * calculates the shortest path between two vertices in a given graph
 * assumes maximum distances between two vertices is less than UINT32_MAX
 * 
 * @param ret_code 0 if success, otherwise 1
 * @param came_from output array of length `vertices`, data undefined if ret_code != 0
 * @param dist symmetrical distance matrix of length `vertices * vertices`
 * @param src vertex to find path from
 * @param target vertex to find path to
 * @param vertices number of vertices in graph
 */
void aStar(uint64_t &ret_code, uint64_t * const came_from, const uint64_t * const dist, const unsigned src, const unsigned target, const unsigned _vertices) {
  static volatile uint64_t gscore[vertices];
  static volatile uint64_t fscore[vertices];
  static volatile uint64_t open_set[vertices];

  for(int i = 0; i < vertices; i++){
    gscore[i] = UINT32_MAX;
    fscore[i] = UINT32_MAX;
    open_set[i] = 0;
  }

  gscore[src] = 0;
  fscore[src] = h(src,target);
  open_set[src] = 1;
  came_from[src] = src;
  
  unsigned curr;
  while((curr = openSetTop(open_set, fscore, vertices)) != UINT32_MAX) {
    open_set[curr] = 0;

    if (curr == target) { ret_code = 0; return; }

    for(unsigned neighbor = 0; neighbor < _vertices; neighbor++){
      if(curr == neighbor) continue;
      if(dist[(_vertices*curr)+neighbor] == UINT64_MAX) continue;

      const uint64_t tentative_g_score = gscore[curr] + dist[(_vertices*curr)+neighbor];
      if (tentative_g_score >= gscore[neighbor]) continue;

      came_from[neighbor] = curr;
      gscore[neighbor]   = tentative_g_score;
      fscore[neighbor]   = tentative_g_score + h(neighbor,target);
      open_set[neighbor]  = 1;
    }
  }

  ret_code = 1;
  return;
}

int configure() {
  size_t time1, time2;
  REV_TIME( time1 );
  for(unsigned i = 0; i<(vertices*vertices); i++){
    distance_matrix[i] = test_data[i];
  }
  for(unsigned i = 0; i<vertices; i++){
    came_from[i] = UINT64_MAX;
  }
  aStar(check_ret_code,(uint64_t*)check_data,(uint64_t*)distance_matrix,src,target,vertices);
  REV_TIME( time2 );
  return time2 - time1; 
}

#if DO_LOOP
size_t theApp() {
  size_t time1, time2;
  REV_TIME( time1 );
  aStar((uint64_t&)ret_code,(uint64_t*)came_from,(uint64_t*)distance_matrix,src,target,vertices);
  REV_TIME( time2 );
  return time2 - time1;
}
#endif

#if DO_PIM
size_t theApp() {
  size_t time1, time2;
  REV_TIME( time1 );
  revpim::init(PIM::FUNC_NUM::U8, &ret_code, (uint64_t)came_from, (uint64_t)distance_matrix, src, target, vertices);
  revpim::run(PIM::FUNC_NUM::U8);
  revpim::finish(PIM::FUNC_NUM::U8); // blocking polling loop :(
  REV_TIME( time2 );
  return time2 - time1;
}
#endif

size_t check() { 
  size_t time1, time2;
  REV_TIME( time1 );

  if(check_ret_code != ret_code) {
    printf("Failed: check_ret_code=%u ret_code=%u\n",check_ret_code,ret_code);
    assert(false);
  }
  if(check_ret_code != 0){
    REV_TIME( time2 );
    return time2 - time1;
  }

  unsigned i = target;
  while(i != UINT32_MAX) {
    if (check_data[i] != came_from[i]) {
      printf("Failed: check_data[%u]=%llu came_from[%d]=%llu\n",
              i, check_data[i], i,came_from[i]);
      assert(false);
    }

    if(i == src) break;
    i = check_data[i];
  }

  for (unsigned i=0;i<(vertices*vertices);i++) {
    if(test_data[i] != distance_matrix[i]) {
      printf("Failed: test_data[%u]=%llu distance_matrix[%u]=%llu\n",
              i, check_data[i], i,distance_matrix[i]);
      assert(false);
    }
  }

  REV_TIME( time2 );
  return time2 - time1;
}

int main( int argc, char** argv ) {
  printf("Starting userfunc8\n");
  size_t time_config, time_exec, time_check;

  printf("\ncheck_data=0x%lx\ncame_from=0x%lx\ndistance_matrix=0x%lx\nsrc=%llu\ntarget=%llu\nvertices=%llu\n",
    reinterpret_cast<uint64_t>(check_data), 
    reinterpret_cast<uint64_t>(came_from), 
    reinterpret_cast<uint64_t>(distance_matrix), 
    src,
    target,
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
  printf("userfunc8 completed normally\n");
  return 0;
}
