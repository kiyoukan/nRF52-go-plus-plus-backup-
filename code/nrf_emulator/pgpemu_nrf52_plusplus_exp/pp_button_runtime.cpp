#include <Arduino.h>

#include "pp_button_runtime.h"

namespace {

static void logButtonRuntimeBytes(const char* label, const uint8_t* data, uint16_t len)
{
  Serial.print(label);
  Serial.print(" len=");
  Serial.print(len);
  Serial.print(" data=");
  for (uint16_t i = 0; i < len; ++i) {
    if (i) {
      Serial.print(" ");
    }
    if (data[i] < 0x10) {
      Serial.print("0");
    }
    Serial.print(data[i], HEX);
  }
  Serial.println();
}

}  // namespace

void ppNotifyButtonState(const ButtonRuntimeRefs& refs)
{
  refs.buttonNotifyChar->write(refs.buttonState, 2);
  refs.buttonNotifyChar->notify(refs.buttonState, 2);
  logButtonRuntimeBytes("Button notify value", refs.buttonState, 2);
  logButtonRuntimeBytes("ServiceA char5 preserved value", refs.serviceAStatus, 2);
  Serial.print("Button notify: ");
  for (uint8_t i = 0; i < 2; ++i) {
    if (i) Serial.print(" ");
    if (refs.buttonState[i] < 0x10) Serial.print("0");
    Serial.print(refs.buttonState[i], HEX);
  }
  Serial.println();
}

void ppSetVisibleButtonState(const ButtonRuntimeRefs& refs, uint8_t b0, uint8_t b1)
{
  refs.buttonState[0] = b0;
  refs.buttonState[1] = b1;
  ppNotifyButtonState(refs);
}

void ppSendRuntimeIdleButtonBaseline(const ButtonRuntimeRefs& refs, const char* reason)
{
  if (*refs.pairingStage != PAIR_STAGE_AUTHENTICATED || !*refs.buttonNotifyEnabled) {
    return;
  }

  refs.buttonState[0] = 0x00;
  refs.buttonState[1] = 0x00;
  ppNotifyButtonState(refs);
  Serial.print("Runtime idle baseline via ");
  Serial.println(reason);
}

void ppResetRuntimeButtonEngine(const ButtonRuntimeRefs& refs)
{
  *refs.runtimeButtonEngineActive = false;
  *refs.runtimeButtonActionArmed = false;
  *refs.runtimeButtonAwaitingAck = false;
  *refs.runtimeButtonSettlingToIdle = false;
  *refs.runtimeButtonFamilyIndex = 0;
  *refs.runtimeButtonCurrentFamily = 0;
  *refs.runtimeButtonIdleCount = 0;
  *refs.runtimeButtonAt = 0;
}

void ppBeginRuntimeButtonEngine(const ButtonRuntimeRefs& refs,
                                uint32_t firstIdleDelayMs,
                                const char* reason)
{
  if (*refs.pairingStage != PAIR_STAGE_AUTHENTICATED) {
    return;
  }

  if (!*refs.runtimeButtonEngineActive) {
    *refs.runtimeButtonEngineActive = true;
    *refs.runtimeButtonAwaitingAck = false;
    *refs.runtimeButtonSettlingToIdle = false;
    *refs.runtimeButtonIdleCount = 0;
    *refs.runtimeButtonAt = millis() + firstIdleDelayMs;
    Serial.print("Runtime button engine armed via ");
    Serial.println(reason);
  }
}

void ppArmRuntimeButtonAction(const ButtonRuntimeRefs& refs,
                              uint32_t firstIdleDelayMs,
                              const char* reason)
{
  ppBeginRuntimeButtonEngine(refs, firstIdleDelayMs, reason);
  *refs.runtimeButtonActionArmed = true;
  *refs.runtimeButtonIdleCount = 0;
  if (!*refs.runtimeButtonAwaitingAck && !*refs.runtimeButtonSettlingToIdle) {
    *refs.runtimeButtonAt = millis() + firstIdleDelayMs;
  }
  Serial.print("Runtime button action armed via ");
  Serial.println(reason);
}

void ppEmitRuntimeButtonPreState(const ButtonRuntimeRefs& refs,
                                 uint8_t familyIndex,
                                 uint8_t prestateSendCount,
                                 uint32_t prestateSendGapMs,
                                 uint32_t ackTimeoutMs)
{
  *refs.runtimeButtonCurrentFamily = familyIndex;
  uint8_t b0 = 0x00;
  uint8_t b1 = 0x3C;
  switch (familyIndex) {
    case 0:
    case 1: b1 = 0x03; break;
    case 2: b1 = 0xF0; break;
    case 3: b1 = 0x1C; break;
    default: b1 = 0x3C; break;
  }
  for (uint8_t i = 0; i < prestateSendCount; ++i) {
    ppSetVisibleButtonState(refs, b0, b1);
    if (i + 1 < prestateSendCount) {
      delay(prestateSendGapMs);
    }
  }
  *refs.runtimeButtonAwaitingAck = true;
  *refs.runtimeButtonActionArmed = false;
  *refs.runtimeButtonSettlingToIdle = false;
  *refs.runtimeButtonAt = millis() + ackTimeoutMs;
  Serial.print("Runtime button pre-state family=");
  Serial.println(familyIndex);
  Serial.print("Runtime button pre-state triple send count=");
  Serial.print(prestateSendCount);
  Serial.print(" gap_ms=");
  Serial.println(prestateSendGapMs);
}

