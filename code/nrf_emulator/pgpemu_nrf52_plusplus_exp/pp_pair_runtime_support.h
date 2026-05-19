#pragma once

#include <stddef.h>
#include <stdint.h>

#include "pp_types.h"

struct PairRuntimeSupportRefs {
  uint8_t* firstPairCertState;
  DonorAuthRuntimeState* donorAuthRuntime;
  AuthParserOutputState* authParserOutput;
  AuthReplyIndexSlotState* authReplyIndexTable;
  size_t authReplyIndexTableLen;
  uint8_t* freshAuthStep;
  uint32_t* freshAuthAt;
  void (*printHex)(const uint8_t* data, uint16_t len);
  void (*setAuthParserSchedulerState)(uint8_t schedulerState,
                                      uint8_t outputClass,
                                      uint16_t outputLen,
                                      uint16_t outputOffset,
                                      uint8_t resultCode,
                                      uint8_t continuationKind,
                                      uint8_t lastOpcode,
                                      bool outputPending,
                                      bool streamPending,
                                      bool notifyInFlight);
};

const char* ppFirstPairCertStateLabel(uint8_t state);
void ppSetFirstPairCertState(const PairRuntimeSupportRefs& refs,
                             uint8_t nextState,
                             const char* reason);
void ppResetDonorAuthRuntime(const PairRuntimeSupportRefs& refs);
void ppResetAuthParserOutput(const PairRuntimeSupportRefs& refs);
void ppLogDonorAuthRuntime(const PairRuntimeSupportRefs& refs, const char* reason);
uint8_t ppDonorLocal29CommandCodeForFamily(uint8_t family);
uint8_t ppDeriveDonorLocalFamilyFromParserBytes(const PairRuntimeSupportRefs& refs);
void ppQueueDonorAuthRuntimeStep(const PairRuntimeSupportRefs& refs,
                                 uint8_t nextStep,
                                 uint32_t delayMs,
                                 const char* reason);
void ppSeedAuthReplyIndexTable(const PairRuntimeSupportRefs& refs,
                               const uint8_t* defaultReply2401);
void ppUpdateAuthReplyIndexSlot(const PairRuntimeSupportRefs& refs,
                                uint8_t index,
                                const uint8_t* value3,
                                const char* reason);
void ppBuildIndexedAuthReply24(const PairRuntimeSupportRefs& refs,
                               uint8_t index,
                               uint8_t* out5,
                               const char* reason);
