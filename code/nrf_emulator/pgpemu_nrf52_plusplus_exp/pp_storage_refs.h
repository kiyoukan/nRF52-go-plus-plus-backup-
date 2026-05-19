#pragma once

// Header-included storage and pair-state composition builders. These bind
// file-local firmware state from the .ino translation unit.

// ---- PersistedPairRuntimeRefs ----
static PersistedPairRuntimeRefs makePersistedPairRuntimeRefs()
{
  PersistedPairRuntimeRefs refs = {
    &persistentStoreReady,
    &persistentReconnectReady,
    &persistedAcceptedCentralNonceValid,
    persistedAcceptedCentralNonce,
    &persistedServiceDSelector4Valid,
    persistedServiceDSelector4,
    &persistedServiceDSelector5Valid,
    &persistedServiceDSelector5,
    &persistedServiceDRecord6Valid,
    persistedServiceDRecord6,
    &persistedServiceDRecord1AValid,
    &persistedServiceDRecord1A,
    &pairingStage,
    &pairEventClass,
    &firstPairSawReal36,
    &firstPairSawReal28,
    rebuildServiceDRuntimeBlock,
    refreshReconnectAuthReply24,
    setManufacturerState,
    refreshPairingExports
  };
  return refs;
}


// ---- Pair-state wrappers ----
static void copyAddressDisplay(uint8_t* dest)
{
  ppCopyAddressDisplay(makePairHelperRefs(), dest);
}

static void resetServiceDCertState()
{
  ppResetServiceDCertState(makePairStateRuntimeRefs());
}

static void clearReconnectServiceDArm()
{
  ppClearReconnectServiceDArm(makePairStateRuntimeRefs());
}

// ---- Persistence wrappers ----
static bool savePersistentPairState(const char* reason)
{
  return ppSavePersistentPairState(makePersistedPairRuntimeRefs(), reason);
}

static bool loadPersistentPairState()
{
  return ppLoadPersistentPairState(makePersistedPairRuntimeRefs());
}

static bool beginPersistentStore()
{
  return InternalFS.begin();
}

static void clearPersistentPairState(const char* reason)
{
  ppClearPersistentPairState(makePersistedPairRuntimeRefs(), reason);
}

static void restorePairedStateFromPersistence()
{
  ppRestorePairedStateFromPersistence(makePersistedPairRuntimeRefs());
}

// ---- PairStateRuntimeRefs ----
static PairStateRuntimeRefs makePairStateRuntimeRefs()
{
  PairStateRuntimeRefs refs = {
    &serviceDCertState,
    &reconnectServiceDArmed,
    &reconnectCompactServiceDMode,
    persistedServiceDSelector4,
    &persistedServiceDSelector4Valid,
    &persistedServiceDSelector5,
    &persistedServiceDSelector5Valid,
    persistedServiceDRecord6,
    &persistedServiceDRecord6Valid,
    &persistedServiceDRecord1A,
    &persistedServiceDRecord1AValid,
    serviceDRuntimeBlock,
    liveCertKey,
    PLUSPLUS_FACTORY_SELECTOR3,
    sizeof(PLUSPLUS_FACTORY_SELECTOR3),
    PLUSPLUS_FACTORY_SELECTOR6,
    sizeof(PLUSPLUS_FACTORY_SELECTOR6),
    &pairingStage,
    &pairEventClass,
    candidatePair,
    activePair,
    stagedPair,
    &stagedPairValid,
    &connStartedAt,
    &serviceCTransferState,
    &serviceCBurstStep,
    &serviceCBurstAt,
    &serviceCNotifyEnabled,
    &serviceBNotifyEnabled,
    &buttonNotifyEnabled,
    &serviceDNotifyEnabled,
    &authControlNotifyEnabled,
    &authBlobNotifyEnabled,
    &postServiceDHandoffActive,
    &postServiceDServiceBReadSeen,
    &postServiceDServiceBValue,
    &freshAuthStep,
    &freshAuthAt,
    &freshAuthStarted,
    &freshAuthBlobSent,
    &sfidaCertStage,
    &userCccdsSeeded,
    &persistentReconnectReady,
    reconnectAuthReply24,
    sizeof(reconnectAuthReply24),
    PLUSPLUS_AUTH_CTRL_REPLY_2401,
    sizeof(PLUSPLUS_AUTH_CTRL_REPLY_2401),
    currentAttemptCentralNonce,
    &currentAttemptCentralNonceValid,
    persistedAcceptedCentralNonce,
    &persistedAcceptedCentralNonceValid,
    &firstPairSawReal36,
    &firstPairSawReal28,
    &firstPairCertState,
    plusplusManufacturerData,
    sizeof(plusplusManufacturerData),
    serviceARead3,
    serviceARead4,
    serviceAStatus,
    serviceBStatus1,
    serviceBStatus2,
    serviceBRead3,
    serviceCState,
    serviceDStatus,
    configState,
    &servicesReady,
    &activeConnHandle,
    &serviceAChar3,
    &serviceAChar4,
    &serviceAChar5,
    &serviceBChar1,
    &serviceBChar2,
    &serviceBChar3,
    &serviceCChar1,
    &serviceDChar2,
    &configChar,
    resetReconnectSleepDataState,
    resetRuntimeButtonEngine,
    stopPostAuthRuntimeScaffolding,
    resetDonorAuthRuntime,
    seedAuthReplyIndexTable,
    clearAuthBlobStage,
    resetAuthParserOutput,
    setManufacturerState,
    refreshPairingExports,
    copyAddressDisplay,
    readyStateCode
  };
  return refs;
}

// ---- Pair-state export wrappers ----
static void setManufacturerState(uint8_t state)
{
  ppSetManufacturerState(makePairStateRuntimeRefs(), state);
}

static void refreshPairingExports(bool notify = false)
{
  ppRefreshPairingExports(makePairStateRuntimeRefs(), notify);
}

// ---- Reset unpaired wrapper ----
static void resetUnpairedState()
{
  ppResetUnpairedState(makePairStateRuntimeRefs());
}

