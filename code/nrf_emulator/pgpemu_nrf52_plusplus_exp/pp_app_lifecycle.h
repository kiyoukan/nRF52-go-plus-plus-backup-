#pragma once

#include <Arduino.h>

#include "pp_types.h"

class BLECharacteristic;

struct AppLifecycleRefs {
  uint16_t* activeConnHandle;
  PairingStage* pairingStage;
  bool* persistentReconnectReady;
  bool* disconnectedSleepArmed;
  uint32_t* disconnectedSleepAt;
  uint32_t disconnectedSleepTimeoutMs;
  bool* physicalResetButtonActive;
  bool* physicalResetButtonTriggered;
  uint32_t* physicalResetButtonPressedAt;
  uint32_t physicalResetButtonRawPin;
  uint32_t physicalResetButtonHoldMs;
  bool* buttonPulseActive;
  uint8_t* buttonState;
  uint16_t buttonStateLen;
  uint8_t* serviceAStatus;
  uint16_t serviceAStatusLen;
  uint32_t* connStartedAt;
  bool* serviceCNotifyEnabled;
  bool* serviceBNotifyEnabled;
  bool* buttonNotifyEnabled;
  bool* serviceDNotifyEnabled;
  bool* authControlNotifyEnabled;
  bool* authBlobNotifyEnabled;
  uint8_t manufacturerReconnectState;
  BLECharacteristic* batteryLevelChar;
  uint8_t batteryLevelDefault;
  void (*prepareReconnectSleepDataOnAuthenticatedDisconnect)();
  void (*resetRuntimeButtonEngine)();
  void (*resetVolatilePairSessionState)();
  void (*stopPostAuthRuntimeScaffolding)();
  void (*setManufacturerState)(uint8_t state);
  void (*refreshPairingExports)(bool notify);
  void (*resetUnpairedState)();
  void (*clearPersistentPairState)(const char* reason);
  void (*startAdvertising)();
  void (*printBuildTag)(const char* reason);
  const char* (*pairingStageLabel)(PairingStage stage);
};

struct AppBootRefs {
  bool* servicesReady;
  bool* persistentStoreReady;
  const char* deviceName;
  const char* bootBanner;
  uint32_t physicalResetButtonRawPin;
  void (*printBuildTag)(const char* reason);
  void (*seedLongReadBlob)();
  void (*seedAuthBlob)();
  void (*configureIdentity)();
  void (*configureGapPreferences)();
  void (*setupServices)();
  bool (*beginPersistentStore)();
  void (*resetUnpairedState)();
  bool (*loadPersistentPairState)();
  void (*restorePairedStateFromPersistence)();
  void (*startAdvertising)();
  void (*armDisconnectedSleepTimer)(const char* reason);
  void (*connectCallback)(uint16_t conn_handle);
  void (*disconnectCallback)(uint16_t conn_handle, uint8_t reason);
};

struct AppLoopRefs {
  void (*processPhysicalResetButtonIfNeeded)();
  void (*processDisconnectedSleepIfNeeded)();
  void (*processAuthenticatedSleepStream)();
  void (*clearButtonPulseIfNeeded)();
  void (*processRuntimeButtonEngineIfNeeded)();
  void (*processPostAuthIdleButtonNotify)();
  void (*processPostAuthReady18)();
  void (*processServiceCFirstPairBurst)();
  void (*processFreshAuthSequence)();
  uint32_t loopDelayMs;
};

void ppResetPairingFromApp(const AppLifecycleRefs& refs, const char* reason);
void ppResetPairingFromPhysicalButton(const AppLifecycleRefs& refs, const char* reason);
void ppDisarmDisconnectedSleepTimer(const AppLifecycleRefs& refs);
void ppArmDisconnectedSleepTimer(const AppLifecycleRefs& refs, const char* reason);
void ppProcessDisconnectedSleepIfNeeded(const AppLifecycleRefs& refs);
void ppProcessPhysicalResetButtonIfNeeded(const AppLifecycleRefs& refs);
void ppHandleConnect(const AppLifecycleRefs& refs, uint16_t conn_handle);
void ppHandleDisconnect(const AppLifecycleRefs& refs, uint16_t conn_handle, uint8_t reason);
void ppRunBootSequence(const AppBootRefs& refs);
void ppRunMainLoopTick(const AppLoopRefs& refs);
