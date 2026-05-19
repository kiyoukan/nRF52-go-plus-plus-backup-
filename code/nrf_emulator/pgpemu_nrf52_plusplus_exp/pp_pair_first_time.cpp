#include <Arduino.h>
#include <string.h>

#include "pp_auth_protocol.h"
#include "pp_pair_first_time.h"
#include "pp_service_d_protocol.h"

bool ppHandleFirstPairAuthWrite(const FirstPairRuntimeRefs& refs,
                                const uint8_t* data,
                                uint16_t len)
{
  return ppAuthProtocolHandleFirstPairWrite(refs, data, len);
}

void ppStageCandidatePair(const FirstPairRuntimeRefs& refs, const char* reason)
{
  if (*refs.pairingStage != PAIR_STAGE_DISCOVERY) {
    return;
  }

  refs.stagedPair[0] = refs.candidatePair[0];
  refs.stagedPair[1] = refs.candidatePair[1];
  *refs.stagedPairValid = true;
  *refs.serviceCTransferState = 0x02;
  *refs.pairEventClass = 0x23;
  *refs.pairingStage = PAIR_STAGE_STAGED;

  Serial.print("Pair stage -> ");
  Serial.print(refs.pairingStageLabel(*refs.pairingStage));
  Serial.print(" via ");
  Serial.println(reason);

  refs.refreshPairingExports(false);
  refs.scheduleServiceCFirstPairBurst();
}

void ppCommitStagedPair(const FirstPairRuntimeRefs& refs, const char* reason)
{
  if (!*refs.stagedPairValid || *refs.pairingStage < PAIR_STAGE_STAGED) {
    return;
  }

  refs.activePair[0] = refs.stagedPair[0];
  refs.activePair[1] = refs.stagedPair[1];
  *refs.stagedPairValid = false;
  *refs.serviceCTransferState = 0x03;
  *refs.pairEventClass = 0x24;
  *refs.pairingStage = PAIR_STAGE_COMMITTED;

  Serial.print("Pair stage -> ");
  Serial.print(refs.pairingStageLabel(*refs.pairingStage));
  Serial.print(" via ");
  Serial.println(reason);

  refs.refreshPairingExports(true);
}

void ppMarkAuthComplete(const FirstPairRuntimeRefs& refs)
{
  refs.clearPostServiceDHandoff();
  refs.clearReconnectServiceDArm();
  *refs.freshAuthStep = 0;
  *refs.freshAuthAt = 0;
  *refs.freshAuthStarted = false;
  *refs.freshAuthBlobSent = false;
  *refs.pairingStage = PAIR_STAGE_AUTHENTICATED;
  *refs.pairEventClass = 0x25;
  *refs.persistentReconnectReady = true;
  if (*refs.firstPairSawReal36 && *refs.firstPairSawReal28 && *refs.currentAttemptCentralNonceValid) {
    memcpy(refs.persistedAcceptedCentralNonce,
           refs.currentAttemptCentralNonce,
           4);
    *refs.persistedAcceptedCentralNonceValid = true;
    refs.updateAuthReplyIndexSlot(1, refs.persistedAcceptedCentralNonce, "auth complete persist");
    Serial.print("Persisted accepted first-pair nonce=");
    refs.printHex(refs.persistedAcceptedCentralNonce, 4);
    Serial.println();
  }
  refs.refreshReconnectAuthReply24("auth complete");
  refs.armPostAuthRuntimeScaffolding();
  refs.resetAuthParserOutput();
  refs.setManufacturerState(MANUFACTURER_STATE_RECONNECT);
  refs.refreshPairingExports(false);
  refs.setFirstPairCertState(CERT_STATE_AUTH_COMPLETE, "markAuthComplete");
  refs.savePersistentPairState("markAuthComplete");
  Serial.println("Pair stage -> authenticated");
  Serial.print("First-pair real 36 seen=");
  Serial.print(*refs.firstPairSawReal36 ? "true" : "false");
  Serial.print(" real 28 seen=");
  Serial.println(*refs.firstPairSawReal28 ? "true" : "false");
}

