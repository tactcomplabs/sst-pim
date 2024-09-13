#ifndef _SST_REVPIM_H_
#define _SST_REVPIM_H_

#include "pimdef.h"
#include <cstdint>

// REV includes
#include "rev-macros.h"
#include "syscalls.h"

// REV macros
#undef assert
#define assert TRACE_ASSERT
//rev_fast_print limited to a format string, 6 simple numeric parameters, 1024 char max output
#define printf rev_fast_printf
//#define printf(...)
#define REV_TIME(X) do { asm volatile( " rdtime %0" : "=r"( X ) ); } while (0)

using namespace SST;

namespace revpim {

volatile uint64_t func[PIM::FUNC_SIZE] __attribute__((section(".func_base")));

//
// Initialization functions
//
const unsigned NUM_REV_FUNC_PARAMS = PIM::NUM_FUNC_PARAMS;
void init(PIM::FUNC_NUM f, 
    uint64_t p0=0, uint64_t p1=0, uint64_t p2=0, uint64_t p3=0,
    uint64_t p4=0, uint64_t p5=0, uint64_t p6=0, uint64_t p7=0 ) 
{
    unsigned f_idx = static_cast<unsigned>(f);
    assert(f_idx<PIM::FUNC_SIZE);
    // initialization packet sent sequentially to same MMIO address
    func[f_idx] = static_cast<uint64_t>(PIM::FUNC_CMD::INIT);
    func[f_idx] = p0;
    func[f_idx] = p1;
    func[f_idx] = p2;
    func[f_idx] = p3;
    func[f_idx] = p4;
    func[f_idx] = p5;
    func[f_idx] = p6;
    func[f_idx] = p7;
}

void init(PIM::FUNC_NUM f, void* ptr0, void* ptr1, size_t sz) {
    init(f, reinterpret_cast<uint64_t>(ptr0), reinterpret_cast<uint64_t>(ptr1), reinterpret_cast<uint64_t>(sz));
}

void run(PIM::FUNC_NUM f) {
    unsigned f_idx = static_cast<unsigned>(f);
    assert(f_idx<PIM::FUNC_SIZE);
    func[f_idx] = static_cast<uint64_t>(PIM::FUNC_CMD::RUN);
}

void finish(PIM::FUNC_NUM f) {
    unsigned f_idx = static_cast<unsigned>(f);
    assert(f_idx<PIM::FUNC_SIZE);
    PIM::FSTATE state = static_cast<PIM::FSTATE>(func[f_idx]);
    if (state == PIM::FSTATE::INVALID) {
        assert(false);
    }
    int done = ( state == PIM::FSTATE::DONE);
    while (! done) {
        state = static_cast<PIM::FSTATE>(func[f_idx]);
        done = ( state == PIM::FSTATE::DONE);
    }
    return;
}

} //namespace revpim

#endif // _SST_REVPIM_H_