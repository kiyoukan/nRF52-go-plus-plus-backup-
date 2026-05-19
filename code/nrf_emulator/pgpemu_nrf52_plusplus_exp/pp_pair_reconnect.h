#pragma once

#include <stddef.h>
#include <stdint.h>

#include <bluefruit.h>

#include "pp_types.h"

struct ReconnectRuntimeRefs {
  uint16_t* activeConnHandle;
  PairingStage* pairingStage;
  bool* buttonNotifyEnabled;
  bool* authBlobNotifyEnabled;
  bool* persistentReconnectReady;
  bool* persistedAcceptedCentralNonceValid;
  uint8_t* persistedAcceptedCentralNonce;
  uint8_t* reconnectAuthReply24;
  size_t reconnectAuthReply24Len;
  bool* freshAuthBlobSent;
  bool* reconnectServiceDArmed;
  BLECharacteristic* authControl;
  void (*updateAuthReplyIndexSlot)(uint8_t index, const uint8_t* value3, const char* reason);
  void (*buildIndexedAuthReply24)(uint8_t index, uint8_t* out5, const char* reason);
  void (*printHex)(const uint8_t* data, uint16_t len);
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
  void (*buildDonorReconnectAuthBlob)();
  void (*notifyStagedAuthBlob106)(const char* reason);
  void (*markAuthComplete)();
};

struct ReconnectCompactServiceDRefs {
  bool* reconnectCompactServiceDMode;
  bool* reconnectServiceDArmed;
  bool* serviceBNotifyEnabled;
  bool* postServiceDHandoffActive;
  bool* postServiceDServiceBReadSeen;
  uint8_t* postServiceDServiceBValue;
  ServiceDCertState* serviceDCertState;
  bool* lastOlderServiceDType2WriteValid;
  uint8_t* freshAuthStep;
  uint32_t* freshAuthAt;
  PairingStage* pairingStage;
  BLECharacteristic* serviceBChar1;
  void (*refreshServiceDLocalPayloadsFromRuntime)();
  void (*prepareSfidaReadChallenge)(uint8_t stage);
  void (*notifyServiceDStatus)(const uint8_t* status, const char* label);
  void (*armServiceDCertExpectation)(uint8_t state, uint8_t expectedType, uint16_t expectedLen, const char* reason);
  void (*buildServiceDType5PayloadFromLiveWrite)(const uint8_t* write36);
  void (*resetServiceDCertState)();
  void (*logServiceDCertState)(const char* reason);
};

void ppRefreshReconnectAuthReply24(const ReconnectRuntimeRefs& refs, const char* reason);
void ppHandleReconnectAuthWrite24(const ReconnectRuntimeRefs& refs,
                                  uint8_t selectorIndex,
                                  const char* reason);
void ppHandleReconnectAuthWrite25(const ReconnectRuntimeRefs& refs,
                                  const uint8_t* authReply25,
                                  size_t authReply25Len,
                                  const char* blobReason);
void ppBeginReconnectCompactServiceDFlow(const ReconnectCompactServiceDRefs& refs,
                                         const uint8_t* statusStart,
                                         uint32_t waitForWriteMs);
bool ppHandleReconnectCompactServiceDWrite(const ReconnectCompactServiceDRefs& refs,
                                           const uint8_t* data,
                                           uint16_t len,
                                           const uint8_t* expectedWrite3Done,
                                           size_t expectedWrite3DoneLen,
                                           const uint8_t* statusStage1,
                                           const uint8_t* statusStage2,
                                           const uint8_t* statusDone,
                                           const uint8_t* statusFail,
                                           uint32_t waitForWriteMs);
bool ppProcessReconnectCompactCompletion(const ReconnectCompactServiceDRefs& refs);
