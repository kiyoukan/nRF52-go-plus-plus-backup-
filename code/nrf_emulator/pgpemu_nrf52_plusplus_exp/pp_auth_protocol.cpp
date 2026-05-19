#include <Arduino.h>
#include <string.h>

#include "pp_auth_protocol.h"

bool ppAuthProtocolIsAuthenticatedSleepOpcode(uint8_t opcode)
{
  switch (opcode) {
    case 0x01:
    case 0x02:
    case 0x15:
    case 0x16:
    case 0x26:
    case 0x27:
    case 0x2d:
    case 0x30:
    case 0x31:
      return true;
    default:
      return false;
  }
}

bool ppAuthProtocolIsIndexed24Write(const uint8_t* data, uint16_t len)
{
  return data != nullptr && len == 2 && data[0] == 0x24 && data[1] <= 0x1f;
}

bool ppAuthProtocolIsAuth25Write(const uint8_t* data, uint16_t len)
{
  return data != nullptr &&
         ((len == 1 && data[0] == 0x25) ||
          (len == 2 && data[0] == 0x25 && data[1] == 0x00));
}

bool ppAuthProtocolIsFirstPairControlWrite(const uint8_t* data, uint16_t len)
{
  if (data == nullptr || len < 1) {
    return false;
  }

  switch (data[0]) {
    case 0x18:
    case 0x16:
    case 0x36:
    case 0x28:
    case 0x26:
      return true;
    case 0x29:
      return len == 3;
    default:
      return false;
  }
}

