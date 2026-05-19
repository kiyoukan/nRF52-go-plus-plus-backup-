#include <Arduino.h>
#include <string.h>

#include "pp_service_d_protocol.h"

bool ppServiceDProtocolParseTypedWrite(const uint8_t* data,
                                       uint16_t len,
                                       uint8_t* typeOut)
{
  if (typeOut == nullptr || data == nullptr || len < 4) {
    return false;
  }
  if (data[1] != 0x00 || data[2] != 0x00 || data[3] != 0x00) {
    return false;
  }
  *typeOut = data[0];
  return true;
}

bool ppServiceDProtocolValidateExpectedWrite(ServiceDCertState* state,
                                             uint8_t messageType,
                                             uint16_t len)
{
  if (state == nullptr) {
    return false;
  }
  state->lastObservedType = messageType;
  state->lastObservedLen = len;
  return messageType == state->expectedWriteType && len == state->expectedWriteLen;
}

void ppServiceDProtocolBeginOlderLane(const FirstPairOlderServiceDRefs& refs,
                                      const uint8_t* status0)
{
  refs.serviceDCertState->active = true;
  refs.serviceDCertState->laneKind = SERVICE_D_FLOW_OLDER;
  refs.serviceDCertState->completionPending = false;
  refs.serviceDCertState->lastObservedType = 0xff;
  refs.serviceDCertState->lastObservedLen = 0;
  *refs.lastOlderServiceDType2WriteValid = false;
  refs.refreshServiceDLocalPayloadsFromRuntime();
  refs.prepareOlderServiceDReadStage(0);
  refs.notifyServiceDStatus(status0, "ServiceD older 00 00 00 00");
  refs.armServiceDCertExpectation(5, 0x01, 0x14, "serviceD older begin");
  *refs.reconnectServiceDArmed = false;
  *refs.reconnectCompactServiceDMode = false;
}

bool ppServiceDProtocolHandleOlderLaneWrite(const FirstPairOlderServiceDRefs& refs,
                                            const uint8_t* data,
                                            uint16_t len,
                                            const uint8_t* status1,
                                            const uint8_t* status2,
                                            const uint8_t* status4,
                                            uint32_t waitForWriteMs)
{
  if (refs.serviceDCertState->laneKind != SERVICE_D_FLOW_OLDER) {
    return false;
  }

  if (refs.serviceDCertState->laneStep == 5) {
    Serial.println("ServiceD older state 5 -> observed type 1 len=0x14");
    refs.prepareOlderServiceDReadStage(1);
    refs.notifyServiceDStatus(status1, "ServiceD older 01 00 00 00");
    refs.armServiceDCertExpectation(6, 0x02, 0x34, "serviceD older expect type2");
    return true;
  }

  if (refs.serviceDCertState->laneStep == 6) {
    Serial.println("ServiceD older state 6 -> observed type 2 len=0x34");
    memcpy(refs.lastOlderServiceDType2Write, data, refs.lastOlderServiceDType2WriteLen);
    *refs.lastOlderServiceDType2WriteValid = true;
    refs.prepareOlderServiceDReadStage(2);
    refs.incrementServiceDRuntimeTailCounter();
    refs.notifyServiceDStatus(status2, "ServiceD older 02 00 00 00");
    refs.armServiceDCertExpectation(7, 0x00, 0x34, "serviceD older expect type0");
    return true;
  }

  if (refs.serviceDCertState->laneStep == 7) {
    Serial.println("ServiceD older state 7 -> observed type 0 len=0x34");
    refs.notifyServiceDStatus(status4, "ServiceD older 04 00 23 00");
    refs.serviceDCertState->completionPending = true;
    refs.armServiceDCertExpectation(8, 0x00, 0x00, "serviceD older complete");
    *refs.freshAuthStep = 34;
    *refs.freshAuthAt = millis() + waitForWriteMs;
    return true;
  }

  Serial.print("ServiceD older typed write observed in terminal state=");
  Serial.println(refs.serviceDCertState->laneStep);
  refs.logServiceDCertState("older terminal typed write");
  return true;
}

void ppServiceDProtocolBeginCompactLane(const ReconnectCompactServiceDRefs& refs,
                                        const uint8_t* statusStart,
                                        uint32_t waitForWriteMs)
{
  (void) waitForWriteMs;
  *refs.reconnectCompactServiceDMode = *refs.reconnectServiceDArmed;
  refs.serviceDCertState->active = true;
  refs.serviceDCertState->laneKind = SERVICE_D_FLOW_COMPACT;
  refs.serviceDCertState->completionPending = false;
  refs.serviceDCertState->lastObservedType = 0xff;
  refs.serviceDCertState->lastObservedLen = 0;
  *refs.lastOlderServiceDType2WriteValid = false;
  refs.refreshServiceDLocalPayloadsFromRuntime();
  refs.prepareSfidaReadChallenge(1);
  refs.notifyServiceDStatus(statusStart,
                            *refs.reconnectCompactServiceDMode ? "SFIDA reconnect 03 00 00 00"
                                                               : "SFIDA 03 00 00 00");
  refs.armServiceDCertExpectation(1, 0x04, 0x14,
                                  *refs.reconnectCompactServiceDMode ? "serviceD compact reconnect begin"
                                                                     : "serviceD compact begin");
  *refs.reconnectServiceDArmed = false;
}

