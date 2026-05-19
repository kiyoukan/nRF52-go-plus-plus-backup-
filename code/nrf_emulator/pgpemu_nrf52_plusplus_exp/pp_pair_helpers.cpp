#include <Arduino.h>
#include <string.h>

#include "pp_pair_helpers.h"

void ppCopyAddressDisplay(const PairHelperRefs& refs, uint8_t* dest)
{
  for (size_t i = 0; i < refs.realPlusPlusAddrLen; ++i) {
    dest[i] = refs.realPlusPlusAddrLe[refs.realPlusPlusAddrLen - 1 - i];
  }
}

void ppLogServiceDCertState(const PairHelperRefs& refs, const char* reason)
{
  Serial.print("ServiceD cert via ");
  Serial.print(reason);
  Serial.print(": active=");
  Serial.print(refs.serviceDCertState->active ? "true" : "false");
  Serial.print(" lane=");
  Serial.print(refs.serviceDCertState->laneKind);
  Serial.print(" step=");
  Serial.print(refs.serviceDCertState->laneStep);
  Serial.print(" expectType=");
  Serial.print(refs.serviceDCertState->expectedWriteType);
  Serial.print(" expectLen=0x");
  Serial.print(refs.serviceDCertState->expectedWriteLen, HEX);
  Serial.print(" lastType=");
  Serial.print(refs.serviceDCertState->lastObservedType);
  Serial.print(" lastLen=0x");
  Serial.print(refs.serviceDCertState->lastObservedLen, HEX);
  Serial.print(" completion=");
  Serial.print(refs.serviceDCertState->completionPending ? "true" : "false");
  Serial.println();
}

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
                                   bool notifyInFlight)
{
  refs.authParserOutput->schedulerState = schedulerState;
  refs.authParserOutput->outputClass = outputClass;
  refs.authParserOutput->outputLen = outputLen;
  refs.authParserOutput->outputOffset = outputOffset;
  refs.authParserOutput->resultCode = resultCode;
  refs.authParserOutput->continuationKind = continuationKind;
  refs.authParserOutput->lastOpcode = lastOpcode;
  refs.authParserOutput->outputPending = outputPending;
  refs.authParserOutput->streamPending = streamPending;
  refs.authParserOutput->notifyInFlight = notifyInFlight;
}

void ppRebuildServiceDRuntimeBlock(const PairHelperRefs& refs)
{
  memset(refs.serviceDRuntimeBlock, 0, refs.serviceDRuntimeBlockLen);
  if (*refs.persistedServiceDSelector4Valid) {
    memcpy(refs.serviceDRuntimeBlock + 0x00,
           refs.persistedServiceDSelector4,
           refs.persistedServiceDSelector4Len);
  }
  if (*refs.persistedServiceDSelector5Valid) {
    refs.serviceDRuntimeBlock[0x10] = *refs.persistedServiceDSelector5;
  }
  if (*refs.persistedServiceDRecord6Valid) {
    memcpy(refs.serviceDRuntimeBlock + 0x11,
           refs.persistedServiceDRecord6,
           refs.persistedServiceDRecord6Len);
  }
}

void ppIncrementServiceDRuntimeTailCounter(const PairHelperRefs& refs)
{
  uint32_t low = 0;
  uint32_t high = 0;
  memcpy(&low, refs.serviceDRuntimeBlock + 0x11, sizeof(low));
  memcpy(&high, refs.serviceDRuntimeBlock + 0x15, sizeof(high));
  ++low;
  if (low == 0) {
    ++high;
  }
  memcpy(refs.serviceDRuntimeBlock + 0x11, &low, sizeof(low));
  memcpy(refs.serviceDRuntimeBlock + 0x15, &high, sizeof(high));
  memcpy(refs.persistedServiceDRecord6,
         refs.serviceDRuntimeBlock + 0x11,
         refs.persistedServiceDRecord6Len);
}

void ppCommitPersistedServiceDSelectorStateFromAuth36(const PairHelperRefs& refs)
{
  memcpy(refs.persistedServiceDSelector4,
         refs.firmwareRecord4Default,
         refs.persistedServiceDSelector4Len);
  *refs.persistedServiceDSelector4Valid = true;
  *refs.persistedServiceDSelector5 = 0x00;
  *refs.persistedServiceDSelector5Valid = true;
  ppRebuildServiceDRuntimeBlock(refs);
  Serial.print("Persisted ServiceD selector4 via auth 36 = ");
  refs.printHex(refs.persistedServiceDSelector4,
                static_cast<uint16_t>(refs.persistedServiceDSelector4Len));
  Serial.print(" selector5=0x");
  Serial.println(*refs.persistedServiceDSelector5, HEX);
  Serial.print(" selector6/runtime tail=");
  refs.printHex(refs.persistedServiceDRecord6,
                static_cast<uint16_t>(refs.persistedServiceDRecord6Len));
  Serial.println();
}

void ppCommitPersistedServiceDRecord1AFromAuth28(const PairHelperRefs& refs)
{
  *refs.persistedServiceDRecord1A = 1;
  *refs.persistedServiceDRecord1AValid = true;
  Serial.println("Persisted ServiceD record 0x1a via auth 28 = 0x00000001");
}

uint8_t ppReadyStateCode(PairingStage stage)
{
  switch (stage) {
    case PAIR_STAGE_AUTHENTICATED:
    case PAIR_STAGE_COMMITTED:
      return 0x01;
    default:
      return 0x00;
  }
}

const char* ppPairingStageLabel(PairingStage stage)
{
  switch (stage) {
    case PAIR_STAGE_DISCOVERY: return "discovery";
    case PAIR_STAGE_STAGED: return "staged";
    case PAIR_STAGE_COMMITTED: return "committed";
    case PAIR_STAGE_AUTHENTICATED: return "authenticated";
    default: return "unknown";
  }
}