bool ppAuthProtocolHandleFirstPairWrite(const FirstPairRuntimeRefs& refs,
                                        const uint8_t* data,
                                        uint16_t len)
{
  if (*refs.pairingStage == PAIR_STAGE_AUTHENTICATED || *refs.persistentReconnectReady || len < 1) {
    return false;
  }

  if (ppAuthProtocolIsIndexed24Write(data, len)) {
    if (*refs.pairingStage == PAIR_STAGE_DISCOVERY) {
      refs.stageCandidatePair("auth write 24 indexed");
    }
    if (*refs.pairingStage == PAIR_STAGE_STAGED) {
      refs.commitStagedPair("auth write 24 indexed");
    }
    uint8_t indexedReply24[5];
    if (data[1] == 0x01) {
      memcpy(indexedReply24, refs.freshReply24, refs.freshReply24Len);
      Serial.print("Fresh auth first-pair 24 reply via firmware-faithful path -> ");
      refs.printHex(indexedReply24, static_cast<uint16_t>(refs.freshReply24Len));
      Serial.println();
    } else {
      refs.buildIndexedAuthReply24(data[1], indexedReply24, "fresh auth write 24");
    }
    refs.authControl->notify(indexedReply24, refs.freshReply24Len);
    Serial.println("Fresh auth -> indexed 24 reply");
    refs.setAuthParserSchedulerState(1, 3, 3, 3, 1, 0, 0x24, true, false, false);
    *refs.freshAuthStarted = true;
    *refs.freshAuthBlobSent = false;
    *refs.freshAuthStep = 0;
    refs.setFirstPairCertState(CERT_STATE_24_SENT, "device indexed 24 reply");
    return true;
  }

  if (data[0] == 0x18) {
    if (len >= 6) {
      refs.donorAuthRuntime->auth18Selector = data[1];
      refs.donorAuthRuntime->auth18NoncePrefix[0] = data[2];
      refs.donorAuthRuntime->auth18NoncePrefix[1] = data[3];
      refs.donorAuthRuntime->auth18NoncePrefix[2] = data[4];
      refs.donorAuthRuntime->auth18Payload0 = data[2];
      refs.donorAuthRuntime->auth18Payload1 = data[3];
      refs.donorAuthRuntime->auth18Payload2 = data[4];
      refs.donorAuthRuntime->auth18Observed = true;
      refs.updateAuthReplyIndexSlot(refs.donorAuthRuntime->auth18Selector,
                                    refs.donorAuthRuntime->auth18NoncePrefix,
                                    "parser 0x18");
      refs.donorAuthRuntime->mode = 3;
      memcpy(refs.currentAttemptCentralNonce, data + 2, 4);
      *refs.currentAttemptCentralNonceValid = true;
      refs.refreshServiceDLocalPayloadsFromRuntime();
      Serial.print("Observed app 18 selector=0x");
      Serial.print(refs.donorAuthRuntime->auth18Selector, HEX);
      Serial.print(" mode=0x");
      Serial.println(data[5], HEX);
      Serial.print("Observed app central nonce bytes=");
      refs.printHex(refs.currentAttemptCentralNonce, 4);
      Serial.println();
      refs.logDonorAuthRuntime("parser 0x18 store");
      refs.setFirstPairCertState(CERT_STATE_APP_NONCE_OBSERVED, "app write 18");
    }
    refs.scheduleFreshAuthSequence(10, "auth write 18");
    return true;
  }

  if (len == 3 && data[0] == 0x29) {
    refs.donorAuthRuntime->localFamily = data[1];
    refs.donorAuthRuntime->localStatus = data[2];
    refs.donorAuthRuntime->localCommandCode = refs.donorLocal29CommandCodeForFamily(refs.donorAuthRuntime->localFamily);
    refs.donorAuthRuntime->pendingLatch = true;
    refs.setAuthParserSchedulerState(1, 4, 2, 2, 2, 1, 0x29, true, false, false);
    Serial.print("Auth write 29 local family/status -> ");
    refs.printHex(data, len);
    Serial.println();
    refs.logDonorAuthRuntime("parser 0x29 seed");
    if (refs.donorAuthRuntime->mode == 3 && refs.donorAuthRuntime->event4Seen) {
      refs.queueDonorAuthRuntimeStep(DONOR_AUTH_RUNTIME_LOCAL29,
                                     refs.freshAuthStepDelayMs,
                                     "app write 0x29 after event 4");
    }
    return true;
  }

  if (data[0] == 0x16) {
    refs.setFirstPairCertState(CERT_STATE_APP_16_OBSERVED, "app write 16");
    if (*refs.freshAuthStarted &&
        (*refs.freshAuthStep == DONOR_AUTH_RUNTIME_30000 ||
         *refs.freshAuthStep == DONOR_AUTH_RUNTIME_EVENT3_WAIT ||
         *refs.freshAuthStep == DONOR_AUTH_RUNTIME_30001)) {
      refs.donorAuthRuntime->app16Latched = true;
      Serial.println("Auth write 16 observed during donor runtime event chain -> latched");
      refs.logDonorAuthRuntime("early app 16 latched");
      return true;
    }
    if (*refs.freshAuthStarted && *refs.freshAuthStep == 11) {
      Serial.println("Auth write 16 -> scheduling delayed device 16 00 00 00 00 00");
      *refs.freshAuthStep = 35;
      *refs.freshAuthAt = millis() + refs.freshAuth18To16DelayMs;
      return true;
    }
    if (*refs.freshAuthStarted && (*refs.freshAuthStep == 12 || *refs.freshAuthStep == 19)) {
      Serial.println("Auth write 16 after inline 16 -> wait for real app 36/28");
      *refs.freshAuthStep = 30;
      *refs.freshAuthAt = millis() + refs.freshAuthWaitFor28WriteMs;
      Serial.println("Fresh auth waiting for app write 36");
      return true;
    }
    if (*refs.freshAuthStarted && *refs.freshAuthStep == 30) {
      Serial.println("Auth write 16 observed while already waiting for app write 36");
      return true;
    }
    refs.scheduleFreshAuthSequence(11, "auth write 16");
    return true;
  }

  if (len == 1 && data[0] == 0x36) {
    *refs.firstPairSawReal36 = true;
    refs.commitPersistedServiceDSelectorStateFromAuth36();
    refs.setFirstPairCertState(CERT_STATE_REAL_36_OBSERVED, "real app write 36");
    refs.authControl->notify(refs.authReply36, refs.authReply36Len);
    refs.setAuthParserSchedulerState(1, 4, 2, 2, 1, 0, 0x36, true, false, false);
    Serial.println("Auth write 36 -> 36 00; waiting for app write 28");
    *refs.freshAuthStep = 31;
    *refs.freshAuthAt = millis() + refs.freshAuthWaitFor28WriteMs;
    return true;
  }

  if (len == 1 && data[0] == 0x28) {
    *refs.firstPairSawReal28 = true;
    refs.commitPersistedServiceDRecord1AFromAuth28();
    refs.setFirstPairCertState(CERT_STATE_REAL_28_OBSERVED, "real app write 28");
    refs.authControl->notify(refs.authReply28, refs.authReply28Len);
    refs.setAuthParserSchedulerState(1, 4, 2, 2, 1, 0, 0x28, true, false, false);
    Serial.println("Auth write 28 -> 28 00; waiting for ServiceD/SFIDA CCCD");
    *refs.freshAuthStep = 32;
    *refs.freshAuthAt = millis() + refs.freshAuthWaitFor28WriteMs;
    return true;
  }

  if (data[0] == 0x26) {
    refs.clearPostServiceDHandoff();
    refs.authControl->notify(refs.authReply26, refs.authReply26Len);
    refs.setAuthParserSchedulerState(1, 4, 2, 2, 1, 0, 0x26, true, false, false);
    Serial.println("Auth write 26 -> 26 00");
    refs.authBlob->notify(refs.authBlobShort, refs.authBlobShortLen);
    Serial.println("Auth write 26 -> short auth blob 00 00 00 00 01 00 00 00");
    *refs.freshAuthStep = 0;
    ppMarkAuthComplete(refs);
    return true;
  }

  if (ppAuthProtocolIsAuth25Write(data, len)) {
    if (*refs.pairingStage < PAIR_STAGE_COMMITTED) {
      Serial.println("Auth 25 gated: pair not committed yet");
      return true;
    }
    if (*refs.freshAuthBlobSent) {
      Serial.println("Auth step 25 ack after blob -> waiting for next fresh auth write");
      return true;
    }
    refs.authControl->notify(refs.authReply25, refs.authReply25Len);
    refs.setAuthParserSchedulerState(1, 4, 2, 2, 2, 2, 0x25, true, true, false);
    refs.buildGeneratedInitialAuthBlob();
    refs.notifyStagedAuthBlob106("auth write 25");
    *refs.freshAuthBlobSent = true;
    Serial.println("Auth step 25 -> 25 00 + visible auth blob; 378-byte challenge staged on ServiceD read");
    return true;
  }

  return false;
}

