#include <Arduino.h>
#include <string.h>

#include "pp_cccd_seed.h"

namespace {

static uint16_t crc16_ccitt(const uint8_t* data, uint16_t len)
{
  uint16_t crc = 0xFFFF;
  while (len--) {
    uint8_t x = static_cast<uint8_t>((crc >> 8) ^ *data++);
    x ^= x >> 4;
    crc = static_cast<uint16_t>((crc << 8) ^ (static_cast<uint16_t>(x) << 12) ^
                                (static_cast<uint16_t>(x) << 5) ^ x);
  }
  return crc;
}

static bool appendSysAttrCccd(uint8_t* buf, uint16_t& pos, uint16_t handle, uint16_t value)
{
  if (handle == BLE_GATT_HANDLE_INVALID) {
    return true;
  }
  if (pos + 6 > 96) {
    return false;
  }
  buf[pos++] = static_cast<uint8_t>(handle & 0xFF);
  buf[pos++] = static_cast<uint8_t>(handle >> 8);
  buf[pos++] = 0x02;
  buf[pos++] = 0x00;
  buf[pos++] = static_cast<uint8_t>(value & 0xFF);
  buf[pos++] = static_cast<uint8_t>(value >> 8);
  return true;
}

}  // namespace

void ppSeedUserServiceCccds(const CccdSeedRefs& refs, const char* reason)
{
  if (*refs.activeConnHandle == BLE_CONN_HANDLE_INVALID || *refs.userCccdsSeeded) {
    return;
  }

  uint8_t sysAttrs[96];
  uint16_t len = 0;
  const uint16_t notifyEnabled = 0x0001;
  bool ok = true;

  ok &= appendSysAttrCccd(sysAttrs, len, refs.serviceAChar5->handles().cccd_handle, notifyEnabled);
  ok &= appendSysAttrCccd(sysAttrs, len, refs.serviceBChar1->handles().cccd_handle, notifyEnabled);
  ok &= appendSysAttrCccd(sysAttrs, len, refs.serviceBChar2->handles().cccd_handle, notifyEnabled);
  ok &= appendSysAttrCccd(sysAttrs, len, refs.serviceCChar1->handles().cccd_handle, notifyEnabled);
  ok &= appendSysAttrCccd(sysAttrs, len, refs.serviceDChar2->handles().cccd_handle, notifyEnabled);
  ok &= appendSysAttrCccd(sysAttrs, len, refs.buttonNotifyChar->handles().cccd_handle, notifyEnabled);
  ok &= appendSysAttrCccd(sysAttrs, len, refs.authControl->handles().cccd_handle, notifyEnabled);
  ok &= appendSysAttrCccd(sysAttrs, len, refs.authBlob->handles().cccd_handle, notifyEnabled);

  if (!ok || len + 2 > sizeof(sysAttrs)) {
    Serial.println("Seed user CCCDs failed: buffer too small");
    return;
  }

  uint16_t crc = crc16_ccitt(sysAttrs, len);
  sysAttrs[len++] = static_cast<uint8_t>(crc & 0xFF);
  sysAttrs[len++] = static_cast<uint8_t>(crc >> 8);

  uint32_t setStatus = sd_ble_gatts_sys_attr_set(*refs.activeConnHandle,
                                                 sysAttrs,
                                                 len,
                                                 BLE_GATTS_SYS_ATTR_FLAG_USR_SRVCS);

  uint8_t serviceDValue[2] = { 0x00, 0x00 };
  ble_gatts_value_t serviceDGattsValue;
  memset(&serviceDGattsValue, 0, sizeof(serviceDGattsValue));
  serviceDGattsValue.len = sizeof(serviceDValue);
  serviceDGattsValue.offset = 0;
  serviceDGattsValue.p_value = serviceDValue;
  uint32_t getStatus = sd_ble_gatts_value_get(*refs.activeConnHandle,
                                              refs.serviceDChar2->handles().cccd_handle,
                                              &serviceDGattsValue);

  *refs.userCccdsSeeded = (setStatus == NRF_SUCCESS);
  *refs.serviceDNotifyEnabled = ((serviceDValue[0] & 0x01) != 0);
  Serial.print("Seed late full CCCDs via ");
  Serial.print(reason);
  Serial.print(" len=");
  Serial.print(len);
  Serial.print(" crc=0x");
  Serial.print(crc, HEX);
  Serial.print(" set=0x");
  Serial.print(setStatus, HEX);
  Serial.print(" serviceD_get=0x");
  Serial.print(getStatus, HEX);
  Serial.print(" serviceD=");
  Serial.print(serviceDValue[0], HEX);
  Serial.print(" ");
  Serial.println(serviceDValue[1], HEX);
}
