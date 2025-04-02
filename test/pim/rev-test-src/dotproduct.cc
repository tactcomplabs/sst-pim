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
#include <cstdint>
#include <cstdlib>
#include <cstring>

// PIM definitions
#include "revpim.h"

// Select one and only one
// #define DO_LOOP 1
#define DO_PIM 1

// Globals
const int xfr_size = 256; // dma transfer size in words
uint64_t check_data;
uint64_t sram[PIM::SRAM_SIZE] __attribute__((section(".pimsram")));
uint64_t dram_dst __attribute__((section(".pimdram")));
uint32_t dram_src1[xfr_size] __attribute__((section(".pimdram")));
uint32_t dram_src2[xfr_size] __attribute__((section(".pimdram")));

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
  check_data = 0;
  for (int i = 0; i < xfr_size; i++) {
    uint32_t d1 = (0xacedU << 16) | static_cast<uint32_t>(i);
    uint32_t d2 = (0x89afU << 16) | static_cast<uint32_t>(i);
    dram_src1[i] = d1;
    dram_src2[i] = d2;
    check_data += d1 * d2;
    printf("%d : check_data=0x%lx\n", i, check_data);
  }
  REV_TIME(time2);
  return time2 - time1;
}

#if DO_LOOP
size_t theApp() {
  size_t time1, time2;
  REV_TIME(time1);
  uint64_t sum = 0;
  for (int i = 0; i < xfr_size; i++)
    sum += dram_src1[i] * dram_src2[i];
  REV_TIME(time2);
  return time2 - time1;
}
#endif

#if DO_PIM
size_t theApp() {
  size_t time1, time2;
  REV_TIME(time1);
  revpim::init(PIM::FUNC_NUM::U6, reinterpret_cast<uint64_t *>(dram_dst),
               reinterpret_cast<uint64_t *>(dram_src1),
               reinterpret_cast<uint64_t *>(dram_src2),
               xfr_size * sizeof(uint32_t));
  revpim::run(PIM::FUNC_NUM::U6);
  revpim::finish(PIM::FUNC_NUM::U6); // blocking polling loop :(
  REV_TIME(time2);
  return time2 - time1;
}
#endif

size_t check() {
  size_t time1, time2;
  REV_TIME(time1);
  if (check_data != dram_dst) {
    printf("Failed: check_data=0x%lx dram_dst=0x%lx\n", check_data, dram_dst);
    // assert(false);
  }
  REV_TIME(time2);
  return time2 - time1;
}

int main(int argc, char **argv) {
  printf("Starting userfunc test\n");
  size_t time_id, time_config, time_exec, time_check;

  printf("\ndram_dst=0x%lx\ndram_src1=0x%lx\ndram_src2=0x%lx\nxfr_size=%d\n",
         reinterpret_cast<uint64_t>(dram_dst),
         reinterpret_cast<uint64_t>(dram_src1),
         reinterpret_cast<uint64_t>(dram_src2), xfr_size);

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
