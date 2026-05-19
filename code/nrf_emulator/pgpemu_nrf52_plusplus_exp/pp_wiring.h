#pragma once

static void stageCandidatePair(const char* reason);
static void commitStagedPair(const char* reason);
static void markAuthComplete();
static bool savePersistentPairState(const char* reason);
static bool loadPersistentPairState();
static void clearPersistentPairState(const char* reason);
static void restorePairedStateFromPersistence();
static void resetPairingFromApp(const char* reason);
static void resetPairingFromPhysicalButton(const char* reason);
static void processPhysicalResetButtonIfNeeded();
static void start_advertising();
static void setManufacturerState(uint8_t state);
static void refreshPairingExports(bool notify);
static void processAuthenticatedSleepStream();
static void prepareReconnectSleepDataOnAuthenticatedDisconnect();
static void resetReconnectSleepDataState();
static void printHex(const uint8_t* data, uint16_t len);
static bool beginPersistentStore();
static void setup_services();
static void configure_identity();
static void configure_gap_preferences();
static void connect_callback(uint16_t conn_handle);
static void disconnect_callback(uint16_t conn_handle, uint8_t reason);
static void clearButtonPulseIfNeeded();
static void processPostAuthIdleButtonNotify();
static void processPostAuthReady18();
static void processDisconnectedSleepIfNeeded();
static void processFreshAuthSequence();
static void processServiceCFirstPairBurst();
static void processRuntimeButtonEngineIfNeeded();
static PairStateRuntimeRefs makePairStateRuntimeRefs();
static FirstPairOlderServiceDRefs makeFirstPairOlderServiceDRefs();
static ReconnectCompactServiceDRefs makeReconnectCompactServiceDRefs();
static PersistedPairRuntimeRefs makePersistedPairRuntimeRefs();
static BleDispatchRefs makeBleDispatchRefs();
static ServiceDLocalRefs makeServiceDLocalRefs();
static ReconnectRuntimeRefs makeReconnectRuntimeRefs();
static AuthBlobRuntimeRefs makeAuthBlobRuntimeRefs();
static AuthMaterialRefs makeAuthMaterialRefs();
static CccdSeedRefs makeCccdSeedRefs();
static ButtonRuntimeRefs makeButtonRuntimeRefs();
static SleepExportRuntimeRefs makeSleepExportRuntimeRefs();
static FirstPairRuntimeRefs makeFirstPairRuntimeRefs();
static PairRuntimeSupportRefs makePairRuntimeSupportRefs();
static PairHelperRefs makePairHelperRefs();
static AuthBootstrapRefs makeAuthBootstrapRefs();
static void rebuildServiceDRuntimeBlock();
static void logServiceDCertState(const char* reason);
static void buildGeneratedInitialAuthBlob();
static void buildDonorReconnectAuthBlob();
static void buildIndexedAuthReply24(uint8_t index, uint8_t* out5, const char* reason);
static void buildServiceDType5PayloadFromLiveWrite(const uint8_t* write36);
static void cccd_write_callback(uint16_t conn_handle, BLECharacteristic* chr, uint16_t value);
static void clearAuthBlobStage();
static void clearPostServiceDHandoff();
static void clearReconnectServiceDArm();
static void setFirstPairCertState(uint8_t nextState, const char* reason);
static void commitPersistedServiceDSelectorStateFromAuth36();
static void commitPersistedServiceDRecord1AFromAuth28();
static uint8_t deriveDonorLocalFamilyFromParserBytes();
static void incrementServiceDRuntimeTailCounter();
static bool handleFirstPairAuthWriteDispatch(const uint8_t* data, uint16_t len);
static void handleReconnectAuthWrite24Dispatch(uint8_t selector, const char* reason);
static void handleReconnectAuthWrite25Dispatch(const uint8_t* reply,
                                               uint16_t replyLen,
                                               const char* reason);
static SleepExportWriteResult handleAuthenticatedSleepWriteDispatch(const uint8_t* data, uint16_t len);
static bool handleOlderServiceDFlowWriteDispatch(const uint8_t* data, uint16_t len);
static bool handleReconnectServiceDFlowWriteDispatch(const uint8_t* data, uint16_t len);
static void refreshServiceDLocalPayloadsFromRuntime();
static void beginServiceDCertFlow();
static void beginServiceDOlderFlow();
static bool handleServiceDCertWrite(const uint8_t* data, uint16_t len);
static void logDonorAuthRuntime(const char* reason);
static void notifyServiceDStatus(const uint8_t* status, const char* label);
static void notifyStagedAuthBlob106(const char* reason);
static void prepareOlderServiceDReadStage(uint8_t stage);
static void prepareSfidaReadChallenge(uint8_t stage);
static void print_build_tag(const char* context);
static void queueDonorAuthRuntimeStep(uint8_t nextStep, uint32_t delayMs, const char* reason);
static uint8_t readyStateCode();
static void read_authorize_callback(uint16_t conn_handle, BLECharacteristic* chr, ble_gatts_evt_read_t* request);
static void refreshReconnectAuthReply24(const char* reason);
static void resetAuthParserOutput();
static void resetDonorAuthRuntime();
static void resetRuntimeButtonEngine();
static void resetServiceDCertState();
static void resetUnpairedState();
static void scheduleFreshAuthSequence(uint8_t firstStep, const char* reason);
static void scheduleServiceCFirstPairBurst();
static void seedAuthBlob();
static void seedAuthReplyIndexTable();
static void seedLongReadBlob();
static void seedUserServiceCccds(const char* reason);
static void service_write_callback(uint16_t conn_handle, BLECharacteristic* chr, uint8_t* data, uint16_t len);
static void setAuthParserSchedulerState(uint8_t schedulerState,
                                        uint8_t outputClass,
                                        uint16_t outputLen,
                                        uint16_t outputOffset,
                                        uint8_t resultCode,
                                        uint8_t continuationKind,
                                        uint8_t lastOpcode,
                                        bool outputPending,
                                        bool streamPending,
                                        bool notifyInFlight);
static void stopPostAuthRuntimeScaffolding();
static void updateAuthReplyIndexSlot(uint8_t index, const uint8_t* value3, const char* reason);
static void armDisconnectedSleepTimer(const char* reason);
static void armServiceDCertExpectation(uint8_t state, uint8_t expectedType, uint16_t expectedLen, const char* reason);
static void armPostAuthRuntimeScaffolding();
static uint8_t donorLocal29CommandCodeForFamily(uint8_t family);
static bool plusplusGenerateReconnectResponse(const uint8_t* challenge32, uint8_t* response16);
static const char* pairingStageLabel(PairingStage stage);
static bool processReconnectCompactCompletion();
static void maybeTriggerButtonFromLedPayload(const uint8_t* data, uint16_t len);




#include "pp_ble_refs.h"

#include "pp_flow_refs.h"
