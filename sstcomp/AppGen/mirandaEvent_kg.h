// Copyright 2009-2024 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2024, NTESS
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// of the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef _H_SST_APPGEN_MIRANDA_EVENT_EVENT
#define _H_SST_APPGEN_MIRANDA_EVENT_EVENT

// clang-format off
// order dependent includes
#include <stdint.h>
#include <sst/core/event.h>
#include <sst/core/params.h>

// clang-format on

namespace SST {
namespace AppGen {

class MirandaReqEvent : public SST::Event {
public:
  struct Generator {
    std::string name;
    SST::Params params;
  };

  std::deque<std::pair<std::string, SST::Params>> generators;

  uint64_t key;

private:
  void serialize_order( SST::Core::Serialization::serializer& ser ) override {
    Event::serialize_order( ser );
    ser& key;
    ser& generators;
  }

  ImplementSerializable( SST::AppGen::MirandaReqEvent );
};

class MirandaRspEvent : public SST::Event {
public:
  uint64_t key;

private:
  void serialize_order( SST::Core::Serialization::serializer& ser ) override {
    Event::serialize_order( ser );
    ser& key;
  }

  ImplementSerializable( SST::AppGen::MirandaRspEvent );
};

}  // namespace AppGen
}  // namespace SST

#endif  //_H_SST_APPGEN_MIRANDA_EVENT_EVENT
