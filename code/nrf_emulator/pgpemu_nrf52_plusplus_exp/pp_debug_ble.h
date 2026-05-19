#pragma once

#include <bluefruit.h>

#include "pp_log.h"

class DebugBLEService : public BLEService
{
public:
  DebugBLEService(BLEUuid bleuuid) : BLEService(bleuuid) {}
  DebugBLEService(uint16_t uuid16) : BLEService(uuid16) {}
  uint16_t handleValue() const { return _handle; }
};

class DebugBLECharacteristic : public BLECharacteristic
{
public:
  DebugBLECharacteristic(BLEUuid bleuuid, const char* label)
    : BLECharacteristic(bleuuid), _debugLabel(label) {}
  DebugBLECharacteristic(uint16_t uuid16, const char* label)
    : BLECharacteristic(uuid16), _debugLabel(label) {}

protected:
  void _eventHandler(ble_evt_t* evt) override
  {
    if (evt == nullptr) {
      return;
    }

    ble_gatts_char_handles_t h = handles();
    uint16_t value_handle = h.value_handle;
    uint16_t cccd_handle = h.cccd_handle;

    switch (evt->header.evt_id) {
      case BLE_GATTS_EVT_WRITE:
      {
        ble_gatts_evt_write_t const* write = &evt->evt.gatts_evt.params.write;
        if (PP_LOG_RAW_BLE_ENABLED && (write->handle == value_handle || write->handle == cccd_handle)) {
          Serial.print("[RAW] ");
          Serial.print(_debugLabel);
          Serial.print(" evt=WRITE handle=0x");
          Serial.print(write->handle, HEX);
          Serial.print(" op=0x");
          Serial.print(write->op, HEX);
          Serial.print(" len=");
          Serial.println(write->len);
        }
        break;
      }

      case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
      {
        ble_gatts_evt_rw_authorize_request_t const* req =
          &evt->evt.gatts_evt.params.authorize_request;
        uint16_t handle = 0;
        const char* kind = "unknown";
        if (req->type == BLE_GATTS_AUTHORIZE_TYPE_READ) {
          handle = req->request.read.handle;
          kind = "READ_AUTH";
        } else if (req->type == BLE_GATTS_AUTHORIZE_TYPE_WRITE) {
          handle = req->request.write.handle;
          kind = "WRITE_AUTH";
        }
        if (PP_LOG_RAW_BLE_ENABLED && (handle == value_handle || handle == cccd_handle)) {
          Serial.print("[RAW] ");
          Serial.print(_debugLabel);
          Serial.print(" evt=");
          Serial.print(kind);
          Serial.print(" handle=0x");
          Serial.println(handle, HEX);
        }
        break;
      }
    }

    BLECharacteristic::_eventHandler(evt);
  }

private:
  const char* _debugLabel;
};
