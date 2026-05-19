#pragma once

#include <bluefruit.h>

struct BleSetupCallbacks {
  void (*cccdWriteCallback)(uint16_t conn_handle, BLECharacteristic* chr, uint16_t value);
  void (*readAuthorizeCallback)(uint16_t conn_handle, BLECharacteristic* chr, ble_gatts_evt_read_t* request);
  void (*serviceWriteCallback)(uint16_t conn_handle, BLECharacteristic* chr, uint8_t* data, uint16_t len);
};

struct BleIdentityConfig {
  const char* deviceName;
  const uint8_t* realAddrLe;
  uint16_t realAddrLen;
  uint16_t appearance;
};

struct BleGapPreferenceConfig {
  uint16_t connIntervalMinUnits;
  uint16_t connIntervalMaxUnits;
  uint16_t connTimeoutUnits;
};

struct BleAdvertisingConfig {
  const uint8_t* manufacturerData;
  uint16_t manufacturerDataLen;
  const uint8_t* serviceData32;
  uint16_t serviceData32Len;
  uint16_t advIntervalUnits;
};

void ppSetupReadable(BLECharacteristic& chr,
                     const uint8_t* value,
                     uint16_t len);
void ppSetupReadableWithTrace(BLECharacteristic& chr,
                              const uint8_t* value,
                              uint16_t len,
                              const BleSetupCallbacks& callbacks);
void ppSetupReadableVariableWithTrace(BLECharacteristic& chr,
                                      const uint8_t* value,
                                      uint16_t initialLen,
                                      uint16_t maxLen,
                                      const BleSetupCallbacks& callbacks);
void ppSetupWritable(BLECharacteristic& chr,
                     uint8_t props,
                     uint16_t maxLen,
                     const BleSetupCallbacks& callbacks);
void ppSetupNotifyReadable(BLECharacteristic& chr,
                           uint8_t props,
                           const uint8_t* value,
                           uint16_t len,
                           const BleSetupCallbacks& callbacks);
void ppSetupNotifyVariable(BLECharacteristic& chr,
                           uint8_t props,
                           const uint8_t* value,
                           uint16_t initialLen,
                           uint16_t maxLen,
                           const BleSetupCallbacks& callbacks);

void ppConfigureIdentity(const BleIdentityConfig& config);
void ppConfigureGapPreferences(const BleGapPreferenceConfig& config);
void ppStartAdvertising(const BleAdvertisingConfig& config);
