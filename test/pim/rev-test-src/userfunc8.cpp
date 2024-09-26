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

#include "userfunc8.h"

// Select one and only one
#define DO_LOOP 1
// #define DO_PIM 1

// N_VERTICES should equal the stride length to force cache set thrashing
#define CACHE_L1_SIZE 16 * 1024
#define CACHE_L1_ASSOCIATIVITY 4
#define N_VERTICES CACHE_L1_SIZE / (CACHE_L1_ASSOCIATIVITY * sizeof(uint64_t))

// Globals
const unsigned vertices = 4;
const uint64_t src = 0;
const uint64_t target = 3;
const uint64_t * const test_data = two_path_4;
uint64_t check_data[vertices*vertices];

// distance_matrix, + came_from
const size_t total_dram_size = ((vertices * vertices) + vertices) * sizeof(uint64_t);
static_assert(total_dram_size <= (SST::PIM::DEFAULT_REG_BOUND_ADDR - SST::PIM::DEFAULT_DRAM_BASE_ADDR));
volatile uint8_t pim_dram[total_dram_size] __attribute__((section(".pimdram")));

volatile uint64_t * const distance_matrix = (uint64_t *) &pim_dram;
volatile uint64_t * const came_from       = distance_matrix + (vertices*vertices);

/**
 * admissable and consistent heuristic function
 */
uint64_t h(const uint64_t src, const uint64_t target) {
  return 0;
}

/**
 * returns the index with the lowest f_score in openSet
 */
uint64_t openSetTop(const uint64_t * const openSet, const uint64_t * const f_score, const unsigned vertices) {
  int lowestFScore = UINT64_MAX;
  int lowestFScoreIdx = UINT64_MAX;
  for(int i = 0; i < vertices; i++){
    if(openSet[i] == 1 && (f_score[i] < lowestFScore)){
      lowestFScore = f_score[i];
      lowestFScoreIdx = i;
    }
  }
  return lowestFScoreIdx;
}

/**
 * calculates the shortest path between two vertices in a given graph
 * 
 * @param came_from output array of length `vertices`
 * @param dist symmetrical distance matrix of length `vertices * vertices`
 * @param src vertex to find path from
 * @param target vertex to find path to
 * @param vertices number of vertices in graph
 * @returns 0 path found, else 1
 */
unsigned aStar(uint64_t * const came_from, const uint64_t * const dist, const unsigned src, const unsigned target, const unsigned _vertices) {
  volatile uint64_t g_score[vertices];
  volatile uint64_t f_score[vertices];
  volatile uint64_t open_set[vertices];

  for(int i = 0; i < vertices; i++){
    g_score[i] = UINT64_MAX;
    f_score[i] = UINT64_MAX;
    open_set[i] = 0;
  }

  open_set[src] = 1;
  g_score[src] = 0;
  f_score[src] = h(src,target);
  
  unsigned curr;
  while((curr = openSetTop(open_set, f_score, vertices)) != UINT64_MAX) {
    open_set[curr] = 0;
  
    #ifdef DEBUG
    printf("curr=%d\n",curr);
    #endif

    if (curr == target) { return 0; }

    for(unsigned neighbor = 0; neighbor < _vertices; neighbor++){
      if(curr == neighbor) continue;
      if(dist[(_vertices*curr)+neighbor] == UINT64_MAX) continue;

      const uint64_t tentative_g_score = g_score[curr] + dist[(_vertices*curr)+neighbor];
      if (tentative_g_score >= g_score[neighbor]) continue;

      #ifdef DEBUG
      printf("neighbor=%d dist[%d]=%d g_score=%d tentative_g_score=%d\n", 
      neighbor, (nVertices*curr)+neighbor, dist[(nVertices*curr)+neighbor], g_score[neighbor], tentative_g_score);
      #endif

      came_from[neighbor] = curr;
      g_score[neighbor]   = tentative_g_score;
      f_score[neighbor]   = tentative_g_score + h(neighbor,target);
      open_set[neighbor]  = 1;
    }
  }

  return 1;
}

int configure() {
  size_t time1, time2;
  REV_TIME( time1 );
  for(unsigned i = 0; i<(vertices*vertices); i++){
    distance_matrix[i] = test_data[i];
  }
  aStar((uint64_t*)check_data,(uint64_t*)distance_matrix,0,3,vertices);
  REV_TIME( time2 );
  return time2 - time1; 
}

#if DO_LOOP
size_t theApp() {
  size_t time1, time2;
  REV_TIME( time1 );
  aStar((uint64_t*)came_from,(uint64_t*)distance_matrix,0,3,vertices);
  REV_TIME( time2 );
  return time2 - time1;
}
#endif

#if DO_PIM
size_t theApp() {
  size_t time1, time2;
  REV_TIME( time1 );
  revpim::init(PIM::FUNC_NUM::U8, (uint64_t)came_from, (uint64_t)distance_matrix, 0, 3, vertices);
  revpim::run(PIM::FUNC_NUM::U8);
  revpim::finish(PIM::FUNC_NUM::U8); // blocking polling loop :(
  REV_TIME( time2 );
  return time2 - time1;
}
#endif


size_t check() { 
  size_t time1, time2;
  REV_TIME( time1 );
  unsigned n = 0;
  unsigned i = src;
  while(n++ < vertices) { //TODO only path is guaranteed, untouched memory may b ediferent
    if (check_data[i] != came_from[i]) {
      printf("Failed: check_data[%u]=%llu came_from[%d]=%llu\n",
              i, check_data[i], i,came_from[i]);
      assert(false);
    }

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

  printf("\ncheck_data=0x%lx\ntest_data=0x%lx\ndistance_matrix=0x%lx\nsrc=%llu\ntarget=%llu\nvertices=%llu\n",
    reinterpret_cast<uint64_t>(check_data), 
    reinterpret_cast<uint64_t>(test_data), 
    reinterpret_cast<uint64_t>(distance_matrix), 
    src,
    target,
    vertices
  );

  printf("Configuring...\n");
  time_config = configure();
  printf("Executing...\n");
  // time_exec = theApp(); 
  printf("Checking...\n");
  // time_check = check();

  printf("Results:\n");
  printf("cycles: config=%d, exec=%d, check=%d\n", time_config, time_exec, time_check);
  printf("userfunc8 completed normally\n");
  return 0;
}