void ppScheduleFreshAuthSequence(const FirstPairRuntimeRefs& refs,
                                 uint8_t firstStep,
                                 const char* reason)
{
  if (*refs.activeConnHandle == BLE_CONN_HANDLE_INVALID) {
    return;
  }

  if (!*refs.authControlNotifyEnabled && !*refs.authBlobNotifyEnabled) {
    return;
  }

  if (*refs.pairingStage == PAIR_STAGE_DISCOVERY) {
    refs.stageCandidatePair(reason);
  }
  if (*refs.pairingStage == PAIR_STAGE_STAGED) {
    refs.commitStagedPair(reason);
  }

  *refs.freshAuthStep = firstStep;
  *refs.freshAuthAt = millis() + (firstStep == 10 ? refs.freshAuth18ToReplyDelayMs : refs.freshAuthStepDelayMs);
  *refs.freshAuthStarted = true;
  Serial.print("Scheduled fresh auth step ");
  Serial.print(firstStep);
  Serial.print(" via ");
  Serial.println(reason);
}

void ppProcessFreshAuthSequence(const FirstPairRuntimeRefs& refs)
{
  if (*refs.activeConnHandle == BLE_CONN_HANDLE_INVALID || *refs.freshAuthStep == 0) {
    return;
  }

  if (static_cast<int32_t>(millis() - *refs.freshAuthAt) < 0) {
    return;
  }

  switch (*refs.freshAuthStep) {
    case 10:
      if (!*refs.authControlNotifyEnabled) {
        *refs.freshAuthAt = millis() + refs.freshAuthStepDelayMs;
        return;
      }
      refs.donorAuthRuntime->mode = 3;
      refs.donorAuthRuntime->workerActive = true;
      refs.donorAuthRuntime->readinessByte = 0;
      refs.donorAuthRuntime->specialArmed = false;
      refs.donorAuthRuntime->pendingLatch = false;
      refs.donorAuthRuntime->localFamily = refs.deriveDonorLocalFamilyFromParserBytes();
      refs.donorAuthRuntime->localStatus = refs.donorAuthRuntime->auth18Payload1;
      refs.donorAuthRuntime->localCommandCode = refs.donorLocal29CommandCodeForFamily(refs.donorAuthRuntime->localFamily);
      refs.setAuthParserSchedulerState(1, 4, 2, 2, AUTH_RESULT_CONTINUE, 1, 0x18, true, false, false);
      refs.logDonorAuthRuntime("app 18 parsed");
      refs.queueDonorAuthRuntimeStep(DONOR_AUTH_RUNTIME_30000, refs.freshAuth18ToReplyDelayMs, "mode3 event 0x30000");
      return;

    case 11:
      Serial.println("Fresh auth waiting for app write 16 before device 16");
      *refs.freshAuthAt = millis() + refs.freshAuthWaitFor28WriteMs;
      return;

    case 12:
      Serial.println("Fresh auth waiting for app write 28 after 16");
      *refs.freshAuthStep = 19;
      *refs.freshAuthAt = millis() + refs.freshAuthWaitFor28WriteMs;
      return;

    case 19:
      Serial.println("Fresh auth app write 28 watchdog expired; leaving auth open");
      *refs.freshAuthAt = millis() + refs.freshAuthWaitFor28WriteMs;
      return;

    case 30:
      if (!*refs.authControlNotifyEnabled) {
        *refs.freshAuthAt = millis() + refs.freshAuthStepDelayMs;
        return;
      }
      Serial.println("Fresh auth still waiting for real app write 36; no forced fallback");
      *refs.freshAuthAt = millis() + refs.freshAuthWaitFor28WriteMs;
      return;

    case 31:
      Serial.println("Fresh auth app write 28 watchdog expired after 36; leaving auth open");
      *refs.freshAuthAt = millis() + refs.freshAuthWaitFor28WriteMs;
      return;

    case 32:
      Serial.println("Fresh auth waiting for ServiceD CCCD enable after real 28");
      *refs.freshAuthAt = millis() + refs.freshAuthWaitFor28WriteMs;
      return;

    case 33:
      Serial.println("SFIDA certification watchdog: waiting for next read/write");
      *refs.freshAuthAt = millis() + refs.freshAuthWaitFor28WriteMs;
      return;

    case 34:
      if (refs.processReconnectCompactCompletion()) {
        return;
      }
      Serial.println("SFIDA complete; waiting for donor post-ServiceD handoff or app write 26");
      *refs.freshAuthAt = millis() + refs.freshAuthWaitFor28WriteMs;
      return;

    case 35:
      if (!*refs.authControlNotifyEnabled) {
        *refs.freshAuthAt = millis() + refs.freshAuthStepDelayMs;
        return;
      }
      refs.authControl->notify(refs.authReply16, refs.authReply16Len);
      Serial.println("Fresh auth delayed -> 16 00 00 00 00 00; waiting for app write 36");
      refs.donorAuthRuntime->readinessByte = 1;
      refs.donorAuthRuntime->workerActive = true;
      refs.authParserOutput->lastOpcode = 0x16;
      refs.authParserOutput->resultCode = AUTH_RESULT_SUCCESS;
      refs.authParserOutput->outputPending = true;
      refs.logDonorAuthRuntime("device 16 ack");
      refs.setFirstPairCertState(CERT_STATE_DEVICE_16_ACKED, "device 16 ack");
      refs.queueDonorAuthRuntimeStep(DONOR_AUTH_RUNTIME_EVENT3_READY, refs.freshAuthStepDelayMs, "mode3 readiness after 16");
      return;

    case DONOR_AUTH_RUNTIME_30000:
      if (!*refs.authControlNotifyEnabled) {
        *refs.freshAuthAt = millis() + refs.freshAuthStepDelayMs;
        return;
      }
      refs.donorAuthRuntime->event30000Seen = true;
      if (!refs.donorAuthRuntime->auth18Observed || refs.donorAuthRuntime->auth18Selector >= 0x20) {
        refs.authControl->notify(refs.authReply18Fail, refs.authReply18FailLen);
        Serial.println("Fresh auth worker event 0x30000 -> 18 05");
        refs.authParserOutput->lastOpcode = 0x18;
        refs.authParserOutput->resultCode = AUTH_RESULT_FAILURE;
        refs.authParserOutput->outputPending = true;
        refs.queueDonorAuthRuntimeStep(30, refs.freshAuthWaitFor28WriteMs, "18 failure waiting");
        return;
      }
      refs.authControl->notify(refs.authReply18, refs.authReply18Len);
      Serial.println("Fresh auth worker event 0x30000 -> 18 00");
      refs.authParserOutput->lastOpcode = 0x18;
      refs.authParserOutput->resultCode = AUTH_RESULT_SUCCESS;
      refs.authParserOutput->outputPending = true;
      refs.setFirstPairCertState(CERT_STATE_DEVICE_18_ACKED, "device 18 ack");
      refs.queueDonorAuthRuntimeStep(DONOR_AUTH_RUNTIME_EVENT3_WAIT, refs.freshAuthStepDelayMs, "mode3 event 3 after 18");
      return;

    case DONOR_AUTH_RUNTIME_EVENT3_WAIT:
      if (!*refs.authControlNotifyEnabled) {
        *refs.freshAuthAt = millis() + refs.freshAuthStepDelayMs;
        return;
      }
      refs.donorAuthRuntime->event3Seen = true;
      refs.authControl->notify(refs.authReply21Wait, refs.authReply21WaitLen);
      Serial.println("Fresh auth worker event 3 -> 21 00 02");
      refs.authParserOutput->lastOpcode = 0x21;
      refs.authParserOutput->resultCode = AUTH_RESULT_SUCCESS;
      refs.authParserOutput->outputClass = 4;
      refs.authParserOutput->outputLen = refs.authReply21WaitLen;
      refs.authParserOutput->outputPending = true;
      refs.logDonorAuthRuntime("event 3 wait");
      refs.queueDonorAuthRuntimeStep(DONOR_AUTH_RUNTIME_30001, refs.freshAuthStepDelayMs, "mode3 event 0x30001");
      return;

    case DONOR_AUTH_RUNTIME_30001:
      refs.donorAuthRuntime->event30001Seen = true;
      refs.donorAuthRuntime->specialArmed = true;
      Serial.println("Fresh auth monitor event 0x30001 -> donor special flag armed");
      refs.logDonorAuthRuntime("event 0x30001");
      if (refs.donorAuthRuntime->app16Latched) {
        Serial.println("Fresh auth latched app write 16 -> scheduling delayed device 16 00 00 00 00 00");
        *refs.freshAuthStep = 35;
        *refs.freshAuthAt = millis() + refs.freshAuth18To16DelayMs;
      } else {
        *refs.freshAuthStep = 11;
        *refs.freshAuthAt = millis() + refs.freshAuthWaitFor28WriteMs;
      }
      return;

    case DONOR_AUTH_RUNTIME_EVENT3_READY:
      if (!*refs.authControlNotifyEnabled) {
        *refs.freshAuthAt = millis() + refs.freshAuthStepDelayMs;
        return;
      }
      refs.donorAuthRuntime->event3Seen = true;
      refs.authControl->notify(refs.authReply21Ready, refs.authReply21ReadyLen);
      Serial.println("Fresh auth readiness -> 21 00 01");
      refs.authParserOutput->lastOpcode = 0x21;
      refs.authParserOutput->resultCode = AUTH_RESULT_SUCCESS;
      refs.authParserOutput->outputClass = 4;
      refs.authParserOutput->outputLen = refs.authReply21ReadyLen;
      refs.authParserOutput->outputPending = true;
      refs.logDonorAuthRuntime("event 3 ready");
      refs.queueDonorAuthRuntimeStep(DONOR_AUTH_RUNTIME_EVENT4, refs.freshAuthStepDelayMs, "mode3 internal event 4");
      return;

    case DONOR_AUTH_RUNTIME_EVENT4:
      refs.donorAuthRuntime->event4Seen = true;
      Serial.println("Fresh auth internal event 4 -> local readiness satisfied");
      if (!refs.donorAuthRuntime->pendingLatch && refs.donorAuthRuntime->auth18Observed) {
        refs.donorAuthRuntime->localFamily = refs.deriveDonorLocalFamilyFromParserBytes();
        refs.donorAuthRuntime->localCommandCode = refs.donorLocal29CommandCodeForFamily(refs.donorAuthRuntime->localFamily);
        refs.donorAuthRuntime->pendingLatch = true;
        refs.donorAuthRuntime->localStatus = 0x00;
      }
      if (refs.donorAuthRuntime->pendingLatch) {
        refs.queueDonorAuthRuntimeStep(DONOR_AUTH_RUNTIME_LOCAL29, refs.freshAuthStepDelayMs, "mode3 local 0x29 pending");
        refs.logDonorAuthRuntime("event 4 pending local 0x29");
        return;
      }
      refs.logDonorAuthRuntime("event 4");
      *refs.freshAuthStep = 30;
      *refs.freshAuthAt = millis() + refs.freshAuthWaitFor28WriteMs;
      return;

    case DONOR_AUTH_RUNTIME_LOCAL29:
      if (!*refs.authControlNotifyEnabled) {
        *refs.freshAuthAt = millis() + refs.freshAuthStepDelayMs;
        return;
      }
      refs.donorAuthRuntime->pendingLatch = true;
      if (refs.donorAuthRuntime->localFamily <= 0x03) {
        refs.donorAuthRuntime->localStatus = refs.donorAuthRuntime->readinessByte ? 0x00 : 0x0A;
      } else if (refs.donorAuthRuntime->localFamily >= 0x10 && refs.donorAuthRuntime->localFamily <= 0x12) {
        refs.donorAuthRuntime->localStatus = refs.donorAuthRuntime->readinessByte ? 0x00 : 0x0A;
      } else if (refs.donorAuthRuntime->localFamily == 0x20 || refs.donorAuthRuntime->localFamily == 0x21) {
        refs.donorAuthRuntime->localStatus = refs.donorAuthRuntime->event3Seen ? 0x00 : 0x09;
      } else {
        refs.donorAuthRuntime->localStatus = 0x09;
      }
      refs.donorAuthRuntime->localCommandCode = refs.donorLocal29CommandCodeForFamily(refs.donorAuthRuntime->localFamily);
      if (refs.donorAuthRuntime->localStatus == 0x00) {
        Serial.println("Fresh auth internal local status -> 29 00 (suppressed)");
      } else if (refs.donorAuthRuntime->localStatus == 0x09) {
        Serial.println("Fresh auth internal local status -> 29 09 (suppressed)");
      } else {
        Serial.println("Fresh auth internal local status -> 29 0A (suppressed)");
      }
      refs.resetAuthParserOutput();
      refs.donorAuthRuntime->pendingLatch = false;
      refs.donorAuthRuntime->workerActive = false;
      refs.logDonorAuthRuntime("local 0x29 consumed");
      *refs.freshAuthStep = 30;
      *refs.freshAuthAt = millis() + refs.freshAuthWaitFor28WriteMs;
      return;

    default:
      *refs.freshAuthStep = 0;
      return;
  }
}

