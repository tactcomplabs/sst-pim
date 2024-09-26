#include "stdint.h"

// no neighbors
const uint64_t no_neighbors_4[16] = {
    0         ,UINT64_MAX,UINT64_MAX,UINT64_MAX,
    UINT64_MAX,0         ,UINT64_MAX,UINT64_MAX,
    UINT64_MAX,UINT64_MAX,0         ,UINT64_MAX,
    UINT64_MAX,UINT64_MAX,UINT64_MAX,0
};

// no path
const uint64_t no_path_4[16] = {
    0         ,10        ,UINT64_MAX,UINT64_MAX,
    10        ,0         ,5         ,UINT64_MAX,
    UINT64_MAX,5         ,0         ,UINT64_MAX,
    UINT64_MAX,UINT64_MAX,UINT64_MAX,0
};

// 0 -> 1 -> 2 -> 3
const uint64_t one_path_4[16] = {
    0         ,10        ,UINT64_MAX,UINT64_MAX,
    10        ,0         ,5         ,UINT64_MAX,
    UINT64_MAX,5         ,0         ,5,
    UINT64_MAX,UINT64_MAX,5         ,0
};

// 0 -> 2 -> 3
const uint64_t two_path_4[16] = {
    0         ,10        ,5         ,UINT64_MAX,
    10        ,0         ,5         ,UINT64_MAX,
    5         ,5         ,0         ,5,
    UINT64_MAX,UINT64_MAX,5         ,0
};
