#pragma once

// Header-included flow/runtime/auth composition. This keeps the existing
// .ino-local firmware state binding while pp_wiring.h remains a coordinator.

static void resetVolatilePairSessionStateDispatch()
{
  ppResetVolatilePairSessionState(makePairStateRuntimeRefs());
}

static AppLifecycleRefs makeAppLifecycleRefs()
{
  AppLifecycleRefs refs = {
    &activeConnHandle,
    &pairingStage,
    &persistentReconnectReady,
    &disconnectedSleepArmed,
    &disconnectedSleepAt,
    DISCONNECTED_SLEEP_TIMEOUT_MS,
    &physicalResetButtonActive,
    &physicalResetButtonTriggered,
    &physicalResetButtonPressedAt,
    PHYSICAL_RESET_BUTTON_RAW_PIN,
    PHYSICAL_RESET_BUTTON_HOLD_MS,
    &buttonPulseActive,
    buttonState,
    sizeof(buttonState),
    serviceAStatus,
    sizeof(serviceAStatus),
    &connStartedAt,
    &serviceCNotifyEnabled,
    &serviceBNotifyEnabled,
    &buttonNotifyEnabled,
    &serviceDNotifyEnabled,
    &authControlNotifyEnabled,
    &authBlobNotifyEnabled,
    MANUFACTURER_STATE_RECONNECT,
    &batteryLevelChar,
    BATTERY_LEVEL_DEFAULT,
    prepareReconnectSleepDataOnAuthenticatedDisconnect,
    resetRuntimeButtonEngine,
    resetVolatilePairSessionStateDispatch,
    stopPostAuthRuntimeScaffolding,
    setManufacturerState,
    refreshPairingExports,
    resetUnpairedState,
    clearPersistentPairState,
    start_advertising,
    print_build_tag,
    pairingStageLabel
  };
  return refs;
}

static AppBootRefs makeAppBootRefs()
{
  AppBootRefs refs = {
    &servicesReady,
    &persistentStoreReady,
    DEVICE_NAME,
    "=== nRF52 Plus+ prototype boot ===",
    PHYSICAL_RESET_BUTTON_RAW_PIN,
    print_build_tag,
    seedLongReadBlob,
    seedAuthBlob,
    configure_identity,
    configure_gap_preferences,
    setup_services,
    beginPersistentStore,
    resetUnpairedState,
    loadPersistentPairState,
    restorePairedStateFromPersistence,
    start_advertising,
    armDisconnectedSleepTimer,
    connect_callback,
    disconnect_callback
  };
  return refs;
}

static AppLoopRefs makeAppLoopRefs()
{
  AppLoopRefs refs = {
    processPhysicalResetButtonIfNeeded,
    processDisconnectedSleepIfNeeded,
    processAuthenticatedSleepStream,
    clearButtonPulseIfNeeded,
    processRuntimeButtonEngineIfNeeded,
    processPostAuthIdleButtonNotify,
    processPostAuthReady18,
    processServiceCFirstPairBurst,
    processFreshAuthSequence,
    10
  };
  return refs;
}

static bool handleOlderServiceDFlowWriteDispatch(const uint8_t* data, uint16_t len)
{
  return ppFirstPairFlowHandleOlderServiceDWrite(makeFirstPairOlderServiceDRefs(),
                                                 data,
                                                 len,
                                                 PLUSPLUS_SERVICE_D_STATUS1,
                                                 PLUSPLUS_SERVICE_D_STATUS2,
                                                 PLUSPLUS_SERVICE_D_STATUS4,
                                                 FRESH_AUTH_WAIT_FOR_28_WRITE_MS);
}

static bool handleReconnectServiceDFlowWriteDispatch(const uint8_t* data, uint16_t len)
{
  return ppReconnectFlowHandleCompactServiceDWrite(makeReconnectCompactServiceDRefs(),
                                                   data,
                                                   len,
                                                   DONOR_SERVICE_D_EXPECT_WRITE3_DONE,
                                                   sizeof(DONOR_SERVICE_D_EXPECT_WRITE3_DONE),
                                                   PLUSPLUS_SERVICE_D_STATUS4_STAGE1,
                                                   PLUSPLUS_SERVICE_D_STATUS5_STAGE2,
                                                   PLUSPLUS_SERVICE_D_STATUS4_DONE,
                                                   PLUSPLUS_SERVICE_D_STATUS4_FAIL,
                                                   FRESH_AUTH_WAIT_FOR_28_WRITE_MS);
}

