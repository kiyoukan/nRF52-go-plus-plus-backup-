#include <Arduino.h>
#include <string.h>

#include "pp_pair_state.h"

namespace {

static void logHexBytes(const uint8_t* data, size_t len)
{
  for (size_t i = 0; i < len; ++i) {
    if (i) {
      Serial.print(" ");
    }
    if (data[i] < 0x10) {
      Serial.print("0");
    }
    Serial.print(data[i], HEX);
  }
}

static void logPairingExportValue(const char* label, const uint8_t* data, size_t len)
{
  Serial.print("Pairing export ");
  Serial.print(label);
  Serial.print(" len=");
  Serial.print(static_cast<uint32_t>(len));
  Serial.print(" data=");
  logHexBytes(data, len);
  Serial.println();
}

}  // namespace

void ppResetServiceDCertState(const PairStateRuntimeRefs& refs)
{
  refs.serviceDCertState->active = false;
  refs.serviceDCertState->readPrepared = false;
  refs.serviceDCertState->completionPending = false;
  refs.serviceDCertState->laneKind = SERVICE_D_FLOW_NONE;
  refs.serviceDCertState->laneStep = 0;
  refs.serviceDCertState->expectedWriteType = 0;
  refs.serviceDCertState->expectedWriteLen = 0;
  refs.serviceDCertState->lastObservedType = 0xff;
  refs.serviceDCertState->lastObservedLen = 0;
  memset(refs.serviceDCertState->lastStatus, 0, sizeof(refs.serviceDCertState->lastStatus));
}

void ppClearReconnectServiceDArm(const PairStateRuntimeRefs& refs)
{
  *refs.reconnectServiceDArmed = false;
  *refs.reconnectCompactServiceDMode = false;
}

void ppResetPersistedServiceDState(const PairStateRuntimeRefs& refs)
{
  memset(refs.persistedServiceDSelector4, 0, 16);
  *refs.persistedServiceDSelector4Valid = false;
  *refs.persistedServiceDSelector5 = 0x00;
  *refs.persistedServiceDSelector5Valid = false;
  memcpy(refs.persistedServiceDRecord6, refs.factorySelector6, refs.factorySelector6Len);
  *refs.persistedServiceDRecord6Valid = true;
  *refs.persistedServiceDRecord1A = 0;
  *refs.persistedServiceDRecord1AValid = false;
  memset(refs.serviceDRuntimeBlock, 0, 32);
}

void ppResetVolatilePairSessionState(const PairStateRuntimeRefs& refs)
{
  *refs.freshAuthStep = 0;
  *refs.freshAuthAt = 0;
  *refs.freshAuthStarted = false;
  *refs.freshAuthBlobSent = false;
  *refs.sfidaCertStage = 0;
  *refs.userCccdsSeeded = false;
  ppResetServiceDCertState(refs);
  refs.resetDonorAuthRuntime();
  ppClearReconnectServiceDArm(refs);
}

void ppResetUnpairedState(const PairStateRuntimeRefs& refs)
{
  refs.resetReconnectSleepDataState();
  memcpy(refs.liveCertKey, refs.selector3, refs.selector3Len);
  ppResetPersistedServiceDState(refs);
  ppClearReconnectServiceDArm(refs);
  *refs.pairingStage = PAIR_STAGE_DISCOVERY;
  *refs.pairEventClass = 0x21;
  refs.candidatePair[0] = 0x00;
  refs.candidatePair[1] = 0x00;
  refs.activePair[0] = 0x00;
  refs.activePair[1] = 0x00;
  refs.stagedPair[0] = 0x00;
  refs.stagedPair[1] = 0x00;
  *refs.stagedPairValid = false;
  *refs.connStartedAt = 0;
  refs.resetRuntimeButtonEngine();
  *refs.serviceCTransferState = 0x00;
  *refs.serviceCBurstStep = 0;
  *refs.serviceCBurstAt = 0;
  *refs.serviceCNotifyEnabled = false;
  *refs.serviceBNotifyEnabled = false;
  *refs.buttonNotifyEnabled = false;
  *refs.serviceDNotifyEnabled = false;
  *refs.authControlNotifyEnabled = false;
  *refs.authBlobNotifyEnabled = false;
  *refs.postServiceDHandoffActive = false;
  *refs.postServiceDServiceBReadSeen = false;
  *refs.postServiceDServiceBValue = 0x04;
  ppResetVolatilePairSessionState(refs);
  refs.stopPostAuthRuntimeScaffolding();
  *refs.persistentReconnectReady = false;
  refs.reconnectAuthReply24[0] = 0x24;
  refs.reconnectAuthReply24[1] = 0x00;
  refs.reconnectAuthReply24[2] = refs.defaultReply2401[2];
  refs.reconnectAuthReply24[3] = refs.defaultReply2401[3];
  refs.reconnectAuthReply24[4] = refs.defaultReply2401[4];
  memset(refs.currentAttemptCentralNonce, 0, 4);
  *refs.currentAttemptCentralNonceValid = false;
  memset(refs.persistedAcceptedCentralNonce, 0, 4);
  *refs.persistedAcceptedCentralNonceValid = false;
  *refs.firstPairSawReal36 = false;
  *refs.firstPairSawReal28 = false;
  refs.seedAuthReplyIndexTable();
  refs.clearAuthBlobStage();
  refs.resetAuthParserOutput();
  *refs.firstPairCertState = CERT_STATE_IDLE;
  refs.setManufacturerState(MANUFACTURER_STATE_PAIRABLE);
  refs.refreshPairingExports(false);
}

