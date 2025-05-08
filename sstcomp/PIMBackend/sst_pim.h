//
// _sst_pim_h_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

// Header file to include all SST headers, so that Rev compiler warnings can be
// turned off during third-party SST header inclusion.

#ifndef _SST_PIM_H_
#define _SST_PIM_H_

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wreorder"
#pragma GCC diagnostic ignored "-Wmissing-noreturn"

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Wsuggest-final-methods"
#pragma GCC diagnostic ignored "-Wsuggest-final-types"
// kg added
#pragma GCC diagnostic ignored "-Wformat"
#endif

// The #include order is important, so we prevent clang-format from reordering
// clang-format off
#include <sst/core/sst_config.h>
#include <sst/core/component.h>
#include <sst/core/event.h>
#include <sst/core/link.h>

#include <sst/core/params.h>
#include <sst/core/sst_types.h>

#include "memEventBase.h"
#include "memEvent.h"
#include "memEventCustom.h"
#include "cacheListener.h"
#include "memNIC.h"
#include "memLink.h"
#include "util.h"

#include <sst/elements/memHierarchy/cacheListener.h>
#include <sst/elements/memHierarchy/customcmd/customCmdMemory.h>
#include <sst/elements/memHierarchy/memEvent.h>
#include <sst/elements/memHierarchy/memLinkBase.h>
#include <sst/elements/memHierarchy/membackend/backing.h>
#include <sst/elements/memHierarchy/membackend/memBackend.h>
#include <sst/elements/memHierarchy/membackend/memBackendConvertor.h>
#include <sst/elements/memHierarchy/util.h>

// clang-format on

#pragma GCC diagnostic pop

#endif
