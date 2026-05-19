#pragma once

#include <bluefruit.h>

#include "pp_types.h"
#include "pp_sleep_export.h"

struct BleDispatchRefs {
  PairingStage* pairingStage;
  bool* buttonNotifyEnabled;
  bool* serviceCNotifyEnabled;
  bool* serviceBNotifyEnabled;
  bool* serviceDNotifyEnabled;
  bool* authControlNotifyEnabled;
  bool* authBlobNotifyEnabled;
  bool* freshAuthStarted;
  uint8_t* freshAuthStep;
  uint32_t* freshAuthAt;
  uint8_t* sfidaCertStage;
  bool* reconnectServiceDArmed;
  bool* persistentReconnectReady;
  bool* freshAuthBlobSent;
  bool* postServiceDHandoffActive;
  bool* postServiceDServiceBReadSeen;
  uint8_t* postServiceDServiceBValue;
  uint8_t* serviceCTransferState;
  uint8_t* pairEventClass;
  uint8_t* serviceCState;
  uint16_t serviceCStateLen;
  uint8_t* serviceDStatus;
  uint16_t serviceDStatusLen;
  uint8_t* configState;
  uint16_t configStateLen;
  const uint8_t* freshAuthInitialChallenge;
  uint16_t freshAuthInitialChallengeLen;
  uint8_t* authBlobVisibleValue;
  uint16_t authBlobVisibleLen;
  AuthBlobStageState* authBlobStage;
  AuthParserOutputState* authParserOutput;
  ServiceDCertState* serviceDCertState;
  BLECharacteristic* batteryLevelChar;
  BLECharacteristic* serviceAChar1;
  BLECharacteristic* serviceAChar2;
  BLECharacteristic* serviceAChar3;
  BLECharacteristic* serviceAChar4;
  BLECharacteristic* serviceAChar5;
  BLECharacteristic* serviceBChar1;
  BLECharacteristic* serviceBChar2;
  BLECharacteristic* serviceCChar1;
  BLECharacteristic* serviceCChar2;
  BLECharacteristic* serviceDChar1;
  BLECharacteristic* serviceDChar2;
  BLECharacteristic* serviceDChar3;
  BLECharacteristic* ledWriteChar;
  BLECharacteristic* buttonNotifyChar;
  BLECharacteristic* serviceEChar3;
  BLECharacteristic* serviceEChar4;
  BLECharacteristic* versionChar;
  BLECharacteristic* configChar;
  BLECharacteristic* authControl;
  BLECharacteristic* authBlob;
  const uint8_t* authCtrlReply25;
  uint16_t authCtrlReply25Len;
  uint8_t configResetPairedStateMagic;
  uint16_t shortValueLen;
  uint16_t mediumValueLen;
  uint32_t freshAuthWaitFor28WriteMs;
  const char* (*pairingStageLabel)(PairingStage stage);
  void (*printHex)(const uint8_t* data, uint16_t len);
  void (*refreshPairingExports)(bool notify);
  void (*stageCandidatePair)(const char* reason);
  void (*commitStagedPair)(const char* reason);
  void (*scheduleServiceCFirstPairBurst)();
  void (*beginServiceDOlderFlow)();
  void (*beginServiceDCertFlow)();
  bool (*handleServiceDCertWrite)(const uint8_t* data, uint16_t len);
  void (*stopPostAuthRuntimeScaffolding)();
  void (*maybeTriggerButtonFromLedPayload)(const uint8_t* data, uint16_t len);
  void (*resetPairingFromApp)(const char* reason);
  bool (*plusplusGenerateReconnectResponse)(const uint8_t* challenge32, uint8_t* response16);
  bool (*handleFirstPairAuthWrite)(const uint8_t* data, uint16_t len);
  void (*handleReconnectAuthWrite24)(uint8_t selector, const char* reason);
  void (*handleReconnectAuthWrite25)(const uint8_t* reply, uint16_t replyLen, const char* reason);
  SleepExportWriteResult (*handleAuthenticatedSleepWrite)(const uint8_t* data, uint16_t len);
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

void ppCccdWriteCallback(const BleDispatchRefs& refs,
                         uint16_t conn_handle,
                         BLECharacteristic* chr,
                         uint16_t value);
void ppReadAuthorizeCallback(const BleDispatchRefs& refs,
                             uint16_t conn_handle,
                             BLECharacteristic* chr,
                             ble_gatts_evt_read_t* request);
void ppServiceWriteCallback(const BleDispatchRefs& refs,
                            uint16_t conn_handle,
                            BLECharacteristic* chr,
                            uint8_t* data,
                            uint16_t len);