static ServiceDLocalRefs makeServiceDLocalRefs()
{
  ServiceDLocalRefs refs = {
    &serviceDCertState,
    &activeConnHandle,
    &serviceBChar1,
    &serviceDChar2,
    &serviceDChar3,
    serviceDStatus,
    sizeof(serviceDStatus),
    serviceDReadBlob,
    &serviceDReadPendingLen,
    serviceDType3ReadPayload,
    sizeof(serviceDType3ReadPayload),
    serviceDType5ReadPayload,
    sizeof(serviceDType5ReadPayload),
    &postServiceDHandoffActive,
    &postServiceDServiceBReadSeen,
    &postServiceDServiceBValue,
    DONOR_SERVICE_D_READ_TYPE3,
    sizeof(DONOR_SERVICE_D_READ_TYPE3),
    rebuildServiceDRuntimeBlock,
    printHex,
    logServiceDCertState,
    refreshPairingExports,
    handleOlderServiceDFlowWriteDispatch,
    handleReconnectServiceDFlowWriteDispatch
  };
  return refs;
}


static void handleReconnectAuthWrite25Dispatch(const uint8_t* reply,
                                               uint16_t replyLen,
                                               const char* reason);

static ReconnectRuntimeRefs makeReconnectRuntimeRefs()
{
  ReconnectRuntimeRefs refs = {
    &activeConnHandle,
    &pairingStage,
    &buttonNotifyEnabled,
    &authBlobNotifyEnabled,
    &persistentReconnectReady,
    &persistedAcceptedCentralNonceValid,
    persistedAcceptedCentralNonce,
    reconnectAuthReply24,
    sizeof(reconnectAuthReply24),
    &freshAuthBlobSent,
    &reconnectServiceDArmed,
    &authControl,
    updateAuthReplyIndexSlot,
    buildIndexedAuthReply24,
    printHex,
    setAuthParserSchedulerState,
    buildDonorReconnectAuthBlob,
    notifyStagedAuthBlob106,
    markAuthComplete
  };
  return refs;
}

static AuthBlobRuntimeRefs makeAuthBlobRuntimeRefs()
{
  AuthBlobRuntimeRefs refs = {
    &authBlobStage,
    &authParserOutput,
    &authBlob,
    authBlobVisibleValue,
    sizeof(authBlobVisibleValue),
    setAuthParserSchedulerState
  };
  return refs;
}


static CccdSeedRefs makeCccdSeedRefs()
{
  CccdSeedRefs refs = {
    &activeConnHandle,
    &userCccdsSeeded,
    &serviceDNotifyEnabled,
    &serviceAChar5,
    &serviceBChar1,
    &serviceBChar2,
    &serviceCChar1,
    &serviceDChar2,
    &buttonNotifyChar,
    &authControl,
    &authBlob
  };
  return refs;
}

static ButtonRuntimeRefs makeButtonRuntimeRefs()
{
  ButtonRuntimeRefs refs = {
    &activeConnHandle,
    &pairingStage,
    &userCccdsSeeded,
    &buttonNotifyEnabled,
    &authControlNotifyEnabled,
    &buttonNotifyChar,
    &serviceAChar5,
    &authControl,
    &serviceBChar1,
    buttonState,
    serviceAStatus,
    &runtimeButtonEngineActive,
    &runtimeButtonActionArmed,
    &runtimeButtonAwaitingAck,
    &runtimeButtonSettlingToIdle,
    &runtimeButtonFamilyIndex,
    &runtimeButtonCurrentFamily,
    &runtimeButtonIdleCount,
    &runtimeButtonAt,
    &buttonPulseActive,
    &buttonPulseReleaseAt,
    &postAuthIdleButtonRemaining,
    &postAuthIdleButtonAt,
    &postAuthReady18Pending,
    &postAuthReady18At,
    PLUSPLUS_AUTH_CTRL_REPLY_18,
    sizeof(PLUSPLUS_AUTH_CTRL_REPLY_18),
    seedUserServiceCccds
  };
  return refs;
}

