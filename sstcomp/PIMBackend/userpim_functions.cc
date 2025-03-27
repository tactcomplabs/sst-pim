//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
// See LICENSE in the top level directory for licensing details
//

#include "userpim_functions.h"

namespace SST::PIM {

/// MulVecByScalar -----------------------------------------

MulVecByScalar::MulVecByScalar(TCLPIM *p) : FSM(p) {};

void MulVecByScalar::start(uint64_t params[NUM_FUNC_PARAMS]) {
  unsigned numBytes = (unsigned)params[3];
  if (numBytes == 0)
    return;
  assert((numBytes % 8) == 0);
  this->dst = params[0];
  this->src = params[1];
  this->scalar = params[2];
  total_words = numBytes / 8;
  word_counter = numBytes / 8;
  dma_state = DMA_STATE::READ;
  parent->output->verbose(CALL_INFO, 3, 0,
                          "MulVecByScalar: dst=0x%" PRIx64 " src=0x%" PRIx64
                          "scalar=%" PRId64 " total_words=%" PRId32 "\n",
                          dst, src, scalar, total_words);
}

bool MulVecByScalar::clock() {
#if 1
  unsigned words = word_counter >= 64 ? 64 : word_counter;
#else
  unsigned words = 1;
#endif
  unsigned bytes = words * sizeof(uint64_t);
  // if (parent->buffer.size() != bytes)
  parent->buffer.resize(bytes);
  const bool WRITE = true;
  const bool READ = false;
  if (dma_state == DMA_STATE::READ) {
    parent->m_issueDRAMRequest(src, &parent->buffer, READ,
                               [this](const MemEventBase::dataVec &d) {
                                 assert(parent->buffer.size() == d.size());
                                 // TODO use SRAM to save intermediate data
                                 // TODO better utilities for manage the SST
                                 // payload For now multiply every dword loaded
                                 // by the scalar (messy)
                                 for (size_t i = 0; i < d.size(); i += 8) {
                                   uint64_t data = 0;
                                   uint8_t *p = (uint8_t *)(&data);
                                   for (size_t j = 0; j < 8; j++) {
                                     p[j] = d[i + j];
                                   }
                                   data = scalar * data;
                                   for (size_t j = 0; j < 8; j++) {
                                     parent->buffer[i + j] = p[j];
                                   }
                                 }
                                 dma_state = DMA_STATE::WRITE;
                               });
    dma_state = DMA_STATE::WAITING;
    src += bytes;
  } else if (dma_state == DMA_STATE::WRITE) {
    assert(word_counter >= words);
    word_counter = word_counter - words;
    if (word_counter > 0) {
      parent->m_issueDRAMRequest(dst, &parent->buffer, WRITE,
                                 [this](const MemEventBase::dataVec &d) {
                                   dma_state = DMA_STATE::READ;
                                 });
    } else {
      parent->m_issueDRAMRequest(dst, &parent->buffer, WRITE,
                                 [this](const MemEventBase::dataVec &d) {
                                   dma_state = DMA_STATE::DONE;
                                 });
    }
    dst += bytes;
    dma_state = DMA_STATE::WAITING;
  } else if (dma_state == DMA_STATE::DONE) {
    parent->output->verbose(CALL_INFO, 1, 0, "DMA Done\n");
    dma_state = DMA_STATE::IDLE;
    return true; // finished!
  }
  return false;
}

/// DotProduct -------------------------------------------

DotProduct::DotProduct(TCLPIM *p) : FSM(p) {};

void DotProduct::start(uint64_t params[NUM_FUNC_PARAMS]) {
  unsigned num_bytes = (unsigned)params[2];
  if (num_bytes == 0)
    return;
  assert((num_bytes % 8) == 0);
  this->dst = params[0];
  this->src1 = params[1];
  // this->src2 = params[2];
  this->src2 = this->src1 + num_bytes / 2;
  total_words = num_bytes / 8;
  word_counter = num_bytes / 8;
  dma_state = DMA_STATE::READ1;
  parent->output->verbose(CALL_INFO, 3, 0,
                          "DotProduct: src1=0x%" PRIx64 " src2=0x%" PRIx64
                          "dst=%" PRId64 " total_words=%" PRId32 "\n",
                          src1, src2, dst, total_words);
}

bool DotProduct::clock() {
  unsigned words = word_counter >= 64 ? 64 : word_counter;
  unsigned bytes = words * sizeof(uint64_t);
  if (bytes > 0) {
    parent->buffer.resize(bytes);
  }
  const bool WRITE = true;
  const bool READ = false;
  if (dma_state == DMA_STATE::READ1) {
    word_counter = word_counter - words;
    parent->output->verbose(CALL_INFO, 3, 0,
                            "DotProduct: word_counter=%" PRId32
                            " words=%" PRId32 " bytes=%" PRId32
                            " buffer size=%zu\n",
                            word_counter, words, bytes, parent->buffer.size());

    parent->m_issueDRAMRequest(src1, &parent->buffer, READ,
                               [this](const MemEventBase::dataVec &d) {
                                 // parent->output->verbose(CALL_INFO, 3, 0, "d
                                 // size=%zu\n", d.size());
                                 assert(parent->buffer.size() == d.size());
                                 for (size_t i = 0; i < d.size(); i += 8) {
                                   /*
                                   for (size_t j = 0; j < 8; j++) {
                                     psrc[j] = d[i + j];
                                   }
                                   */
                                   for (size_t j = 0; j < 8; j++) {
                                     parent->buffer[i + j] = d[i + j];
                                   }
                                 }
                                 dma_state = DMA_STATE::READ2;
                               });
    dma_state = DMA_STATE::WAITING;
    src1 += bytes;
  } else if (dma_state == DMA_STATE::READ2) {
    word_counter = word_counter - words;
    parent->output->verbose(CALL_INFO, 3, 0,
                            "DotProduct: word_counter=%" PRId32
                            " words=%" PRId32 " bytes=%" PRId32
                            " buffer size=%zu\n",
                            word_counter, words, bytes, parent->buffer.size());
    if (word_counter > 0) {
      parent->m_issueDRAMRequest(
          src2, &parent->buffer, READ, [this](const MemEventBase::dataVec &d) {
            // parent->output->verbose(CALL_INFO, 3, 0, "DotProduct: d size=%zu
            // parent buffer size=%zu\n",
            //                         d.size(), parent->buffer.size());
            assert(parent->buffer.size() == d.size());
            for (size_t i = 0; i < d.size(); i += 8) {
              uint64_t r2_data = 0;
              uint64_t r1_data = 0;
              uint8_t *pr1 = (uint8_t *)(&r1_data);
              uint8_t *pr2 = (uint8_t *)(&r2_data);
              for (size_t j = 0; j < 8; j++) {
                pr2[j] = d[i + j];
                pr1[j] = parent->buffer[i + j];
              }
              // TODO: Is sum ok here or out of scope?
              sum += r1_data * r2_data;
              parent->output->verbose(CALL_INFO, 3, 0,
                                      "DotProduct: checkpoint sum=0x%" PRIx64
                                      "r1_data=0x%" PRIx64 " r2_data=0x%" PRIx64
                                      "\n",
                                      sum, r1_data, r2_data);
            }
            dma_state = DMA_STATE::READ1;
          });
    } else {
      parent->m_issueDRAMRequest(
          src2, &parent->buffer, READ, [this](const MemEventBase::dataVec &d) {
            // parent->output->verbose(CALL_INFO, 3, 0, "DotProduct: d size=%zu
            // parent buffer size=%zu\n",
            //                         d.size(), parent->buffer.size());
            assert(parent->buffer.size() == d.size());
            for (size_t i = 0; i < d.size(); i += 8) {
              uint64_t r2_data = 0;
              uint64_t r1_data = 0;
              uint8_t *pr1 = (uint8_t *)(&r1_data);
              uint8_t *pr2 = (uint8_t *)(&r2_data);
              for (size_t j = 0; j < 8; j++) {
                pr2[j] = d[i + j];
                pr1[j] = parent->buffer[i + j];
              }
              // TODO: Is sum ok here or out of scope?
              sum += r1_data * r2_data;
              parent->output->verbose(CALL_INFO, 3, 0,
                                      "DotProduct: checkpoint sum=0x%" PRIx64
                                      "r1_data=0x%" PRIx64 " r2_data=0x%" PRIx64
                                      "\n",
                                      sum, r1_data, r2_data);
            }
            uint8_t *psum = (uint8_t *)(&sum);
            for (size_t j = 0; j < 8; j++) {
              // parent->buffer[j] = 0;
              parent->buffer[j] = psum[j];
            }
            dma_state = DMA_STATE::WRITE;
          });
    }
    dma_state = DMA_STATE::WAITING;
    src2 += bytes;
  } else if (dma_state == DMA_STATE::WRITE) {
    parent->buffer.resize(8);
    assert(word_counter >= words);
    word_counter = word_counter - words;
    parent->m_issueDRAMRequest(dst, &parent->buffer, WRITE,
                               [this](const MemEventBase::dataVec &d) {
                                 dma_state = DMA_STATE::DONE;
                               });
    // dst += bytes;
    dma_state = DMA_STATE::WAITING;
  } else if (dma_state == DMA_STATE::DONE) {
    parent->output->verbose(CALL_INFO, 1, 0, "DMA Done\n");
    dma_state = DMA_STATE::IDLE;
    return true; // finished!
  }
  return false;
}

} // namespace SST::PIM