void ppStopPostAuthRuntimeScaffolding(const ButtonRuntimeRefs& refs)
{
  *refs.postAuthIdleButtonRemaining = 0;
  *refs.postAuthIdleButtonAt = 0;
  *refs.postAuthReady18Pending = false;
  *refs.postAuthReady18At = 0;
}

void ppArmPostAuthRuntimeScaffolding(const ButtonRuntimeRefs& refs,
                                     uint8_t idleButtonCount,
                                     uint32_t idleButtonFirstDelayMs,
                                     uint32_t ready18DelayMs)
{
  if (!*refs.buttonNotifyEnabled) {
    *refs.userCccdsSeeded = false;
    refs.seedUserServiceCccds("auth complete idle train");
    *refs.buttonNotifyEnabled = *refs.userCccdsSeeded || *refs.buttonNotifyEnabled;
  }

  *refs.postAuthIdleButtonRemaining = idleButtonCount;
  *refs.postAuthIdleButtonAt = millis() + idleButtonFirstDelayMs;
  *refs.postAuthReady18Pending = true;
  *refs.postAuthReady18At = millis() + ready18DelayMs;
}

void ppPulseButton(const ButtonRuntimeRefs& refs, uint32_t pulseMs)
{
  ppSetVisibleButtonState(refs, 0x00, 0x07);
  *refs.buttonPulseActive = true;
  *refs.buttonPulseReleaseAt = millis() + pulseMs;
}

void ppClearButtonPulseIfNeeded(const ButtonRuntimeRefs& refs)
{
  if (!*refs.buttonPulseActive) {
    return;
  }
  if (static_cast<int32_t>(millis() - *refs.buttonPulseReleaseAt) < 0) {
    return;
  }
  ppSetVisibleButtonState(refs, 0x00, 0x00);
  *refs.buttonPulseActive = false;
}

void ppAcknowledgeRuntimeMainButtonAction(const ButtonRuntimeRefs& refs, uint32_t idlePeriodMs)
{
  if (!*refs.runtimeButtonEngineActive || !*refs.runtimeButtonAwaitingAck) {
    return;
  }
  *refs.runtimeButtonAwaitingAck = false;
  *refs.runtimeButtonSettlingToIdle = false;
  *refs.runtimeButtonIdleCount = 0;

  if (*refs.runtimeButtonCurrentFamily <= 1) {
    ppSetVisibleButtonState(refs, 0x02, 0x00);
    *refs.runtimeButtonSettlingToIdle = true;
    *refs.runtimeButtonAt = millis() + idlePeriodMs;
  } else {
    ppSetVisibleButtonState(refs, 0x00, 0x00);
    *refs.runtimeButtonEngineActive = false;
    *refs.runtimeButtonActionArmed = false;
    *refs.runtimeButtonAt = 0;
  }
  *refs.runtimeButtonFamilyIndex = (*refs.runtimeButtonFamilyIndex + 1) % 5;
  Serial.print("Runtime button ack family=");
  Serial.println(*refs.runtimeButtonCurrentFamily);
}

void ppProcessRuntimeButtonEngineIfNeeded(const ButtonRuntimeRefs& refs,
                                          uint8_t idleCountBeforeAction,
                                          uint32_t idlePeriodMs)
{
  if (!*refs.runtimeButtonEngineActive || *refs.pairingStage != PAIR_STAGE_AUTHENTICATED) {
    return;
  }
  if (static_cast<int32_t>(millis() - *refs.runtimeButtonAt) < 0) {
    return;
  }
  if (*refs.runtimeButtonAwaitingAck) {
    ppSetVisibleButtonState(refs, 0x00, 0x00);
    *refs.runtimeButtonAwaitingAck = false;
    *refs.runtimeButtonIdleCount = 0;
    *refs.runtimeButtonEngineActive = false;
    *refs.runtimeButtonActionArmed = false;
    *refs.runtimeButtonAt = 0;
    Serial.println("Runtime button ack timeout -> 00 00");
    return;
  }
  if (*refs.runtimeButtonSettlingToIdle) {
    ppSetVisibleButtonState(refs, 0x00, 0x00);
    *refs.runtimeButtonSettlingToIdle = false;
    *refs.runtimeButtonEngineActive = false;
    *refs.runtimeButtonActionArmed = false;
    *refs.runtimeButtonAt = 0;
    Serial.println("Runtime button settle -> 00 00");
    return;
  }
  if (*refs.runtimeButtonActionArmed && *refs.runtimeButtonIdleCount >= idleCountBeforeAction) {
    return;
  }
  ppSendRuntimeIdleButtonBaseline(refs, "runtime button engine");
  (*refs.runtimeButtonIdleCount)++;
  *refs.runtimeButtonAt = millis() + idlePeriodMs;
}

