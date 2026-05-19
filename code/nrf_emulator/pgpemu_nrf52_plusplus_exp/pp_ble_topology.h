#pragma once

#include <bluefruit.h>

#include "pp_ble_services.h"
#include "pp_debug_ble.h"

struct BleTopologyRefs {
  DebugBLEService* serviceA;
  DebugBLEService* batteryService;
  DebugBLEService* serviceB;
  DebugBLEService* serviceC;
  DebugBLEService* serviceD;
  DebugBLEService* serviceE;
  DebugBLEService* authService;
  BLECharacteristic* batteryLevelChar;
  BLECharacteristic* serviceAChar1;
  BLECharacteristic* serviceAChar2;
  BLECharacteristic* serviceAChar3;
  BLECharacteristic* serviceAChar4;
  BLECharacteristic* serviceAChar5;
  BLECharacteristic* serviceBChar1;
  BLECharacteristic* serviceBChar2;
  BLECharacteristic* serviceBChar3;
  BLECharacteristic* serviceCChar1;
  BLECharacteristic* serviceCChar2;
  BLECharacteristic* serviceDChar1;
  BLECharacteristic* serviceDChar2;
  BLECharacteristic* serviceDChar3;
  BLECharacteristic* ledWriteChar;
  BLECharacteristic* buttonNotifyChar;
  BLECharacteristic* serviceEChar3;
  BLECharacteristic* serviceEChar4;
  BLECharacteristic* versionChar;
  BLECharacteristic* configChar;
  BLECharacteristic* authControl;
  BLECharacteristic* authBlob;
  const uint8_t* serviceARead3;
  uint16_t serviceARead3Len;
  const uint8_t* serviceARead4;
  uint16_t serviceARead4Len;
  const uint8_t* serviceAStatus;
  uint16_t serviceAStatusLen;
  const uint8_t* batteryLevelDefault;
  uint16_t batteryLevelDefaultLen;
  const uint8_t* serviceBStatus1;
  uint16_t serviceBStatus1Len;
  const uint8_t* serviceBStatus2;
  uint16_t serviceBStatus2Len;
  const uint8_t* serviceBRead3;
  uint16_t serviceBRead3Len;
  const uint8_t* serviceCState;
  uint16_t serviceCStateLen;
  const uint8_t* serviceDStatus;
  uint16_t serviceDStatusLen;
  const uint8_t* serviceDReadBlob;
  uint16_t serviceDReadBlobInitialLen;
  uint16_t serviceDReadBlobMaxLen;
  const uint8_t* buttonState;
  uint16_t buttonStateLen;
  const uint8_t* versionStringBytes;
  uint16_t versionStringLen;
  uint8_t* configState;
  uint16_t configStateLen;
  const uint8_t* authCtrlReply25;
  uint16_t authCtrlReply25Len;
  uint8_t* authBlobVisibleValue;
  uint16_t authBlobVisibleValueLen;
  uint16_t authBlobMaxLen;
  uint16_t shortValueLen;
  uint16_t mediumValueLen;
};

void ppSetupPokemonGoPlusTopology(const BleTopologyRefs& refs, const BleSetupCallbacks& callbacks);