static ReconnectCompactServiceDRefs makeReconnectCompactServiceDRefs()
{
  ReconnectCompactServiceDRefs refs = {
    &reconnectCompactServiceDMode,
    &reconnectServiceDArmed,
    &serviceBNotifyEnabled,
    &postServiceDHandoffActive,
    &postServiceDServiceBReadSeen,
    &postServiceDServiceBValue,
    &serviceDCertState,
    &lastOlderServiceDType2WriteValid,
    &freshAuthStep,
    &freshAuthAt,
    &pairingStage,
    &serviceBChar1,
    refreshServiceDLocalPayloadsFromRuntime,
    prepareSfidaReadChallenge,
    notifyServiceDStatus,
    armServiceDCertExpectation,
    buildServiceDType5PayloadFromLiveWrite,
    resetServiceDCertState,
    logServiceDCertState
  };
  return refs;
}

static SleepExportRuntimeRefs makeSleepExportRuntimeRefs()
{
  SleepExportRuntimeRefs refs = {
    &pairingStage,
    &freshAuthStep,
    &authControl,
    &authBlob,
    LONG_VALUE_LEN,
    PLUSPLUS_AUTH_CTRL_REPLY_26,
    sizeof(PLUSPLUS_AUTH_CTRL_REPLY_26),
    PLUSPLUS_AUTH_BLOB_SHORT,
    sizeof(PLUSPLUS_AUTH_BLOB_SHORT),
    clearPostServiceDHandoff,
    setAuthParserSchedulerState
  };
  return refs;
}

static FirstPairRuntimeRefs makeFirstPairRuntimeRefs()
{
  FirstPairRuntimeRefs refs = {
    &activeConnHandle,
    &pairingStage,
    &persistentReconnectReady,
    &serviceCNotifyEnabled,
    &buttonNotifyEnabled,
    &authControlNotifyEnabled,
    &authBlobNotifyEnabled,
    &serviceCChar1,
    &buttonNotifyChar,
    &authControl,
    &authBlob,
    &donorAuthRuntime,
    &authParserOutput,
    serviceCState,
    buttonState,
    candidatePair,
    activePair,
    stagedPair,
    currentAttemptCentralNonce,
    persistedAcceptedCentralNonce,
    plusplusManufacturerData,
    &stagedPairValid,
    &currentAttemptCentralNonceValid,
    &persistedAcceptedCentralNonceValid,
    &freshAuthStarted,
    &freshAuthBlobSent,
    &serviceCBurstStep,
    &serviceCBurstAt,
    &connStartedAt,
    &freshAuthStep,
    &freshAuthAt,
    &firstPairSawReal36,
    &firstPairSawReal28,
    &serviceCTransferState,
    &pairEventClass,
    PLUSPLUS_FRESH_AUTH_CTRL_REPLY_24,
    sizeof(PLUSPLUS_FRESH_AUTH_CTRL_REPLY_24),
    PLUSPLUS_AUTH_CTRL_REPLY_25,
    sizeof(PLUSPLUS_AUTH_CTRL_REPLY_25),
    PLUSPLUS_AUTH_CTRL_REPLY_18,
    sizeof(PLUSPLUS_AUTH_CTRL_REPLY_18),
    PLUSPLUS_AUTH_CTRL_REPLY_18_FAIL,
    sizeof(PLUSPLUS_AUTH_CTRL_REPLY_18_FAIL),
    PLUSPLUS_AUTH_CTRL_REPLY_16,
    sizeof(PLUSPLUS_AUTH_CTRL_REPLY_16),
    PLUSPLUS_AUTH_CTRL_REPLY_21_WAIT,
    sizeof(PLUSPLUS_AUTH_CTRL_REPLY_21_WAIT),
    PLUSPLUS_AUTH_CTRL_REPLY_21_READY,
    sizeof(PLUSPLUS_AUTH_CTRL_REPLY_21_READY),
    PLUSPLUS_AUTH_CTRL_REPLY_36,
    sizeof(PLUSPLUS_AUTH_CTRL_REPLY_36),
    PLUSPLUS_AUTH_CTRL_REPLY_28,
    sizeof(PLUSPLUS_AUTH_CTRL_REPLY_28),
    PLUSPLUS_AUTH_CTRL_REPLY_26,
    sizeof(PLUSPLUS_AUTH_CTRL_REPLY_26),
    PLUSPLUS_AUTH_BLOB_SHORT,
    sizeof(PLUSPLUS_AUTH_BLOB_SHORT),
    SERVICE_C_NOTIFY_LEN,
    REAL_PLUSPLUS_ADDR_LE,
    sizeof(REAL_PLUSPLUS_ADDR_LE),
    PLUSPLUS_FACTORY_CERT_BLOCK,
    sizeof(PLUSPLUS_FACTORY_CERT_BLOCK),
    FRESH_AUTH_STEP_DELAY_MS,
    FIRSTPAIR_SERVICEC_NOTIFY1_DELAY_MS,
    FIRSTPAIR_SERVICEC_NOTIFY2_DELAY_MS,
    FIRSTPAIR_SERVICEC_NOTIFY3_DELAY_MS,
    FRESH_AUTH_18_TO_16_DELAY_MS,
    FRESH_AUTH_18_TO_REPLY_DELAY_MS,
    FRESH_AUTH_WAIT_FOR_28_WRITE_MS,
    FRESH_AUTH_STEP_DELAY_MS,
    stageCandidatePair,
    commitStagedPair,
    buildIndexedAuthReply24,
    updateAuthReplyIndexSlot,
    refreshServiceDLocalPayloadsFromRuntime,
    printHex,
    setAuthParserSchedulerState,
    setFirstPairCertState,
    scheduleFreshAuthSequence,
    logDonorAuthRuntime,
    donorLocal29CommandCodeForFamily,
    queueDonorAuthRuntimeStep,
    commitPersistedServiceDSelectorStateFromAuth36,
    commitPersistedServiceDRecord1AFromAuth28,
    clearReconnectServiceDArm,
    clearPostServiceDHandoff,
    refreshReconnectAuthReply24,
    armPostAuthRuntimeScaffolding,
    savePersistentPairState,
    refreshPairingExports,
    buildGeneratedInitialAuthBlob,
    notifyStagedAuthBlob106,
    setManufacturerState,
    pairingStageLabel,
    readyStateCode,
    deriveDonorLocalFamilyFromParserBytes,
    resetAuthParserOutput,
    processReconnectCompactCompletion,
    scheduleServiceCFirstPairBurst
  };
  return refs;
}

