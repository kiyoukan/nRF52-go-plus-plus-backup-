#pragma once

#include <stddef.h>
#include <stdint.h>

#include <bluefruit.h>

#include "pp_types.h"

struct FirstPairRuntimeRefs {
  uint16_t* activeConnHandle;
  PairingStage* pairingStage;
  bool* persistentReconnectReady;
  bool* serviceCNotifyEnabled;
  bool* buttonNotifyEnabled;
  bool* authControlNotifyEnabled;
  bool* authBlobNotifyEnabled;
  BLECharacteristic* serviceCChar1;
  BLECharacteristic* buttonNotifyChar;
  BLECharacteristic* authControl;
  BLECharacteristic* authBlob;
  DonorAuthRuntimeState* donorAuthRuntime;
  AuthParserOutputState* authParserOutput;
  uint8_t* serviceCState;
  uint8_t* buttonState;
  uint8_t* candidatePair;
  uint8_t* activePair;
  uint8_t* stagedPair;
  uint8_t* currentAttemptCentralNonce;
  uint8_t* persistedAcceptedCentralNonce;
  uint8_t* plusplusManufacturerData;
  bool* stagedPairValid;
  bool* currentAttemptCentralNonceValid;
  bool* persistedAcceptedCentralNonceValid;
  bool* freshAuthStarted;
  bool* freshAuthBlobSent;
  uint8_t* serviceCBurstStep;
  uint32_t* serviceCBurstAt;
  uint32_t* connStartedAt;
  uint8_t* freshAuthStep;
  uint32_t* freshAuthAt;
  bool* firstPairSawReal36;
  bool* firstPairSawReal28;
  uint8_t* serviceCTransferState;
  uint8_t* pairEventClass;
  const uint8_t* freshReply24;
  size_t freshReply24Len;
  const uint8_t* authReply25;
  size_t authReply25Len;
  const uint8_t* authReply18;
  size_t authReply18Len;
  const uint8_t* authReply18Fail;
  size_t authReply18FailLen;
  const uint8_t* authReply16;
  size_t authReply16Len;
  const uint8_t* authReply21Wait;
  size_t authReply21WaitLen;
  const uint8_t* authReply21Ready;
  size_t authReply21ReadyLen;
  const uint8_t* authReply36;
  size_t authReply36Len;
  const uint8_t* authReply28;
  size_t authReply28Len;
  const uint8_t* authReply26;
  size_t authReply26Len;
  const uint8_t* authBlobShort;
  size_t authBlobShortLen;
  size_t serviceCNotifyLen;
  const uint8_t* realPlusPlusAddrLe;
  size_t realPlusPlusAddrLeLen;
  const uint8_t* factoryCertBlock;
  size_t factoryCertBlockLen;
  uint32_t freshAuthDelayMs;
  uint32_t firstPairServiceCNotify1DelayMs;
  uint32_t firstPairServiceCNotify2DelayMs;
  uint32_t firstPairServiceCNotify3DelayMs;
  uint32_t freshAuth18To16DelayMs;
  uint32_t freshAuth18ToReplyDelayMs;
  uint32_t freshAuthWaitFor28WriteMs;
  uint32_t freshAuthStepDelayMs;
  void (*stageCandidatePair)(const char* reason);
  void (*commitStagedPair)(const char* reason);
  void (*buildIndexedAuthReply24)(uint8_t index, uint8_t* out5, const char* reason);
  void (*updateAuthReplyIndexSlot)(uint8_t index, const uint8_t* value3, const char* reason);
  void (*refreshServiceDLocalPayloadsFromRuntime)();
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
  void (*setFirstPairCertState)(uint8_t nextState, const char* reason);
  void (*scheduleFreshAuthSequence)(uint8_t firstStep, const char* reason);
  void (*logDonorAuthRuntime)(const char* reason);
  uint8_t (*donorLocal29CommandCodeForFamily)(uint8_t family);
  void (*queueDonorAuthRuntimeStep)(uint8_t nextStep, uint32_t delayMs, const char* reason);
  void (*commitPersistedServiceDSelectorStateFromAuth36)();
  void (*commitPersistedServiceDRecord1AFromAuth28)();
  void (*clearReconnectServiceDArm)();
  void (*clearPostServiceDHandoff)();
  void (*refreshReconnectAuthReply24)(const char* reason);
  void (*armPostAuthRuntimeScaffolding)();
  bool (*savePersistentPairState)(const char* reason);
  void (*refreshPairingExports)(bool forceNotify);
  void (*buildGeneratedInitialAuthBlob)();
  void (*notifyStagedAuthBlob106)(const char* reason);
  void (*setManufacturerState)(uint8_t state);
  const char* (*pairingStageLabel)(PairingStage stage);
  uint8_t (*readyStateCode)();
  uint8_t (*deriveDonorLocalFamilyFromParserBytes)();
  void (*resetAuthParserOutput)();
  bool (*processReconnectCompactCompletion)();
  void (*scheduleServiceCFirstPairBurst)();
};

bool ppHandleFirstPairAuthWrite(const FirstPairRuntimeRefs& refs,
                                const uint8_t* data,
                                uint16_t len);
void ppStageCandidatePair(const FirstPairRuntimeRefs& refs, const char* reason);
void ppCommitStagedPair(const FirstPairRuntimeRefs& refs, const char* reason);
void ppMarkAuthComplete(const FirstPairRuntimeRefs& refs);
void ppScheduleFreshAuthSequence(const FirstPairRuntimeRefs& refs,
                                 uint8_t firstStep,
                                 const char* reason);
void ppProcessFreshAuthSequence(const FirstPairRuntimeRefs& refs);
void ppScheduleServiceCFirstPairBurst(const FirstPairRuntimeRefs& refs);
void ppProcessServiceCFirstPairBurst(const FirstPairRuntimeRefs& refs);

struct FirstPairOlderServiceDRefs {
  ServiceDCertState* serviceDCertState;
  bool* lastOlderServiceDType2WriteValid;
  uint8_t* lastOlderServiceDType2Write;
  size_t lastOlderServiceDType2WriteLen;
  uint8_t* freshAuthStep;
  uint32_t* freshAuthAt;
  bool* reconnectServiceDArmed;
  bool* reconnectCompactServiceDMode;
  void (*refreshServiceDLocalPayloadsFromRuntime)();
  void (*prepareOlderServiceDReadStage)(uint8_t stage);
  void (*notifyServiceDStatus)(const uint8_t* status, const char* label);
  void (*armServiceDCertExpectation)(uint8_t state, uint8_t expectedType, uint16_t expectedLen, const char* reason);
  void (*incrementServiceDRuntimeTailCounter)();
  void (*logServiceDCertState)(const char* reason);
};

void ppBeginFirstPairOlderServiceDFlow(const FirstPairOlderServiceDRefs& refs,
                                       const uint8_t* status0);
bool ppHandleFirstPairOlderServiceDWrite(const FirstPairOlderServiceDRefs& refs,
                                         const uint8_t* data,
                                         uint16_t len,
                                         const uint8_t* status1,
                                         const uint8_t* status2,
                                         const uint8_t* status4,
                                         uint32_t waitForWriteMs);
