#pragma once

#include <stddef.h>
#include <stdint.h>

#include <bluefruit.h>

#include "pp_types.h"

struct PairStateRuntimeRefs {
  ServiceDCertState* serviceDCertState;
  bool* reconnectServiceDArmed;
  bool* reconnectCompactServiceDMode;
  uint8_t* persistedServiceDSelector4;
  bool* persistedServiceDSelector4Valid;
  uint8_t* persistedServiceDSelector5;
  bool* persistedServiceDSelector5Valid;
  uint8_t* persistedServiceDRecord6;
  bool* persistedServiceDRecord6Valid;
  uint32_t* persistedServiceDRecord1A;
  bool* persistedServiceDRecord1AValid;
  uint8_t* serviceDRuntimeBlock;
  uint8_t* liveCertKey;
  const uint8_t* selector3;
  size_t selector3Len;
  const uint8_t* factorySelector6;
  size_t factorySelector6Len;
  PairingStage* pairingStage;
  uint8_t* pairEventClass;
  uint8_t* candidatePair;
  uint8_t* activePair;
  uint8_t* stagedPair;
  bool* stagedPairValid;
  uint32_t* connStartedAt;
  uint8_t* serviceCTransferState;
  uint8_t* serviceCBurstStep;
  uint32_t* serviceCBurstAt;
  bool* serviceCNotifyEnabled;
  bool* serviceBNotifyEnabled;
  bool* buttonNotifyEnabled;
  bool* serviceDNotifyEnabled;
  bool* authControlNotifyEnabled;
  bool* authBlobNotifyEnabled;
  bool* postServiceDHandoffActive;
  bool* postServiceDServiceBReadSeen;
  uint8_t* postServiceDServiceBValue;
  uint8_t* freshAuthStep;
  uint32_t* freshAuthAt;
  bool* freshAuthStarted;
  bool* freshAuthBlobSent;
  uint8_t* sfidaCertStage;
  bool* userCccdsSeeded;
  bool* persistentReconnectReady;
  uint8_t* reconnectAuthReply24;
  size_t reconnectAuthReply24Len;
  const uint8_t* defaultReply2401;
  size_t defaultReply2401Len;
  uint8_t* currentAttemptCentralNonce;
  bool* currentAttemptCentralNonceValid;
  uint8_t* persistedAcceptedCentralNonce;
  bool* persistedAcceptedCentralNonceValid;
  bool* firstPairSawReal36;
  bool* firstPairSawReal28;
  uint8_t* firstPairCertState;
  uint8_t* manufacturerData;
  size_t manufacturerDataLen;
  uint8_t* serviceARead3;
  uint8_t* serviceARead4;
  uint8_t* serviceAStatus;
  uint8_t* serviceBStatus1;
  uint8_t* serviceBStatus2;
  uint8_t* serviceBRead3;
  uint8_t* serviceCState;
  uint8_t* serviceDStatus;
  uint8_t* configState;
  bool* servicesReady;
  uint16_t* activeConnHandle;
  BLECharacteristic* serviceAChar3;
  BLECharacteristic* serviceAChar4;
  BLECharacteristic* serviceAChar5;
  BLECharacteristic* serviceBChar1;
  BLECharacteristic* serviceBChar2;
  BLECharacteristic* serviceBChar3;
  BLECharacteristic* serviceCChar1;
  BLECharacteristic* serviceDChar2;
  BLECharacteristic* configChar;
  void (*resetReconnectSleepDataState)();
  void (*resetRuntimeButtonEngine)();
  void (*stopPostAuthRuntimeScaffolding)();
  void (*resetDonorAuthRuntime)();
  void (*seedAuthReplyIndexTable)();
  void (*clearAuthBlobStage)();
  void (*resetAuthParserOutput)();
  void (*setManufacturerState)(uint8_t state);
  void (*refreshPairingExports)(bool forceNotify);
  void (*copyAddressDisplay)(uint8_t* dest);
  uint8_t (*readyStateCode)();
};

void ppResetServiceDCertState(const PairStateRuntimeRefs& refs);
void ppClearReconnectServiceDArm(const PairStateRuntimeRefs& refs);
void ppResetPersistedServiceDState(const PairStateRuntimeRefs& refs);
void ppResetVolatilePairSessionState(const PairStateRuntimeRefs& refs);
void ppResetUnpairedState(const PairStateRuntimeRefs& refs);
void ppSetManufacturerState(const PairStateRuntimeRefs& refs, uint8_t state);
void ppRefreshPairingExports(const PairStateRuntimeRefs& refs, bool notify);
