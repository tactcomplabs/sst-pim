#ifndef _SST_PIMDEF_H_
#define _SST_PIMDEF_H_

#include <cstdint>

#define PIM_TYPE_NONE    0
#define PIM_TYPE_TEST    1
#define PIM_TYPE_RESERVE 2
#define PIM_TYPE_GENERIC 3
        
namespace SST::PIM
{
    // 16M DRAM
    const uint64_t DRAM_SIZE = 0x00001000000llu;
    const uint64_t DRAM_BASE = 0x10000000000llu;

    // 64k sram
    const uint64_t SRAM_SIZE = 0x00000010000llu;
    const uint64_t SRAM_BASE = 0x20000000000llu;

    // 16 functions
    const uint64_t FUNC_SIZE = 0x00000000010llu;
    const uint64_t FUNC_BASE = 0x40000000000llu;

    // SRAM Access
    enum class SRAM_CMD : int { NOP, READ, WRITE, DONE };
    
    // FUNC Access
    enum class FUNC_CMD : int { INIT, RUN, FINISH };

} //namespace SST::PIM

#endif //_SST_PIMDEF_H_
