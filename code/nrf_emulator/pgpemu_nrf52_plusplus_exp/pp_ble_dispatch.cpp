#include <Arduino.h>
#include <string.h>

#include "pp_ble_dispatch.h"
#include "pp_connection_coordinator.h"
#include "pp_log.h"

namespace {

static void genericWriteLog(const char* label,
                            const uint8_t* data,
                            uint16_t len,
                            void (*printHex)(const uint8_t* data, uint16_t len))
{
  Serial.print(label);
  Serial.print(" len=");
  Serial.print(len);
#if PP_LOG_VERBOSE_ENABLED
  Serial.print(" data=");
  printHex(data, len);
#endif
  Serial.println();
}

static void genericStoreWrite(BLECharacteristic& chr, uint8_t* data, uint16_t len, uint16_t maxLen)
{
  if (len > maxLen) {
    len = maxLen;
  }
  chr.write(data, len);
}

static const char* characteristicLabel(const BleDispatchRefs& refs, BLECharacteristic* chr);

static void logCharacteristicReadValue(const BleDispatchRefs& refs, BLECharacteristic* chr)
{
  uint8_t value[80] = { 0 };
  uint16_t len = chr->read(value, sizeof(value));

  Serial.print(characteristicLabel(refs, chr));
  Serial.print(" read value len=");
  Serial.print(len);
#if PP_LOG_VERBOSE_ENABLED
  Serial.print(" data=");
  refs.printHex(value, len);
#endif
  Serial.println();
}

static const char* characteristicLabel(const BleDispatchRefs& refs, BLECharacteristic* chr)
{
  if (chr == refs.batteryLevelChar) return "Battery level";
  if (chr == refs.serviceAChar3) return "ServiceA char3";
  if (chr == refs.serviceAChar4) return "ServiceA char4";
  if (chr == refs.serviceAChar5) return "ServiceA char5";
  if (chr == refs.serviceBChar1) return "ServiceB char1";
  if (chr == refs.serviceBChar2) return "ServiceB char2";
  if (chr == refs.serviceCChar1) return "ServiceC char1";
  if (chr == refs.serviceCChar2) return "ServiceC char2";
  if (chr == refs.serviceDChar1) return "ServiceD char1";
  if (chr == refs.serviceDChar2) return "ServiceD char2";
  if (chr == refs.buttonNotifyChar) return "Button notify";
  if (chr == refs.versionChar) return "Version";
  if (chr == refs.serviceDChar3) return "ServiceD char3";
  if (chr == refs.authControl) return "Auth control";
  if (chr == refs.authBlob) return "Auth blob";
  return "Unknown characteristic";
}

}  // namespace

void ppCccdWriteCallback(const BleDispatchRefs& refs,
                         uint16_t conn_handle,
                         BLECharacteristic* chr,
                         uint16_t value)
{
  (void) conn_handle;

  Serial.print("Pair stage before CCCD: ");
  Serial.println(refs.pairingStageLabel(*refs.pairingStage));
  Serial.print(characteristicLabel(refs, chr));
  Serial.print(" CCCD write=0x");
  Serial.println(value, HEX);

  if (chr == refs.buttonNotifyChar) {
    *refs.buttonNotifyEnabled = (value & 0x0001) != 0;
    if (*refs.buttonNotifyEnabled && *refs.pairingStage == PAIR_STAGE_DISCOVERY) {
      refs.stageCandidatePair("button cccd enable");
    }
    return;
  }

  if (chr == refs.serviceCChar1) {
    *refs.serviceCNotifyEnabled = (value & 0x0001) != 0;
    if (*refs.serviceCNotifyEnabled) {
      if (*refs.pairingStage == PAIR_STAGE_DISCOVERY) {
        refs.stageCandidatePair("serviceC cccd enable");
      }
      refs.scheduleServiceCFirstPairBurst();
    }
    return;
  }

  if (chr == refs.serviceBChar1) {
    *refs.serviceBNotifyEnabled = (value & 0x0001) != 0;
    if (*refs.postServiceDHandoffActive) {
      Serial.print("Post-ServiceD ServiceB char1 CCCD=");
      Serial.println(*refs.serviceBNotifyEnabled ? "enabled" : "disabled");
      if (*refs.serviceBNotifyEnabled && !*refs.postServiceDServiceBReadSeen) {
        refs.serviceBChar1->write(refs.postServiceDServiceBValue, 1);
        Serial.println("Post-ServiceD handoff ServiceB char1 notify suppressed; value remains readable");
      }
    }
    return;
  }

  if (chr == refs.serviceDChar2) {
    *refs.serviceDNotifyEnabled = (value & 0x0001) != 0;
    if (*refs.serviceDNotifyEnabled) {
      ppConnectionCoordinatorHandleServiceDNotifyEnabled(refs);
    }
    return;
  }

  if (chr == refs.authControl) {
    *refs.authControlNotifyEnabled = (value & 0x0001) != 0;
    return;
  }

  if (chr == refs.authBlob) {
    *refs.authBlobNotifyEnabled = (value & 0x0001) != 0;
  }
}

