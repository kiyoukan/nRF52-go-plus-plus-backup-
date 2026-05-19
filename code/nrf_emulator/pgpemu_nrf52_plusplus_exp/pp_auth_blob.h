#pragma once

#include <stddef.h>
#include <stdint.h>

#include <bluefruit.h>

#include "pp_types.h"

struct AuthBlobRuntimeRefs {
  AuthBlobStageState* authBlobStage;
  AuthParserOutputState* authParserOutput;
  BLECharacteristic* authBlob;
  uint8_t* authBlobVisibleValue;
  size_t authBlobVisibleValueLen;
  void (*setAuthParserSchedulerState)(uint8_t schedulerState,
                                      uint8_t outputClass,
                                      uint16_t outputOffset,
                                      uint16_t outputLen,
                                      uint8_t resultCode,
                                      uint8_t continuationKind,
                                      uint8_t parserOpcode,
                                      bool outputPending,
                                      bool streamPending,
                                      bool notifyInFlight);
};

void ppClearAuthBlobStage(const AuthBlobRuntimeRefs& refs);
void ppStageVisibleAuthBlob(const AuthBlobRuntimeRefs& refs,
                            const uint8_t* payload,
                            uint16_t payloadLen,
                            uint8_t stageClass,
                            uint8_t lastOpcode);
void ppStageVisibleAuthBlob106(const AuthBlobRuntimeRefs& refs);
void ppNotifyStagedAuthBlob106(const AuthBlobRuntimeRefs& refs, const char* reason);
