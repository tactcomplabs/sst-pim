//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
// See LICENSE in the top level directory for licensing details
//

#ifndef _H_SST_PIM_BACKEND_
#define _H_SST_PIM_BACKEND_

// clang-format off
#include "sst/elements/memHierarchy/membackend/memBackend.h"
#include "pim.h"
#include "memEvent.h"
#include <queue>
// clang-format on

using namespace SST::MemHierarchy;

#define PIM_WRITE Command::Write
#define PIM_READ  Command::GetS

namespace SST::PIM {

class PIM;

class PIMBackend : public SimpleMemBackend {
public:
  /* Element Library Info */
  SST_ELI_REGISTER_SUBCOMPONENT(
    PIMBackend,
    "PIM",
    "PIMBackend",
    SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
    "PIM Backend for nesting other backends",
    SST::MemHierarchy::SimpleMemBackend
  )

  SST_ELI_DOCUMENT_PARAMS(
    MEMBACKEND_ELI_PARAMS,
    /* Own parameters */
    { "verbose", "Sets the verbosity of the backend output", "0" },
    { "backend", "Backend memory system", "memHierarchy.simpleMem" },
    { "request_delay", "Constant delay to be added to requests with units (e.g., 1us)", "0ns" },
    { "pim_type", "1:test mode, 2:reserved, 3:tclpim", "1" },
    { "num_nodes", "Number of nodes", "1" },
  )

  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS( { "backend", "Backend memory model", "SST::MemHierarchy::SimpleMemBackend" } )

  /* Begin class definition */
  PIMBackend();
  PIMBackend( ComponentId_t id, Params& params );
  virtual ~PIMBackend();
  virtual bool issueRequest( ReqId, Addr, bool isWrite, unsigned numBytes ) override;

  virtual std::string getBackendConvertorType() override { return "memHierarchy.simpleMemBackendConvertor"; }

  // PIM Callbacks for memory controller event injection from PIM and response to PIM. Map to MemController::handleEvent
  virtual void setEventInjectionHandler( std::function<void( SST::Event* )> handler ) { m_pimRequest = handler; }

  // Use component name for issuing dram requests. Memhierarchy won't recognize the backend as a source or dest.
  void setComponentName( const std::string& name );

  const std::string& getComponentName() { return componentName; }

  // Called by PIM to initiate a new DRAM request
  void issueDRAMRequest(
    uint64_t a, MemEventBase::dataVec* d, bool isWrite, std::function<void( const MemEventBase::dataVec& )> completion
  );

  // Delay Buffer ( delay_self_link )
  void handleNextRequest( SST::Event* ev );

  // PIM DRAM access
  void handlePIMCompletion( SST::Event* );        // PIM DRAM access done
  void handleMMIOReadCompletion( SST::Event* );   // Memory controller gets PIM Data to provide in response to network
  void handleMMIOWriteCompletion( SST::Event* );  // Memory controller writes event payload into PIM

  /* Component API */
  void         setup() override;
  void         finish() override;
  virtual bool clock( Cycle_t cycle ) override;
// TODO put back
#if 0
    virtual bool isClocked() override { return backend->isClocked(); }
#else
  virtual bool isClocked() override { return true; }
#endif

protected:
  SST::Output pimOutput;  /// special output stream for PIM different from parent class output

private:
  unsigned num_nodes = 0;  // Important: Set by configuration to >0
  void     handleMemReponse( ReqId id );

  struct Req {
    Req( ReqId id, Addr addr, bool isWrite, unsigned numBytes )
      : id( id ), addr( addr ), isWrite( isWrite ), numBytes( numBytes ) {}

    ReqId    id;
    Addr     addr;
    bool     isWrite;
    unsigned numBytes;
  };

  SimpleMemBackend* backend;
  unsigned int      fwdDelay;
  Link*             delay_self_link;

  std::queue<Req> requestBuffer;

  std::string                                                  componentName = "none";
  PIM*                                                         pimsim;
  uint32_t                                                     pim_type;
  std::map<Event::id_type,
           std::function<void( const MemEventBase::dataVec )>> pendingPIMEvents;  // id, completion map

  std::map<ReqId, MemEvent*> outstandingPIMReqs;

  std::function<void( SST::Event* )> m_pimRequest;  // called by backend. memory controller's incoming event handler

  bool  initDRAMDone  = false;
  bool  selfCheckDone = false;
  ReqId nextPIMReqID  = 0;

  ReqId genPIMReqID() { return nextPIMReqID++; }

  MemEventBase::dataVec buffer = { 0, 0, 0, 0, 0, 0, 0, 0 };

  unsigned node_id;
  uint64_t spdBase;

};  //class PIMBackend

}  // namespace SST::PIM

#endif