static FirstPairOlderServiceDRefs makeFirstPairOlderServiceDRefs()
{
  FirstPairOlderServiceDRefs refs = {
    &serviceDCertState,
    &lastOlderServiceDType2WriteValid,
    lastOlderServiceDType2Write,
    sizeof(lastOlderServiceDType2Write),
    &freshAuthStep,
    &freshAuthAt,
    &reconnectServiceDArmed,
    &reconnectCompactServiceDMode,
    refreshServiceDLocalPayloadsFromRuntime,
    prepareOlderServiceDReadStage,
    notifyServiceDStatus,
    armServiceDCertExpectation,
    incrementServiceDRuntimeTailCounter,
    logServiceDCertState
  };
  return refs;
}


static void seedUserServiceCccds(const char* reason)
{
  ppSeedUserServiceCccds(makeCccdSeedRefs(), reason);
}


static void logServiceDCertState(const char* reason)
{
  ppLogServiceDCertState(makePairHelperRefs(), reason);
}

static void setAuthParserSchedulerState(uint8_t schedulerState,
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
  ppSetAuthParserSchedulerState(makePairHelperRefs(),
                                schedulerState,
                                outputClass,
                                outputLen,
                                outputOffset,
                                resultCode,
                                continuationKind,
                                lastOpcode,
                                outputPending,
                                streamPending,
                                notifyInFlight);
}

static void rebuildServiceDRuntimeBlock()
{
  ppRebuildServiceDRuntimeBlock(makePairHelperRefs());
}

static void incrementServiceDRuntimeTailCounter()
{
  ppIncrementServiceDRuntimeTailCounter(makePairHelperRefs());
}

static void commitPersistedServiceDSelectorStateFromAuth36()
{
  ppCommitPersistedServiceDSelectorStateFromAuth36(makePairHelperRefs());
}

static void commitPersistedServiceDRecord1AFromAuth28()
{
  ppCommitPersistedServiceDRecord1AFromAuth28(makePairHelperRefs());
}

static void refreshServiceDLocalPayloadsFromRuntime()
{
  ppRefreshServiceDLocalPayloadsFromRuntime(makeServiceDLocalRefs());
}