void ppSetManufacturerState(const PairStateRuntimeRefs& refs, uint8_t state)
{
  refs.manufacturerData[refs.manufacturerDataLen - 1] = state;
}

void ppRefreshPairingExports(const PairStateRuntimeRefs& refs, bool notify)
{
  uint8_t ready = refs.readyStateCode();
  bool exposeCommittedPair = *refs.pairingStage >= PAIR_STAGE_COMMITTED;
  uint8_t pair0 = exposeCommittedPair ? refs.activePair[0] : 0x00;
  uint8_t pair1 = exposeCommittedPair ? refs.activePair[1] : 0x00;

  refs.copyAddressDisplay(refs.serviceARead3);

  refs.serviceARead4[0] = ready;
  refs.serviceARead4[1] = exposeCommittedPair ? 0x01 : 0x00;
  refs.serviceARead4[2] = pair0;
  refs.serviceARead4[3] = pair1;
  refs.serviceARead4[4] = refs.serviceARead3[3];
  refs.serviceARead4[5] = refs.serviceARead3[4];
  refs.serviceARead4[6] = refs.serviceARead3[5];
  refs.serviceARead4[7] = refs.manufacturerData[refs.manufacturerDataLen - 1];

  refs.serviceAStatus[0] = ready;
  refs.serviceAStatus[1] = *refs.pairEventClass;

  refs.serviceBStatus1[0] = ready;
  refs.serviceBStatus1[1] = pair0;
  refs.serviceBStatus1[2] = pair1;
  refs.serviceBStatus1[3] = *refs.pairEventClass;

  refs.serviceBStatus2[0] = exposeCommittedPair ? refs.candidatePair[0] : 0x00;
  refs.serviceBStatus2[1] = exposeCommittedPair ? refs.candidatePair[1] : 0x00;
  refs.serviceBStatus2[2] = exposeCommittedPair ? 0x01 : 0x00;
  refs.serviceBStatus2[3] = ready;

  refs.serviceCState[0] = 0x2c;
  refs.serviceCState[1] = 0xa4;
  refs.serviceCState[2] = *refs.serviceCTransferState;
  refs.serviceCState[3] = ready;
  refs.serviceCState[4] = pair0;
  refs.serviceCState[5] = pair1;

  refs.serviceDStatus[0] = ready;
  refs.serviceDStatus[1] = pair0;
  refs.serviceDStatus[2] = pair1;
  refs.serviceDStatus[3] = *refs.pairEventClass;

  refs.serviceBRead3[0] = ready;
  refs.serviceBRead3[1] = *refs.pairEventClass;

  refs.configState[0] = ready;

  if (!*refs.servicesReady) {
    return;
  }

  refs.serviceAChar3->write(refs.serviceARead3, 6);
  refs.serviceAChar4->write(refs.serviceARead4, 8);
  refs.serviceAChar5->write(refs.serviceAStatus, 2);
  refs.serviceBChar1->write(refs.serviceBStatus1, 4);
  refs.serviceBChar2->write(refs.serviceBStatus2, 4);
  refs.serviceBChar3->write(refs.serviceBRead3, 2);
  refs.serviceCChar1->write(refs.serviceCState, 74);
  refs.serviceDChar2->write(refs.serviceDStatus, 4);
  refs.configChar->write(refs.configState, 1);

  logPairingExportValue("serviceAChar3", refs.serviceARead3, 6);
  logPairingExportValue("serviceAChar4", refs.serviceARead4, 8);
  logPairingExportValue("serviceAChar5", refs.serviceAStatus, 2);
  logPairingExportValue("serviceBChar1", refs.serviceBStatus1, 4);
  logPairingExportValue("serviceBChar2", refs.serviceBStatus2, 4);
  logPairingExportValue("serviceBChar3", refs.serviceBRead3, 2);
  logPairingExportValue("serviceDChar2", refs.serviceDStatus, 4);
  logPairingExportValue("configChar", refs.configState, 1);

  if (notify && *refs.activeConnHandle != BLE_CONN_HANDLE_INVALID && *refs.pairingStage >= PAIR_STAGE_COMMITTED) {
    if (*refs.serviceCNotifyEnabled) {
      refs.serviceCChar1->notify(refs.serviceCState, 74);
    }
    if (*refs.serviceDNotifyEnabled) {
      logPairingExportValue("serviceDChar2 notify", refs.serviceDStatus, 4);
      refs.serviceDChar2->notify(refs.serviceDStatus, 4);
    }
  }
}
