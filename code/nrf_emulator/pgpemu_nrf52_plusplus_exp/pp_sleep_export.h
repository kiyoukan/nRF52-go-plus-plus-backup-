#pragma once

#include <stddef.h>
#include <stdint.h>

#include <bluefruit.h>

#include "pp_types.h"

enum SleepExportWriteResult : uint8_t {
  SLEEP_EXPORT_NOT_HANDLED = 0,
  SLEEP_EXPORT_HANDLED_NO_STORE = 1,
  SLEEP_EXPORT_HANDLED_AND_STORE = 2
};

struct SleepExportRuntimeRefs {
  PairingStage* pairingStage;
  uint8_t* freshAuthStep;
  BLECharacteristic* authControl;
  BLECharacteristic* authBlob;
  uint16_t authBlobChunkMaxLen;
  const uint8_t* authReply26;
  size_t authReply26Len;
  const uint8_t* authBlobShort;
  size_t authBlobShortLen;
  void (*clearPostServiceDHandoff)();
  void (*setAuthParserSchedulerState)(uint8_t schedulerState,
                                      uint8_t outputClass,
                                      uint16_t outputLen,
                                      uint16_t outputOffset,
                                      uint8_t resultCode,
                                      uint8_t continuationKind,
                                      uint8_t parserOpcode,
                                      bool outputPending,
                                      bool streamPending,
                                      bool notifyInFlight);
};

SleepExportWriteResult ppHandleAuthenticatedSleepWrite(const SleepExportRuntimeRefs& refs,
                                                       const uint8_t* data,
                                                       uint16_t len);
void ppProcessAuthenticatedSleepStream(const SleepExportRuntimeRefs& refs);
void ppPrepareReconnectSleepDataForAuthenticatedDisconnect();
void ppResetReconnectSleepDataState();
void ppClearReconnectSleepPolicyState();
