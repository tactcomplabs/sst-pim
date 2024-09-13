# PIM SST

This project provides a set of SST components that provide methods to model Processor In Memory (PIM) applications combined with the REV RISCV CPU.

The PIM provides a user extensible finite state machine (FSM) that allows sequencing access to/from a region of DRAM and to/from a local SRAM.


## Getting Started

Checkout and build the 'fastprint' branch of the REV CPU.

```
export PIM_REV_HOME=<path to REV>
git clone git@github.com:kpgriesser/sst-pim.git
cd sst-pim
mkdir build
cd build
cmake ../
make -j -s
make install
ctest
```

## Application Examples

```
cd test/pim
make clean
make
cat rev-output/rev-test-src/appTest2/run.revlog
cat rev-output/rev-test-src/userfunc/run.revlog
ls rev-test-src
```

These tests demonstrate both software and hardware algorithms:
- appTest2.cpp: memcpy function in hardware.
- userfunc.cpp: scalar-vector multiply function.

The tests can be modified at compile time by modifying these lines of the source code:
```
// Select one and only one
//#define DO_LOOP 1
//#define DO_MEMCPY 1
#define DO_PIM 1
```

The finite state machines (FSMs) are in sstcomp/PIMBackend:
- tclpim_functions.*: built-in functions
- userpim_functions.*: user provided functions

# TODO

## Linker Script
- Link to standard libraries so we can parse command line arguments.
- Eliminate initialization of global memory segments in the linker script.

## PIM FSM
- Modify FSMs to buffer data in the SRAM. The current code sequences DRAM reads and writes for DMA operation but all calculations on the data are done in the same clock. In real hardware the data needs to be buffered which implies additional states.

## REV/Tests/Demos
- Add demo code to show REV core accessing SRAM.
- REV PR for fast print operation so we can all use the 'devel' branch.

## SST Configuration
- Control regions for Cachability. Currently all REV accesses are non-cachable. Good for latency but it does not represent real-world hardware.