bool ppServiceDProtocolHandleCompactLaneWrite(const ReconnectCompactServiceDRefs& refs,
                                              const uint8_t* data,
                                              uint16_t len,
                                              const uint8_t* expectedWrite3Done,
                                              size_t expectedWrite3DoneLen,
                                              const uint8_t* statusStage1,
                                              const uint8_t* statusStage2,
                                              const uint8_t* statusDone,
                                              const uint8_t* statusFail,
                                              uint32_t waitForWriteMs)
{
  (void) len;
  if (refs.serviceDCertState->laneKind != SERVICE_D_FLOW_COMPACT) {
    return false;
  }

  if (refs.serviceDCertState->laneStep == 1) {
    Serial.println(*refs.reconnectCompactServiceDMode
                     ? "SFIDA state 1 reconnect compact type4 observed len=0x14"
                     : "SFIDA state 1 compact type4 observed len=0x14");
    refs.notifyServiceDStatus(statusStage1, "SFIDA 04 00 01 00");
    refs.armServiceDCertExpectation(2, 0x05, 0x24, "serviceD compact expect type5");
    return true;
  }

  if (refs.serviceDCertState->laneStep == 2) {
    Serial.println("SFIDA state 2 -> observed type 5 len=0x24");
    refs.buildServiceDType5PayloadFromLiveWrite(data);
    refs.notifyServiceDStatus(statusStage2, "SFIDA 05 00 00 00");
    refs.prepareSfidaReadChallenge(2);
    refs.armServiceDCertExpectation(3, 0x03, 0x05, "serviceD compact expect type3 completion");
    return true;
  }

  if (refs.serviceDCertState->laneStep == 3) {
    bool type3DoneMatches = (memcmp(data, expectedWrite3Done, expectedWrite3DoneLen) == 0);
    Serial.print("SFIDA state 3 compact type3-done match=");
    Serial.println(type3DoneMatches ? "true" : "false");
    Serial.println("SFIDA state 3 -> observed type 3 len=0x05");
    if (!type3DoneMatches) {
      refs.notifyServiceDStatus(statusFail, "SFIDA 04 00 03 00");
      refs.serviceDCertState->completionPending = false;
      refs.armServiceDCertExpectation(3, 0x03, 0x05, "serviceD compact completion rejected");
      *refs.freshAuthStep = 33;
      *refs.freshAuthAt = millis() + waitForWriteMs;
      return true;
    }
    refs.notifyServiceDStatus(statusDone, "SFIDA 04 00 02 00");
    // Device B leaves ServiceB char1 readable as 04 after compact ServiceD.
    // The app treats an unsolicited reconnect-ready notify here as low battery.
    *refs.postServiceDHandoffActive = true;
    *refs.postServiceDServiceBReadSeen = false;
    *refs.postServiceDServiceBValue = 0x04;
    refs.serviceBChar1->write(refs.postServiceDServiceBValue, sizeof(*refs.postServiceDServiceBValue));
    Serial.println("Reconnect ServiceB handoff primed: char1 read -> 04");
    refs.serviceDCertState->completionPending = true;
    refs.armServiceDCertExpectation(4, 0x00, 0x00, "serviceD compact complete");
    *refs.freshAuthStep = 34;
    *refs.freshAuthAt = millis() + waitForWriteMs;
    return true;
  }

  Serial.print("SFIDA typed write observed in terminal state=");
  Serial.println(refs.serviceDCertState->laneStep);
  refs.logServiceDCertState("terminal typed write");
  return true;
}

bool ppServiceDProtocolProcessCompactCompletion(const ReconnectCompactServiceDRefs& refs)
{
  if (*refs.pairingStage == PAIR_STAGE_AUTHENTICATED &&
      refs.serviceDCertState->laneKind == SERVICE_D_FLOW_COMPACT &&
      refs.serviceDCertState->completionPending) {
    // Keep the 04 handoff readable, then suppress the old 01 ready notify for Device B parity.
    Serial.println("Reconnect SFIDA complete -> ServiceB ready notify suppressed for Device B parity");
    *refs.postServiceDHandoffActive = false;
    *refs.postServiceDServiceBReadSeen = false;
    Serial.println("Reconnect SFIDA complete -> returning to authenticated idle");
    *refs.freshAuthStep = 0;
    *refs.freshAuthAt = 0;
    refs.resetServiceDCertState();
    return true;
  }

  return false;
}