static void resetPairingFromApp(const char* reason)
{
  ppResetPairingFromApp(makeAppLifecycleRefs(), reason);
}

static void resetPairingFromPhysicalButton(const char* reason)
{
  ppResetPairingFromPhysicalButton(makeAppLifecycleRefs(), reason);
}

static void armDisconnectedSleepTimer(const char* reason)
{
  ppArmDisconnectedSleepTimer(makeAppLifecycleRefs(), reason);
}

static void processDisconnectedSleepIfNeeded()
{
  ppProcessDisconnectedSleepIfNeeded(makeAppLifecycleRefs());
}

static void processPhysicalResetButtonIfNeeded()
{
  ppProcessPhysicalResetButtonIfNeeded(makeAppLifecycleRefs());
}

static void buildServiceDType5PayloadFromLiveWrite(const uint8_t* write36)
{
  ppBuildServiceDType5PayloadFromLiveWrite(makeAuthMaterialRefs(), write36);
}

static void armServiceDCertExpectation(uint8_t state, uint8_t expectedType, uint16_t expectedLen, const char* reason)
{
  serviceDCertState.active = true;
  serviceDCertState.laneStep = state;
  serviceDCertState.expectedWriteType = expectedType;
  serviceDCertState.expectedWriteLen = expectedLen;
  logServiceDCertState(reason);
}

static uint8_t freshAuthInitialChallenge[16] = { 0x00 };
static uint8_t freshAuthSessionKey[16] = { 0x00 };
static uint8_t liveCertKey[16] = { 0x00 };
static uint8_t sfidaCertStage = 0;
#include "pp_storage_refs.h"

static bool handleFirstPairAuthWriteDispatch(const uint8_t* data, uint16_t len)
{
  return ppFirstPairFlowHandleAuthWrite(makeFirstPairRuntimeRefs(), data, len);
}

static void handleReconnectAuthWrite24Dispatch(uint8_t selector, const char* reason)
{
  ppReconnectFlowHandleAuthWrite24(makeReconnectRuntimeRefs(), selector, reason);
}

static void handleReconnectAuthWrite25Dispatch(const uint8_t* reply,
                                               uint16_t replyLen,
                                               const char* reason)
{
  ppReconnectFlowHandleAuthWrite25(makeReconnectRuntimeRefs(), reply, replyLen, reason);
}

static SleepExportWriteResult handleAuthenticatedSleepWriteDispatch(const uint8_t* data, uint16_t len)
{
  return ppHandleAuthenticatedSleepWrite(makeSleepExportRuntimeRefs(), data, len);
}

static void processAuthenticatedSleepStream()
{
  ppProcessAuthenticatedSleepStream(makeSleepExportRuntimeRefs());
}

static void prepareReconnectSleepDataOnAuthenticatedDisconnect()
{
  ppPrepareReconnectSleepDataForAuthenticatedDisconnect();
}

static void resetReconnectSleepDataState()
{
  ppResetReconnectSleepDataState();
}

