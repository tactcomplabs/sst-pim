#ifndef _SST_PIMDEF_H_
#define _SST_PIMDEF_H_

#include <cstdint>

#define PIM_TYPE_NONE    0
#define PIM_TYPE_TEST    1
#define PIM_TYPE_RESERVE 2
#define PIM_TYPE_GENERIC 3
        
namespace SST::PIM
{
    const uint64_t DRAM_SIZE = 0x20000000000llu;
    const uint64_t DRAM_BASE = 0x08000000000llu;
    const uint64_t SRAM_BASE = 0x10000000000llu;
    const uint64_t FUNC_BASE = 0x20000000000llu;
    enum class Cmd : int { NOP, READ, WRITE, DONE };

} //namespace SST::PIM

#endif //_SST_PIMDEF_H_
