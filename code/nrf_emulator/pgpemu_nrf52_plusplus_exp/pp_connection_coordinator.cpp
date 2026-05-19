#include <Arduino.h>

#include "pp_auth_protocol.h"
#include "pp_connection_coordinator.h"

void ppConnectionCoordinatorHandleServiceDNotifyEnabled(const BleDispatchRefs& refs)
{
  if (*refs.serviceDNotifyEnabled && *refs.freshAuthStarted && *refs.freshAuthStep == 32) {
    *refs.sfidaCertStage = 0;
    refs.beginServiceDOlderFlow();
    *refs.freshAuthStep = 33;
    *refs.freshAuthAt = millis() + refs.freshAuthWaitFor28WriteMs;
    return;
  }
  if (*refs.serviceDNotifyEnabled &&
      *refs.pairingStage == PAIR_STAGE_AUTHENTICATED &&
      *refs.reconnectServiceDArmed) {
    *refs.sfidaCertStage = 0;
    Serial.println("Reconnect ServiceD CCCD enabled -> starting compact reconnect lane");
    refs.beginServiceDCertFlow();
  }
}

bool ppConnectionCoordinatorHandleServiceDWrite(const BleDispatchRefs& refs,
                                                const uint8_t* data,
                                                uint16_t len)
{
  if (refs.serviceDCertState->active && *refs.pairingStage >= PAIR_STAGE_COMMITTED) {
    if (refs.handleServiceDCertWrite(data, len)) {
      *refs.sfidaCertStage = refs.serviceDCertState->laneStep;
    } else {
      Serial.println("SFIDA write not handled by typed donor cert flow");
    }
    return true;
  }
  return false;
}

ConnectionCoordinatorAuthResult ppConnectionCoordinatorHandleAuthWrite(const BleDispatchRefs& refs,
                                                                       const uint8_t* data,
                                                                       uint16_t len)
{
  if (len >= 1 && ppAuthProtocolIsAuthenticatedSleepOpcode(data[0])) {
    SleepExportWriteResult sleepResult = refs.handleAuthenticatedSleepWrite(data, len);
    if (sleepResult == SLEEP_EXPORT_HANDLED_AND_STORE) {
      return CONNECTION_COORDINATOR_AUTH_HANDLED_AND_STORE;
    }
    if (sleepResult == SLEEP_EXPORT_HANDLED_NO_STORE) {
      return CONNECTION_COORDINATOR_AUTH_HANDLED_NO_STORE;
    }
  }

  if (ppAuthProtocolIsIndexed24Write(data, len)) {
    if (*refs.pairingStage == PAIR_STAGE_AUTHENTICATED || *refs.persistentReconnectReady) {
      refs.handleReconnectAuthWrite24(data[1], "auth write 24 indexed");
    } else {
      refs.handleFirstPairAuthWrite(data, len);
    }
    return CONNECTION_COORDINATOR_AUTH_HANDLED_NO_STORE;
  }

  if (ppAuthProtocolIsFirstPairControlWrite(data, len)) {
    refs.handleFirstPairAuthWrite(data, len);
    return CONNECTION_COORDINATOR_AUTH_HANDLED_NO_STORE;
  }

  if (ppAuthProtocolIsAuth25Write(data, len)) {
    if (*refs.pairingStage < PAIR_STAGE_COMMITTED) {
      Serial.println("Auth 25 gated: pair not committed yet");
      return CONNECTION_COORDINATOR_AUTH_HANDLED_NO_STORE;
    }
    if (*refs.freshAuthBlobSent) {
      Serial.println("Auth step 25 ack after blob -> waiting for next fresh auth write");
      return CONNECTION_COORDINATOR_AUTH_HANDLED_NO_STORE;
    }
    refs.authControl->notify(refs.authCtrlReply25, refs.authCtrlReply25Len);
    refs.setAuthParserSchedulerState(1, 4, 2, 2, AUTH_RESULT_CONTINUE, 2, 0x25, true, true, false);
    if (*refs.pairingStage == PAIR_STAGE_AUTHENTICATED || *refs.persistentReconnectReady) {
      refs.handleReconnectAuthWrite25(refs.authCtrlReply25,
                                      refs.authCtrlReply25Len,
                                      "reconnect auth write 25");
      return CONNECTION_COORDINATOR_AUTH_HANDLED_NO_STORE;
    }
    refs.handleFirstPairAuthWrite(data, len);
    return CONNECTION_COORDINATOR_AUTH_HANDLED_NO_STORE;
  }

  return CONNECTION_COORDINATOR_AUTH_NOT_HANDLED;
}