static BleDispatchRefs makeBleDispatchRefs()
{
  BleDispatchRefs refs = {
    &pairingStage,
    &buttonNotifyEnabled,
    &serviceCNotifyEnabled,
    &serviceBNotifyEnabled,
    &serviceDNotifyEnabled,
    &authControlNotifyEnabled,
    &authBlobNotifyEnabled,
    &freshAuthStarted,
    &freshAuthStep,
    &freshAuthAt,
    &sfidaCertStage,
    &reconnectServiceDArmed,
    &persistentReconnectReady,
    &freshAuthBlobSent,
    &postServiceDHandoffActive,
    &postServiceDServiceBReadSeen,
    &postServiceDServiceBValue,
    &serviceCTransferState,
    &pairEventClass,
    serviceCState,
    sizeof(serviceCState),
    serviceDStatus,
    sizeof(serviceDStatus),
    configState,
    sizeof(configState),
    freshAuthInitialChallenge,
    sizeof(freshAuthInitialChallenge),
    authBlobVisibleValue,
    sizeof(authBlobVisibleValue),
    &authBlobStage,
    &authParserOutput,
    &serviceDCertState,
    &batteryLevelChar,
    &serviceAChar1,
    &serviceAChar2,
    &serviceAChar3,
    &serviceAChar4,
    &serviceAChar5,
    &serviceBChar1,
    &serviceBChar2,
    &serviceCChar1,
    &serviceCChar2,
    &serviceDChar1,
    &serviceDChar2,
    &serviceDChar3,
    &ledWriteChar,
    &buttonNotifyChar,
    &serviceEChar3,
    &serviceEChar4,
    &versionChar,
    &configChar,
    &authControl,
    &authBlob,
    PLUSPLUS_AUTH_CTRL_REPLY_25,
    sizeof(PLUSPLUS_AUTH_CTRL_REPLY_25),
    CONFIG_RESET_PAIRED_STATE_MAGIC,
    SHORT_VALUE_LEN,
    MEDIUM_VALUE_LEN,
    FRESH_AUTH_WAIT_FOR_28_WRITE_MS,
    pairingStageLabel,
    printHex,
    refreshPairingExports,
    stageCandidatePair,
    commitStagedPair,
    scheduleServiceCFirstPairBurst,
    beginServiceDOlderFlow,
    beginServiceDCertFlow,
    handleServiceDCertWrite,
    stopPostAuthRuntimeScaffolding,
    maybeTriggerButtonFromLedPayload,
    resetPairingFromApp,
    plusplusGenerateReconnectResponse,
    handleFirstPairAuthWriteDispatch,
    handleReconnectAuthWrite24Dispatch,
    handleReconnectAuthWrite25Dispatch,
    handleAuthenticatedSleepWriteDispatch,
    setAuthParserSchedulerState
  };
  return refs;
}


static void prepareOlderServiceDReadStage(uint8_t stage)
{
  ppPrepareOlderServiceDReadStage(makeAuthMaterialRefs(), stage);
}

static void prepareSfidaReadChallenge(uint8_t stage)
{
  ppPrepareSfidaReadChallenge(makeAuthMaterialRefs(), stage);
}

static void buildGeneratedInitialAuthBlob()
{
  ppBuildGeneratedInitialAuthBlob(makeAuthMaterialRefs());
}

static void buildDonorReconnectAuthBlob()
{
  ppBuildDonorReconnectAuthBlob(makeAuthMaterialRefs());
}

static bool plusplusGenerateReconnectResponse(const uint8_t* challenge32, uint8_t* response16)
{
  return ppGenerateReconnectResponse(makeAuthMaterialRefs(), challenge32, response16);
}

static uint8_t readyStateCode()
{
  return ppReadyStateCode(pairingStage);
}

static const char* pairingStageLabel(PairingStage stage)
{
  return ppPairingStageLabel(stage);
}

static void scheduleServiceCFirstPairBurst()
{
  ppFirstPairFlowScheduleServiceCBurst(makeFirstPairRuntimeRefs());
}

static void processServiceCFirstPairBurst()
{
  ppFirstPairFlowProcessServiceCBurst(makeFirstPairRuntimeRefs());
}

static void processPostAuthIdleButtonNotify()
{
  ppProcessPostAuthIdleButtonNotify(makeButtonRuntimeRefs(),
                                    POST_AUTH_IDLE_BUTTON_PERIOD_MS);
}

static void processPostAuthReady18()
{
  ppProcessPostAuthReady18(makeButtonRuntimeRefs());
}

static void armPostAuthRuntimeScaffolding()
{
  ppArmPostAuthRuntimeScaffolding(makeButtonRuntimeRefs(),
                                  POST_AUTH_IDLE_BUTTON_COUNT,
                                  POST_AUTH_IDLE_BUTTON_FIRST_DELAY_MS,
                                  POST_AUTH_READY_18_DELAY_MS);
}

static bool processReconnectCompactCompletion()
{
  return ppReconnectFlowProcessCompactCompletion(makeReconnectCompactServiceDRefs());
}

static void scheduleFreshAuthSequence(uint8_t firstStep, const char* reason)
{
  ppFirstPairFlowScheduleFreshAuthSequence(makeFirstPairRuntimeRefs(), firstStep, reason);
}

static void clearAuthBlobStage()
{
  ppClearAuthBlobStage(makeAuthBlobRuntimeRefs());
}

static void refreshReconnectAuthReply24(const char* reason)
{
  ppReconnectFlowRefreshAuthReply24(makeReconnectRuntimeRefs(), reason);
}

static void setFirstPairCertState(uint8_t nextState, const char* reason)
{
  ppSetFirstPairCertState(makePairRuntimeSupportRefs(), nextState, reason);
}

