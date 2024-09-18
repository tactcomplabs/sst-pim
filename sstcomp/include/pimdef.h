#ifndef _SST_PIMDEF_H_
#define _SST_PIMDEF_H_

#include <cstdint>

#define PIM_TYPE_NONE    0
#define PIM_TYPE_TEST    1
#define PIM_TYPE_RESERVE 2
#define PIM_TYPE_TCL     3
        
namespace SST::PIM
{    
    // Important: Keep memory map in sync with pim.lds

    // 16 built-in functions + 16 user functions
    const uint64_t FUNC_LEN  = 16 + 16;

    // Default function MMIO base address
    const uint64_t DEFAULT_FUNC_BASE_ADDR = 0x0E000000llu;
    
    // Default SRAM base address
    const uint64_t DEFAULT_SRAM_BASE_ADDR = 0x0E000100llu;
    
    // Default DRAM base address
    const uint64_t DEFAULT_DRAM_BASE_ADDR = 0x0E000500llu;
    
    // Default PIM region upper bound (exclusive)
    const uint64_t DEFAULT_REG_BOUND_ADDR = 0x0E400500llu;

    // SRAM Access
    enum class SRAM_CMD : int { NOP, READ, WRITE, DONE };
    
    // FUNC Access
    enum class FUNC_CMD : int { INIT, RUN, FINISH };

    // Function ID Convenience
    enum class FUNC_NUM    : int { 
        F0, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15,
        U0, U1, U2, U3, U4, U5, U6, U7, U8, U9, U10, U11, U12, U13, U14, U15
    };
    
    // Number of funtion params to send on init
    const unsigned NUM_FUNC_PARAMS = 8;

    // Function states
    enum class FSTATE  : int { INVALID, INITIALIZING, READY, RUNNING, DONE };

} //namespace SST::PIM

#endif //_SST_PIMDEF_H_