void ppScheduleServiceCFirstPairBurst(const FirstPairRuntimeRefs& refs)
{
  if (*refs.pairingStage < PAIR_STAGE_STAGED) {
    return;
  }

  if (!*refs.serviceCNotifyEnabled) {
    return;
  }

  *refs.serviceCBurstStep = 1;
  *refs.serviceCBurstAt = millis() + refs.firstPairServiceCNotify1DelayMs;
}

void ppProcessServiceCFirstPairBurst(const FirstPairRuntimeRefs& refs)
{
  if (*refs.activeConnHandle == BLE_CONN_HANDLE_INVALID || *refs.serviceCBurstStep == 0) {
    return;
  }

  if (!*refs.serviceCNotifyEnabled) {
    *refs.serviceCBurstAt = millis() + refs.freshAuthDelayMs;
    return;
  }

  if (static_cast<int32_t>(millis() - *refs.serviceCBurstAt) < 0) {
    return;
  }

  uint8_t payload[74] = { 0 };
  payload[0] = 0x2c;
  payload[1] = 0xa4;
  payload[2] = 0x20 + *refs.serviceCBurstStep;
  payload[3] = *refs.serviceCTransferState;
  payload[4] = refs.readyStateCode();
  payload[5] = *refs.pairEventClass;
  payload[6] = refs.plusplusManufacturerData[16];

  for (uint8_t i = 0; i < refs.realPlusPlusAddrLeLen; ++i) {
    payload[8 + i] = refs.realPlusPlusAddrLe[i];
  }

  for (uint16_t i = 14; i < refs.serviceCNotifyLen - 2; ++i) {
    uint8_t cert = refs.factoryCertBlock[(i + *refs.serviceCBurstStep * 11U) % refs.factoryCertBlockLen];
    payload[i] = static_cast<uint8_t>(cert ^ (0x31U * *refs.serviceCBurstStep) ^ i);
  }

  payload[refs.serviceCNotifyLen - 2] = *refs.serviceCBurstStep;
  payload[refs.serviceCNotifyLen - 1] = 0x00;

  switch (*refs.serviceCBurstStep) {
    case 1:
      *refs.serviceCBurstAt = *refs.connStartedAt + refs.firstPairServiceCNotify2DelayMs;
      break;
    case 2:
      *refs.serviceCBurstAt = *refs.connStartedAt + refs.firstPairServiceCNotify3DelayMs;
      break;
    case 3:
      break;
    default:
      *refs.serviceCBurstStep = 0;
      return;
  }

  memcpy(refs.serviceCState, payload, refs.serviceCNotifyLen);
  refs.serviceCChar1->write(refs.serviceCState, refs.serviceCNotifyLen);
  refs.serviceCChar1->notify(refs.serviceCState, refs.serviceCNotifyLen);
  Serial.print("ServiceC first-pair notify step ");
  Serial.println(*refs.serviceCBurstStep);

  if (*refs.serviceCBurstStep >= 3) {
    *refs.serviceCBurstStep = 0;
    Serial.println("First-pair sequence complete -> staying staged");
  } else {
    (*refs.serviceCBurstStep)++;
  }
}

void ppBeginFirstPairOlderServiceDFlow(const FirstPairOlderServiceDRefs& refs,
                                       const uint8_t* status0)
{
  ppServiceDProtocolBeginOlderLane(refs, status0);
}

bool ppHandleFirstPairOlderServiceDWrite(const FirstPairOlderServiceDRefs& refs,
                                         const uint8_t* data,
                                         uint16_t len,
                                         const uint8_t* status1,
                                         const uint8_t* status2,
                                         const uint8_t* status4,
                                         uint32_t waitForWriteMs)
{
  return ppServiceDProtocolHandleOlderLaneWrite(refs,
                                                data,
                                                len,
                                                status1,
                                                status2,
                                                status4,
                                                waitForWriteMs);
}