static void resetDonorAuthRuntime()
{
  ppResetDonorAuthRuntime(makePairRuntimeSupportRefs());
}

static void resetAuthParserOutput()
{
  ppResetAuthParserOutput(makePairRuntimeSupportRefs());
}

static void logDonorAuthRuntime(const char* reason)
{
  ppLogDonorAuthRuntime(makePairRuntimeSupportRefs(), reason);
}

static uint8_t donorLocal29CommandCodeForFamily(uint8_t family)
{
  return ppDonorLocal29CommandCodeForFamily(family);
}

static uint8_t deriveDonorLocalFamilyFromParserBytes()
{
  return ppDeriveDonorLocalFamilyFromParserBytes(makePairRuntimeSupportRefs());
}

static void queueDonorAuthRuntimeStep(uint8_t nextStep, uint32_t delayMs, const char* reason)
{
  ppQueueDonorAuthRuntimeStep(makePairRuntimeSupportRefs(), nextStep, delayMs, reason);
}

static void seedAuthReplyIndexTable()
{
  ppSeedAuthReplyIndexTable(makePairRuntimeSupportRefs(), PLUSPLUS_AUTH_CTRL_REPLY_2401);
}

static void updateAuthReplyIndexSlot(uint8_t index, const uint8_t* value3, const char* reason)
{
  ppUpdateAuthReplyIndexSlot(makePairRuntimeSupportRefs(), index, value3, reason);
}

static void buildIndexedAuthReply24(uint8_t index, uint8_t* out5, const char* reason)
{
  ppBuildIndexedAuthReply24(makePairRuntimeSupportRefs(), index, out5, reason);
}

static void stageVisibleAuthBlob(const uint8_t* payload, uint16_t payloadLen, uint8_t stageClass, uint8_t lastOpcode)
{
  ppStageVisibleAuthBlob(makeAuthBlobRuntimeRefs(), payload, payloadLen, stageClass, lastOpcode);
}

static void stageVisibleAuthBlob106()
{
  ppStageVisibleAuthBlob106(makeAuthBlobRuntimeRefs());
}

static void notifyStagedAuthBlob106(const char* reason)
{
  ppNotifyStagedAuthBlob106(makeAuthBlobRuntimeRefs(), reason);
}

static AuthMaterialRefs makeAuthMaterialRefs()
{
  AuthMaterialRefs refs = {
    freshAuthInitialChallenge,
    freshAuthSessionKey,
    liveCertKey,
    &serviceDCertState,
    &serviceDChar3,
    serviceDReadBlob,
    &serviceDReadPendingLen,
    authBlobValue,
    authBlobVisibleValue,
    &authBlobStage,
    serviceDType3ReadPayload,
    serviceDType5ReadPayload,
    persistedServiceDSelector4,
    &persistedServiceDSelector4Valid,
    persistedServiceDRecord6,
    &persistedServiceDRecord6Valid,
    lastOlderServiceDType2Write,
    &lastOlderServiceDType2WriteValid,
    &donorAuthRuntime,
    refreshServiceDLocalPayloadsFromRuntime,
    stageVisibleAuthBlob106,
    stageVisibleAuthBlob,
    printHex
  };
  return refs;
}

static PairRuntimeSupportRefs makePairRuntimeSupportRefs()
{
  PairRuntimeSupportRefs refs = {
    &firstPairCertState,
    &donorAuthRuntime,
    &authParserOutput,
    authReplyIndexTable,
    sizeof(authReplyIndexTable) / sizeof(authReplyIndexTable[0]),
    &freshAuthStep,
    &freshAuthAt,
    printHex,
    setAuthParserSchedulerState
  };
  return refs;
}

static PairHelperRefs makePairHelperRefs()
{
  PairHelperRefs refs = {
    &serviceDCertState,
    &authParserOutput,
    serviceDRuntimeBlock,
    sizeof(serviceDRuntimeBlock),
    persistedServiceDSelector4,
    &persistedServiceDSelector4Valid,
    &persistedServiceDSelector5,
    &persistedServiceDSelector5Valid,
    persistedServiceDRecord6,
    &persistedServiceDRecord6Valid,
    &persistedServiceDRecord1A,
    &persistedServiceDRecord1AValid,
    FIRMWARE_RECORD4_DEFAULT,
    sizeof(persistedServiceDSelector4),
    sizeof(persistedServiceDRecord6),
    REAL_PLUSPLUS_ADDR_LE,
    sizeof(REAL_PLUSPLUS_ADDR_LE),
    &pairingStage,
    printHex
  };
  return refs;
}