void ppReadAuthorizeCallback(const BleDispatchRefs& refs,
                             uint16_t conn_handle,
                             BLECharacteristic* chr,
                             ble_gatts_evt_read_t* request)
{
  (void) conn_handle;

  if (chr == refs.serviceBChar1 &&
      *refs.postServiceDHandoffActive &&
      request->handle == refs.serviceBChar1->handles().value_handle) {
    refs.serviceBChar1->write(refs.postServiceDServiceBValue, 1);
    *refs.postServiceDServiceBReadSeen = true;
    Serial.println("Post-ServiceD handoff observed ServiceB char1 read -> 04");
  }

  Serial.print(characteristicLabel(refs, chr));
  Serial.print(" read handle=0x");
  Serial.print(request->handle, HEX);
  Serial.print(" offset=");
  Serial.println(request->offset);
  logCharacteristicReadValue(refs, chr);
}

void ppServiceWriteCallback(const BleDispatchRefs& refs,
                            uint16_t conn_handle,
                            BLECharacteristic* chr,
                            uint8_t* data,
                            uint16_t len)
{
  (void) conn_handle;

  if (chr == refs.serviceAChar1) {
    genericWriteLog("ServiceA char1 write", data, len, refs.printHex);
    genericStoreWrite(*refs.serviceAChar1, data, len, refs.shortValueLen);
    return;
  }
  if (chr == refs.serviceAChar2) {
    genericWriteLog("ServiceA char2 write", data, len, refs.printHex);
    genericStoreWrite(*refs.serviceAChar2, data, len, refs.shortValueLen);
    return;
  }
  if (chr == refs.serviceCChar1) {
    genericWriteLog("ServiceC char1 write", data, len, refs.printHex);
    genericStoreWrite(*refs.serviceCChar1, data, len, refs.mediumValueLen);
    if (len > 0) {
      uint8_t opcode = data[0];
      if (opcode == 0x02) {
        *refs.serviceCTransferState = 0x02;
        *refs.pairEventClass = 0x23;
        if (*refs.pairingStage == PAIR_STAGE_DISCOVERY) {
          refs.stageCandidatePair("serviceC opcode2");
        } else {
          refs.refreshPairingExports(true);
        }
        return;
      }
      if (opcode == 0x03) {
        *refs.serviceCTransferState = 0x01;
        if (len > 1) {
          refs.serviceCState[0] = data[1];
        }
        refs.refreshPairingExports(true);
        return;
      }
      if (opcode == 0x04) {
        if (*refs.pairingStage == PAIR_STAGE_DISCOVERY) {
          refs.stageCandidatePair("serviceC opcode4");
        }
        if (*refs.serviceCTransferState == 0x02 || *refs.pairingStage == PAIR_STAGE_STAGED) {
          refs.commitStagedPair("serviceC opcode4");
        }
        return;
      }
    }
    memcpy(refs.serviceCState, data, min<uint16_t>(refs.serviceCStateLen, len));
    refs.refreshPairingExports(true);
    return;
  }
  if (chr == refs.serviceCChar2) {
    genericWriteLog("ServiceC char2 write", data, len, refs.printHex);
    genericStoreWrite(*refs.serviceCChar2, data, len, refs.mediumValueLen);
    return;
  }
  if (chr == refs.serviceDChar1) {
    genericWriteLog("ServiceD char1 write", data, len, refs.printHex);
    genericStoreWrite(*refs.serviceDChar1, data, len, refs.mediumValueLen);
    if (!ppConnectionCoordinatorHandleServiceDWrite(refs, data, len)) {
      refs.serviceDStatus[0] = 0x00;
      refs.serviceDStatus[1] = 0x00;
      refs.serviceDStatus[2] = 0x00;
      refs.serviceDStatus[3] = static_cast<uint8_t>(len & 0xff);
      refs.serviceDChar2->notify(refs.serviceDStatus, refs.serviceDStatusLen);
    }
    return;
  }
  if (chr == refs.ledWriteChar) {
    genericWriteLog("LED/effect write", data, len, refs.printHex);
    if (*refs.pairingStage == PAIR_STAGE_AUTHENTICATED) {
      refs.stopPostAuthRuntimeScaffolding();
    }
    genericStoreWrite(*refs.ledWriteChar, data, len, refs.mediumValueLen);
    refs.maybeTriggerButtonFromLedPayload(data, len);
    return;
  }
  if (chr == refs.serviceEChar3) {
    genericWriteLog("ServiceE char3 write", data, len, refs.printHex);
    genericStoreWrite(*refs.serviceEChar3, data, len, refs.shortValueLen);
    return;
  }
  if (chr == refs.serviceEChar4) {
    genericWriteLog("ServiceE char4 write", data, len, refs.printHex);
    genericStoreWrite(*refs.serviceEChar4, data, len, refs.shortValueLen);
    return;
  }
  if (chr == refs.configChar) {
    genericWriteLog("Config write", data, len, refs.printHex);
    if (len > 0) {
      if (data[0] == refs.configResetPairedStateMagic) {
        refs.resetPairingFromApp("config magic");
        return;
      }
      if (*refs.pairingStage == PAIR_STAGE_AUTHENTICATED) {
        refs.stopPostAuthRuntimeScaffolding();
      }
      refs.configState[0] = data[0];
      refs.configChar->write(refs.configState, refs.configStateLen);
      if (*refs.pairingStage == PAIR_STAGE_DISCOVERY) {
        refs.stageCandidatePair("config write");
      }
    }
    return;
  }
  if (chr == refs.authControl) {
    genericWriteLog("Auth control write", data, len, refs.printHex);
    if (len == 32) {
      uint8_t response[16];
      if (refs.plusplusGenerateReconnectResponse(data, response)) {
#if PP_LOG_VERBOSE_ENABLED
        Serial.print("Real-cert reconnect challenge -> ");
        refs.printHex(response, sizeof(response));
        Serial.println();
#else
        Serial.println("Real-cert reconnect challenge response generated");
#endif
        if (*refs.authControlNotifyEnabled) {
          refs.authControl->notify(response, sizeof(response));
        }
      }
      genericStoreWrite(*refs.authControl, data, len, refs.mediumValueLen);
      return;
    }
    if (len == 0x14 || len == 0x24 || len == 0x34) {
      Serial.print("Real-cert staged auth-sized write observed len=0x");
      Serial.print(len, HEX);
      Serial.println(" (firmware handlers 0x41ff0/0x41ef0/0x41d9c/0x420ac path)");
      if (len == 0x14) {
        bool challengeMatches =
          (memcmp(data + 4, refs.freshAuthInitialChallenge, refs.freshAuthInitialChallengeLen) == 0);
        Serial.print("Initial generated challenge response match=");
        Serial.println(challengeMatches ? "true" : "false");
      }
    }
    ConnectionCoordinatorAuthResult authResult = ppConnectionCoordinatorHandleAuthWrite(refs, data, len);
    if (authResult == CONNECTION_COORDINATOR_AUTH_HANDLED_AND_STORE) {
      genericStoreWrite(*refs.authControl, data, len, refs.mediumValueLen);
      return;
    }
    if (authResult == CONNECTION_COORDINATOR_AUTH_HANDLED_NO_STORE) {
      return;
    }
    genericStoreWrite(*refs.authControl, data, len, refs.mediumValueLen);
    return;
  }
  if (chr == refs.authBlob) {
    genericWriteLog("Auth blob write", data, len, refs.printHex);
    if (len == 0x14 || len == 0x24 || len == 0x34) {
      Serial.print("Real-cert staged auth blob-sized write observed len=0x");
      Serial.print(len, HEX);
      Serial.println(" (uses deeper firmware auth path)");
    }
    if (refs.authBlobStage->ready) {
      uint16_t cappedLen = min<uint16_t>(len, refs.authBlobStage->totalLen);
      refs.authBlobStage->offset = cappedLen;
      refs.authBlobStage->streamFlags |= 0x02;
      refs.authParserOutput->schedulerState = 2;
      refs.authParserOutput->outputOffset = cappedLen;
      refs.authParserOutput->streamPending = (cappedLen < refs.authBlobStage->totalLen);
      refs.authParserOutput->outputPending = true;
      refs.authParserOutput->resultCode =
        refs.authParserOutput->streamPending ? AUTH_RESULT_CONTINUE : AUTH_RESULT_SUCCESS;
      Serial.print("Auth blob stage consume len=0x");
      Serial.print(cappedLen, HEX);
      Serial.print(" flags=0x");
      Serial.println(refs.authBlobStage->streamFlags, HEX);
    }
    genericStoreWrite(*refs.authBlob, data, len, refs.authBlobVisibleLen);
    return;
  }
}
