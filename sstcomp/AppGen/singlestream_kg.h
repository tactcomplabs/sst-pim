// Copyright 2009-2023 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2023, NTESS
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// of the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef _H_SST_APPGEN_SINGLE_STREAM_GEN
#define _H_SST_APPGEN_SINGLE_STREAM_GEN

#include "mirandaGenerator_kg.h"
#include <sst/core/output.h>

#include <queue>

namespace SST {
namespace AppGen {

class SingleStreamGenerator_KG : public RequestGenerator {

public:
  SingleStreamGenerator_KG( ComponentId_t id, Params& params );
  void build( Params& params );
  ~SingleStreamGenerator_KG();
  void generate( MirandaRequestQueue<GeneratorRequest*>* q );
  bool isFinished();
  void completed();

  SST_ELI_REGISTER_SUBCOMPONENT(
    SingleStreamGenerator_KG,
    "AppGen",
    "SingleStreamGenerator_KG",
    SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
    "Creates a single ordered stream of accesses to/from memory",
    SST::AppGen::RequestGenerator
  )

  SST_ELI_DOCUMENT_PARAMS(
    { "verbose", "Sets the verbosity of the output", "0" },
    { "count", "Total number of requests", "1000" },
    { "length", "Sets the length of the request", "8" },
    { "startat", "Sets the start address of the array", "0" },
    { "max_address", "Maximum address allowed for generation", "524288" },
    { "memOp", "All reqeusts will be of this type, [Read/Write]", "Read" },
  )

private:
  uint64_t reqLength;
  uint64_t maxAddr;
  uint64_t issueCount;
  uint64_t nextAddr;
  uint64_t startAddr;

  Output*      out;
  ReqOperation memOp;
};

}  // namespace AppGen
}  // namespace SST

#endif  //_H_SST_APPGEN_SINGLE_STREAM_GEN