static AuthBootstrapRefs makeAuthBootstrapRefs()
{
  AuthBootstrapRefs refs = {
    liveCertKey,
    sizeof(liveCertKey),
    PLUSPLUS_FACTORY_SELECTOR3,
    seedAuthReplyIndexTable,
    clearAuthBlobStage,
    buildGeneratedInitialAuthBlob
  };
  return refs;
}

static void notifyServiceDStatus(const uint8_t* status, const char* label)
{
  ppNotifyServiceDStatus(makeServiceDLocalRefs(), status, label);
}

static void clearPostServiceDHandoff()
{
  ppClearPostServiceDHandoff(makeServiceDLocalRefs());
}

static void beginServiceDCertFlow()
{
  ppReconnectFlowBeginCompactServiceD(makeReconnectCompactServiceDRefs(),
                                      PLUSPLUS_SERVICE_D_STATUS3_START,
                                      FRESH_AUTH_WAIT_FOR_28_WRITE_MS);
}

static void beginServiceDOlderFlow()
{
  ppFirstPairFlowBeginOlderServiceD(makeFirstPairOlderServiceDRefs(),
                                    PLUSPLUS_SERVICE_D_STATUS0);
}

static bool handleServiceDCertWrite(const uint8_t* data, uint16_t len)
{
  return ppHandleServiceDCertWrite(makeServiceDLocalRefs(), data, len);
}

static void processFreshAuthSequence()
{
  ppFirstPairFlowProcessFreshAuthSequence(makeFirstPairRuntimeRefs());
}

static void stageCandidatePair(const char* reason)
{
  ppFirstPairFlowStageCandidatePair(makeFirstPairRuntimeRefs(), reason);
}

static void commitStagedPair(const char* reason)
{
  ppFirstPairFlowCommitStagedPair(makeFirstPairRuntimeRefs(), reason);
}

static void markAuthComplete()
{
  ppFirstPairFlowMarkAuthComplete(makeFirstPairRuntimeRefs());
}


static void printHex(const uint8_t* data, uint16_t len)
{
  ppPrintHex(data, len);
}

static void print_build_tag(const char* context)
{
  ppPrintBuildTag(BUILD_TAG, context);
}

static void seedLongReadBlob()
{
  ppSeedLongReadBlob(serviceDReadBlob, LONG_VALUE_LEN, VERSION_STRING);
}

static void seedAuthBlob()
{
  ppSeedAuthBlob(makeAuthBootstrapRefs());
}

static void resetRuntimeButtonEngine()
{
  ppResetRuntimeButtonEngine(makeButtonRuntimeRefs());
}

static void stopPostAuthRuntimeScaffolding()
{
  ppStopPostAuthRuntimeScaffolding(makeButtonRuntimeRefs());
}

static void clearButtonPulseIfNeeded()
{
  ppClearButtonPulseIfNeeded(makeButtonRuntimeRefs());
}

static void processRuntimeButtonEngineIfNeeded()
{
  ppProcessRuntimeButtonEngineIfNeeded(makeButtonRuntimeRefs(),
                                       RUNTIME_BUTTON_IDLE_COUNT_BEFORE_ACTION,
                                       RUNTIME_BUTTON_IDLE_PERIOD_MS);
}

static void maybeTriggerButtonFromLedPayload(const uint8_t* data, uint16_t len)
{
  ppMaybeTriggerButtonFromLedPayload(makeButtonRuntimeRefs(),
                                     data,
                                     len,
                                     RUNTIME_BUTTON_IDLE_COUNT_BEFORE_ACTION,
                                     RUNTIME_BUTTON_PRESTATE_SEND_COUNT,
                                     RUNTIME_BUTTON_PRESTATE_SEND_GAP_MS,
                                     RUNTIME_BUTTON_ACK_TIMEOUT_MS,
                                     RUNTIME_BUTTON_FIRST_IDLE_DELAY_MS,
                                     RUNTIME_BUTTON_IDLE_PERIOD_MS);
}