void ppProcessPostAuthIdleButtonNotify(const ButtonRuntimeRefs& refs, uint32_t idleButtonPeriodMs)
{
  if (*refs.activeConnHandle == BLE_CONN_HANDLE_INVALID || *refs.postAuthIdleButtonRemaining == 0) {
    return;
  }

  if (static_cast<int32_t>(millis() - *refs.postAuthIdleButtonAt) < 0) {
    return;
  }

  if (!*refs.buttonNotifyEnabled) {
    *refs.userCccdsSeeded = false;
    refs.seedUserServiceCccds("post-auth idle");
    *refs.buttonNotifyEnabled = *refs.userCccdsSeeded || *refs.buttonNotifyEnabled;
    if (!*refs.buttonNotifyEnabled) {
      *refs.postAuthIdleButtonRemaining = 0;
      return;
    }
  }

  refs.buttonState[0] = 0x00;
  refs.buttonState[1] = 0x00;
  refs.buttonNotifyChar->write(refs.buttonState, 2);
  logButtonRuntimeBytes("Post-auth idle button notify value", refs.buttonState, 2);
  bool sent = refs.buttonNotifyChar->notify(refs.buttonState, 2);
  Serial.print("Post-auth idle button notify sent=");
  Serial.println(sent ? "true" : "false");
  if (!sent) {
    *refs.postAuthIdleButtonRemaining = 0;
    return;
  }
  (*refs.postAuthIdleButtonRemaining)--;
  *refs.postAuthIdleButtonAt = millis() + idleButtonPeriodMs;
}

void ppProcessPostAuthReady18(const ButtonRuntimeRefs& refs)
{
  if (*refs.activeConnHandle == BLE_CONN_HANDLE_INVALID || !*refs.postAuthReady18Pending) {
    return;
  }

  if (static_cast<int32_t>(millis() - *refs.postAuthReady18At) < 0) {
    return;
  }

  if (!*refs.authControlNotifyEnabled) {
    *refs.userCccdsSeeded = false;
    refs.seedUserServiceCccds("post-auth ready 18");
    *refs.authControlNotifyEnabled = *refs.userCccdsSeeded || *refs.authControlNotifyEnabled;
  }

  bool sent = false;
  if (*refs.authControlNotifyEnabled) {
    sent = refs.authControl->notify(refs.authReply18, refs.authReply18Len);
  }
  Serial.print("Post-auth ready -> 18 00 sent=");
  Serial.println(sent ? "true" : "false");
  *refs.postAuthReady18Pending = false;
}

void ppMaybeTriggerButtonFromLedPayload(const ButtonRuntimeRefs& refs,
                                        const uint8_t* data,
                                        uint16_t len,
                                        uint8_t idleCountBeforeAction,
                                        uint8_t prestateSendCount,
                                        uint32_t prestateSendGapMs,
                                        uint32_t ackTimeoutMs,
                                        uint32_t firstIdleDelayMs,
                                        uint32_t idlePeriodMs)
{
  if (len >= 7 && data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00) {
    if (*refs.pairingStage == PAIR_STAGE_AUTHENTICATED) {
      ppStopPostAuthRuntimeScaffolding(refs);
      if (data[3] == 0x1b) {
        ppBeginRuntimeButtonEngine(refs, firstIdleDelayMs, "authenticated Pokestop/effect write");
        *refs.runtimeButtonActionArmed = false;
        *refs.runtimeButtonIdleCount = idleCountBeforeAction;
        ppEmitRuntimeButtonPreState(refs, *refs.runtimeButtonFamilyIndex, prestateSendCount, prestateSendGapMs, ackTimeoutMs);
        Serial.println("Authenticated Pokestop/effect write -> immediate runtime pre-state");
      } else if (data[3] == 0x0f) {
        ppBeginRuntimeButtonEngine(refs, firstIdleDelayMs, "authenticated prompt/effect write");
        *refs.runtimeButtonActionArmed = false;
        *refs.runtimeButtonIdleCount = idleCountBeforeAction;
        ppEmitRuntimeButtonPreState(refs, *refs.runtimeButtonFamilyIndex, prestateSendCount, prestateSendGapMs, ackTimeoutMs);
        Serial.println("Authenticated prompt/effect write -> immediate runtime pre-state");
      } else if (data[3] == 0x01 &&
                 len == 7 &&
                 data[4] == 0x0a &&
                 data[5] == 0x00 &&
                 data[6] == 0x00) {
        ppAcknowledgeRuntimeMainButtonAction(refs, idlePeriodMs);
      }
      return;
    }
    if (data[3] == 0x0f || data[3] == 0x01) {
      ppPulseButton(refs, 80);
    }
  }
}
