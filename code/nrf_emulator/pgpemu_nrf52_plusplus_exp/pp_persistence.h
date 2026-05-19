#pragma once

#include "pp_types.h"

struct PersistedPairRuntimeRefs {
  bool* persistentStoreReady;
  bool* persistentReconnectReady;
  bool* persistedAcceptedCentralNonceValid;
  uint8_t* persistedAcceptedCentralNonce;
  bool* persistedServiceDSelector4Valid;
  uint8_t* persistedServiceDSelector4;
  bool* persistedServiceDSelector5Valid;
  uint8_t* persistedServiceDSelector5;
  bool* persistedServiceDRecord6Valid;
  uint8_t* persistedServiceDRecord6;
  bool* persistedServiceDRecord1AValid;
  uint32_t* persistedServiceDRecord1A;
  PairingStage* pairingStage;
  uint8_t* pairEventClass;
  bool* firstPairSawReal36;
  bool* firstPairSawReal28;
  void (*rebuildServiceDRuntimeBlock)();
  void (*refreshReconnectAuthReply24)(const char* reason);
  void (*setManufacturerState)(uint8_t state);
  void (*refreshPairingExports)(bool notify);
};

bool ppSavePersistentPairState(const PersistedPairRuntimeRefs& refs, const char* reason);
bool ppLoadPersistentPairState(const PersistedPairRuntimeRefs& refs);
void ppClearPersistentPairState(const PersistedPairRuntimeRefs& refs, const char* reason);
void ppRestorePairedStateFromPersistence(const PersistedPairRuntimeRefs& refs);
