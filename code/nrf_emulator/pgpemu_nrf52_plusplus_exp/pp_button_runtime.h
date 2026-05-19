#pragma once

#include <stddef.h>
#include <stdint.h>

#include <bluefruit.h>

#include "pp_types.h"

struct ButtonRuntimeRefs {
  uint16_t* activeConnHandle;
  PairingStage* pairingStage;
  bool* userCccdsSeeded;
  bool* buttonNotifyEnabled;
  bool* authControlNotifyEnabled;
  BLECharacteristic* buttonNotifyChar;
  BLECharacteristic* serviceAChar5;
  BLECharacteristic* authControl;
  BLECharacteristic* serviceBChar1;
  uint8_t* buttonState;
  uint8_t* serviceAStatus;
  bool* runtimeButtonEngineActive;
  bool* runtimeButtonActionArmed;
  bool* runtimeButtonAwaitingAck;
  bool* runtimeButtonSettlingToIdle;
  uint8_t* runtimeButtonFamilyIndex;
  uint8_t* runtimeButtonCurrentFamily;
  uint8_t* runtimeButtonIdleCount;
  uint32_t* runtimeButtonAt;
  bool* buttonPulseActive;
  uint32_t* buttonPulseReleaseAt;
  uint8_t* postAuthIdleButtonRemaining;
  uint32_t* postAuthIdleButtonAt;
  bool* postAuthReady18Pending;
  uint32_t* postAuthReady18At;
  const uint8_t* authReply18;
  size_t authReply18Len;
  void (*seedUserServiceCccds)(const char* reason);
};

void ppNotifyButtonState(const ButtonRuntimeRefs& refs);
void ppSetVisibleButtonState(const ButtonRuntimeRefs& refs, uint8_t b0, uint8_t b1);
void ppSendRuntimeIdleButtonBaseline(const ButtonRuntimeRefs& refs, const char* reason);
void ppResetRuntimeButtonEngine(const ButtonRuntimeRefs& refs);
void ppBeginRuntimeButtonEngine(const ButtonRuntimeRefs& refs,
                                uint32_t firstIdleDelayMs,
                                const char* reason);
void ppArmRuntimeButtonAction(const ButtonRuntimeRefs& refs,
                              uint32_t firstIdleDelayMs,
                              const char* reason);
void ppEmitRuntimeButtonPreState(const ButtonRuntimeRefs& refs,
                                 uint8_t familyIndex,
                                 uint8_t prestateSendCount,
                                 uint32_t prestateSendGapMs,
                                 uint32_t ackTimeoutMs);
void ppStopPostAuthRuntimeScaffolding(const ButtonRuntimeRefs& refs);
void ppArmPostAuthRuntimeScaffolding(const ButtonRuntimeRefs& refs,
                                     uint8_t idleButtonCount,
                                     uint32_t idleButtonFirstDelayMs,
                                     uint32_t ready18DelayMs);
void ppPulseButton(const ButtonRuntimeRefs& refs, uint32_t pulseMs);
void ppClearButtonPulseIfNeeded(const ButtonRuntimeRefs& refs);
void ppAcknowledgeRuntimeMainButtonAction(const ButtonRuntimeRefs& refs, uint32_t idlePeriodMs);
void ppProcessRuntimeButtonEngineIfNeeded(const ButtonRuntimeRefs& refs,
                                          uint8_t idleCountBeforeAction,
                                          uint32_t idlePeriodMs);
void ppProcessPostAuthIdleButtonNotify(const ButtonRuntimeRefs& refs, uint32_t idleButtonPeriodMs);
void ppProcessPostAuthReady18(const ButtonRuntimeRefs& refs);
void ppMaybeTriggerButtonFromLedPayload(const ButtonRuntimeRefs& refs,
                                        const uint8_t* data,
                                        uint16_t len,
                                        uint8_t idleCountBeforeAction,
                                        uint8_t prestateSendCount,
                                        uint32_t prestateSendGapMs,
                                        uint32_t ackTimeoutMs,
                                        uint32_t firstIdleDelayMs,
                                        uint32_t idlePeriodMs);
