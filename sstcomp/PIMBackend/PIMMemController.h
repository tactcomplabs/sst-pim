//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
// See LICENSE in the top level directory for licensing details
//

#ifndef _PIMMEMORYCONTROLLER_H
#define _PIMMEMORYCONTROLLER_H

// clang-format off
//#include "sst/elements/memHierarchy/memoryController.h"
#include "memoryControllerKG.h"
#include "PIMDecoder.h"

// clang-format on

namespace SST::PIM {

using namespace SST::MemHierarchy;

class PIMMemController : public MemControllerKG {
public:
  /* Element Library Info */
  SST_ELI_REGISTER_COMPONENT(
    PIMMemController,
    "PIM",
    "PIMMemController",
    SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
    "PIM memory controller",
    COMPONENT_CATEGORY_MEMORY
  )

  SST_ELI_DOCUMENT_PARAMS( MEMCONTROLLERKG_ELI_PARAMS )

  SST_ELI_DOCUMENT_PORTS( MEMCONTROLLERKG_ELI_PORTS )

  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS( MEMCONTROLLERKG_ELI_SUBCOMPONENTSLOTS )

  /* Begin class definition */
  //    typedef uint64_t ReqId;

  PIMMemController( ComponentId_t id, Params& params );
  ~PIMMemController();

  /* Event handling */
  void handleMemResponse( SST::Event::id_type id, uint32_t flags ) override;

  /* Component API */
  //virtual void init(unsigned int);
  virtual void setup() override;
  //void finish();

protected:
  //virtual void processInitEvent(MemEventInit* ev);

  virtual void handleEvent( SST::Event* event ) override;

  //virtual bool clock(Cycle_t cycle);

private:
  // mmio decoder (could be generic and static)
  PIMDecoder* mmio_decoder;
};

}  // namespace SST::PIM

#endif /* _PIMMEMORYCONTROLLER_H */