void ppAuthProtocolRefreshReconnectReply24(const ReconnectRuntimeRefs& refs,
                                           const char* reason)
{
  // Reconnect selector 1 is rebuilt from the nonce accepted at first-pair auth
  // completion; this keeps reconnect 24 tied to persisted pair state.
  if (*refs.persistedAcceptedCentralNonceValid) {
    refs.updateAuthReplyIndexSlot(1, refs.persistedAcceptedCentralNonce, "persisted accepted nonce");
  }
  refs.buildIndexedAuthReply24(1, refs.reconnectAuthReply24, reason);

  Serial.print("Reconnect 24 reply refresh via ");
  Serial.print(reason);
  Serial.print(" -> ");
  refs.printHex(refs.reconnectAuthReply24, static_cast<uint16_t>(refs.reconnectAuthReply24Len));
  if (*refs.persistedAcceptedCentralNonceValid) {
    Serial.print(" from persisted accepted nonce=");
    refs.printHex(refs.persistedAcceptedCentralNonce, 4);
  } else {
    Serial.print(" using fallback");
  }
  Serial.println();
}

void ppAuthProtocolHandleReconnectWrite24(const ReconnectRuntimeRefs& refs,
                                          uint8_t selectorIndex,
                                          const char* reason)
{
  if (selectorIndex == 0x01) {
    ppAuthProtocolRefreshReconnectReply24(refs, reason);
  } else {
    refs.buildIndexedAuthReply24(selectorIndex, refs.reconnectAuthReply24, "reconnect indexed 24");
  }

  refs.authControl->notify(refs.reconnectAuthReply24, refs.reconnectAuthReply24Len);
  Serial.print("Reconnect auth write 24 indexed -> ");
  refs.printHex(refs.reconnectAuthReply24, static_cast<uint16_t>(refs.reconnectAuthReply24Len));
  Serial.println();
}

void ppAuthProtocolHandleReconnectWrite25(const ReconnectRuntimeRefs& refs,
                                          const uint8_t* authReply25,
                                          size_t authReply25Len,
                                          const char* blobReason)
{
  refs.authControl->notify(authReply25, authReply25Len);
  refs.setAuthParserSchedulerState(1, 4, 2, 2, 2, 2, 0x25, true, true, false);
  refs.buildDonorReconnectAuthBlob();
  refs.notifyStagedAuthBlob106(blobReason);
  *refs.freshAuthBlobSent = true;
  *refs.reconnectServiceDArmed = true;
  Serial.println("Reconnect auth 25 -> donor visible auth blob + compact ServiceD arm");
}
