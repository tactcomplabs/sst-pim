/*
 * testfunc.cpp
 *
 * Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
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

// Globals
const int xfr_size = 256; // dma transfer size in dwords
// const uint64_t scalar = 16;
uint64_t check_data[xfr_size];
#if 1
uint64_t sram[PIM::SRAM_SIZE] __attribute__((section(".pimsram")));
uint64_t dram_dst[xfr_size] __attribute__((section(".pimdram")));
uint64_t dram_src[xfr_size] __attribute__((section(".pimdram")));
#else
uint64_t sram[64];
uint64_t dram_src[xfr_size];
uint64_t dram_dst[xfr_size];
#endif

size_t checkPIM() {
  size_t time1, time2;
  // sram offset 0 initialized by PIM hardware
  REV_TIME(time1);
  if (sram[0] != (uint64_t(PIM_TYPE_TCL) << 56)) {
    printf("Unexpected PIM TYPE 0x%lx\n", sram[0]);
    assert(false);
  }
  REV_TIME(time2);
  return time2 - time1;
}

size_t configure() {
  size_t time1, time2;
  REV_TIME(time1);
  // Generate source and check data
  for (int i = 0; i < xfr_size; i++) {
    uint64_t d = (0xaced << 16) | i;
    dram_src[i] = d;
  }
  check_data[0] = 0;
  for (int i = 0; i < xfr_size / 2; i++) {
    printf("%d : check_data=0x%lx\n", i, check_data[0]);
    check_data[0] += dram_src[i] * dram_src[xfr_size / 2 + i];
  }
  REV_TIME(time2);
  return time2 - time1;
}

#if DO_LOOP
size_t theApp() {
  size_t time1, time2;
  REV_TIME(time1);
  uint64_t sum = 0;
  for (int i = 0; i < xfr_size / 2; i++)
    sum += dram_src[xfr_size / 2 + i] * dram_src[i];
  REV_TIME(time2);
  return time2 - time1;
}
#endif

#if DO_PIM
size_t theApp() {
  size_t time1, time2;
  REV_TIME(time1);
  revpim::init(PIM::FUNC_NUM::U6, dram_dst, dram_src,
               xfr_size * sizeof(uint64_t));
  revpim::run(PIM::FUNC_NUM::U6);
  revpim::finish(PIM::FUNC_NUM::U6); // blocking polling loop :(
  REV_TIME(time2);
  return time2 - time1;
}
#endif

size_t check() {
  size_t time1, time2;
  REV_TIME(time1);
  /*
  if (check_data[0] != dram_src[xfr_size/2 + i] * dram_src[i]) {
    printf("Failed: check_data[%d]=0x%lx scalar*dram_src[%d]=0x%lx\n",
            i, check_data[i], i, scalar*dram_src[i]);
    assert(false);
  }
  */
  if (check_data[0] != dram_dst[0]) {
    printf("Failed: check_data[0]=0x%lx dram_dst[0]=0x%lx\n", check_data[0],
           dram_dst[0]);
    assert(false);
  }
  REV_TIME(time2);
  return time2 - time1;
}

int main(int argc, char **argv) {
  printf("Starting userfunc test\n");
  size_t time_id, time_config, time_exec, time_check;

  printf("\ndram_dst=0x%lx\ndram_src=0x%lx\nxfr_size=%d\n",
         reinterpret_cast<uint64_t>(dram_dst),
         reinterpret_cast<uint64_t>(dram_src), xfr_size);

  printf("Checking PIM ID...\n");
  time_id = checkPIM();
  printf("Configuring...\n");
  time_config = configure();
  printf("Executing...\n");
  time_exec = theApp();
  printf("Checking...\n");
  time_check = check();

  printf("Results:\n");
  printf("cycles: id_check=%d, config=%d, exec=%d, check=%d\n", time_id,
         time_config, time_exec, time_check);
  printf("userfunc completed normally\n");
  return 0;
}
