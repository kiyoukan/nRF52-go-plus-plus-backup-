#include <Arduino.h>
#include <string.h>

#include "pp_service_d_local.h"
#include "pp_service_d_protocol.h"

void ppBuildServiceDType3PayloadFromPersistedState(const ServiceDLocalRefs& refs)
{
  memcpy(refs.serviceDType3ReadPayload,
         refs.donorServiceDReadType3,
         refs.donorServiceDReadType3Len);
}

void ppRefreshServiceDLocalPayloadsFromRuntime(const ServiceDLocalRefs& refs)
{
  refs.rebuildServiceDRuntimeBlock();
  ppBuildServiceDType3PayloadFromPersistedState(refs);
  memset(refs.serviceDType5ReadPayload, 0, refs.serviceDType5ReadPayloadLen);
  if (refs.serviceDType5ReadPayloadLen >= 4) {
    refs.serviceDType5ReadPayload[0] = 0x05;
  }
}

void ppNotifyServiceDStatus(const ServiceDLocalRefs& refs,
                            const uint8_t* status,
                            const char* label)
{
  memcpy(refs.serviceDStatus, status, refs.serviceDStatusLen);
  memcpy(refs.serviceDCertState->lastStatus, status, sizeof(refs.serviceDCertState->lastStatus));
  if (*refs.serviceDReadPendingLen > 0) {
    refs.serviceDChar2->write(refs.serviceDReadBlob, *refs.serviceDReadPendingLen);
    refs.serviceDChar3->write(refs.serviceDReadBlob, *refs.serviceDReadPendingLen);
  } else {
    refs.serviceDChar2->write(refs.serviceDStatus, refs.serviceDStatusLen);
  }
  uint16_t valueHandle = refs.serviceDChar2->handles().value_handle;
  uint16_t packetLen = static_cast<uint16_t>(refs.serviceDStatusLen);
  ble_gatts_hvx_params_t hvxParams;
  memset(&hvxParams, 0, sizeof(hvxParams));
  hvxParams.handle = valueHandle;
  hvxParams.type = BLE_GATT_HVX_NOTIFICATION;
  hvxParams.offset = 0;
  hvxParams.p_len = &packetLen;
  hvxParams.p_data = refs.serviceDStatus;
  uint32_t rawStatus = NRF_ERROR_INVALID_STATE;
  uint32_t sysAttrStatus = NRF_SUCCESS;
  if (*refs.activeConnHandle != BLE_CONN_HANDLE_INVALID) {
    rawStatus = sd_ble_gatts_hvx(*refs.activeConnHandle, &hvxParams);
    if (rawStatus == BLE_ERROR_GATTS_SYS_ATTR_MISSING) {
      sysAttrStatus = sd_ble_gatts_sys_attr_set(*refs.activeConnHandle, nullptr, 0, 0);
      packetLen = static_cast<uint16_t>(refs.serviceDStatusLen);
      rawStatus = sd_ble_gatts_hvx(*refs.activeConnHandle, &hvxParams);
    }
  }
  Serial.print("Fresh auth ServiceD -> ");
  Serial.print(label);
  Serial.print(" raw_hvx=0x");
  Serial.print(rawStatus, HEX);
  if (sysAttrStatus != NRF_SUCCESS) {
    Serial.print(" sys_attr=0x");
    Serial.print(sysAttrStatus, HEX);
  }
  Serial.println();
  if (*refs.serviceDReadPendingLen > 0) {
    Serial.print("ServiceD staged read blob published on char2/char3 len=0x");
    Serial.println(*refs.serviceDReadPendingLen, HEX);
  }
}

void ppPrimePostServiceDHandoff(const ServiceDLocalRefs& refs)
{
  *refs.postServiceDHandoffActive = true;
  *refs.postServiceDServiceBReadSeen = false;
  *refs.postServiceDServiceBValue = 0x04;
  refs.serviceBChar1->write(refs.postServiceDServiceBValue, sizeof(*refs.postServiceDServiceBValue));
  Serial.println("Post-ServiceD handoff primed: ServiceB char1 -> 04");
}

void ppClearPostServiceDHandoff(const ServiceDLocalRefs& refs)
{
  if (!*refs.postServiceDHandoffActive) {
    return;
  }
  *refs.postServiceDHandoffActive = false;
  *refs.postServiceDServiceBReadSeen = false;
  refs.refreshPairingExports(false);
  Serial.println("Post-ServiceD handoff cleared");
}

bool ppHandleServiceDCertWrite(const ServiceDLocalRefs& refs,
                               const uint8_t* data,
                               uint16_t len)
{
  if (!refs.serviceDCertState->active) {
    return false;
  }

  uint8_t messageType = 0xff;
  if (!ppServiceDProtocolParseTypedWrite(data, len, &messageType)) {
    Serial.print("SFIDA invalid header len=0x");
    Serial.print(len, HEX);
    Serial.println(" expected typed write");
    refs.logServiceDCertState("invalid header");
    return true;
  }

  if (!ppServiceDProtocolValidateExpectedWrite(refs.serviceDCertState, messageType, len)) {
    Serial.print("SFIDA unexpected typed write state=");
    Serial.print(refs.serviceDCertState->laneStep);
    Serial.print(" expected(type=");
    Serial.print(refs.serviceDCertState->expectedWriteType);
    Serial.print(",len=0x");
    Serial.print(refs.serviceDCertState->expectedWriteLen, HEX);
    Serial.print(") observed(type=");
    Serial.print(messageType);
    Serial.print(",len=0x");
    Serial.print(len, HEX);
    Serial.println(")");
    refs.logServiceDCertState("unexpected typed write");
    return true;
  }

  if (refs.serviceDCertState->laneKind == SERVICE_D_FLOW_OLDER) {
    return refs.handleOlderFlowWrite(data, len);
  }

  return refs.handleReconnectFlowWrite(data, len);
}
