#pragma once

#include <stddef.h>
#include <stdint.h>

#include "pp_types.h"

struct PairHelperRefs {
  ServiceDCertState* serviceDCertState;
  AuthParserOutputState* authParserOutput;
  uint8_t* serviceDRuntimeBlock;
  size_t serviceDRuntimeBlockLen;
  uint8_t* persistedServiceDSelector4;
  bool* persistedServiceDSelector4Valid;
  uint8_t* persistedServiceDSelector5;
  bool* persistedServiceDSelector5Valid;
  uint8_t* persistedServiceDRecord6;
  bool* persistedServiceDRecord6Valid;
  uint32_t* persistedServiceDRecord1A;
  bool* persistedServiceDRecord1AValid;
  const uint8_t* firmwareRecord4Default;
  size_t persistedServiceDSelector4Len;
  size_t persistedServiceDRecord6Len;
  const uint8_t* realPlusPlusAddrLe;
  size_t realPlusPlusAddrLen;
  PairingStage* pairingStage;
  void (*printHex)(const uint8_t* data, uint16_t len);
};

void ppCopyAddressDisplay(const PairHelperRefs& refs, uint8_t* dest);
void ppLogServiceDCertState(const PairHelperRefs& refs, const char* reason);
void ppSetAuthParserSchedulerState(const PairHelperRefs& refs,
                                   uint8_t schedulerState,
                                   uint8_t outputClass,
                                   uint16_t outputLen,
                                   uint16_t outputOffset,
                                   uint8_t resultCode,
                                   uint8_t continuationKind,
                                   uint8_t lastOpcode,
                                   bool outputPending,
                                   bool streamPending,
                                   bool notifyInFlight);
void ppRebuildServiceDRuntimeBlock(const PairHelperRefs& refs);
void ppIncrementServiceDRuntimeTailCounter(const PairHelperRefs& refs);
void ppCommitPersistedServiceDSelectorStateFromAuth36(const PairHelperRefs& refs);
void ppCommitPersistedServiceDRecord1AFromAuth28(const PairHelperRefs& refs);
uint8_t ppReadyStateCode(PairingStage stage);
const char* ppPairingStageLabel(PairingStage stage);
