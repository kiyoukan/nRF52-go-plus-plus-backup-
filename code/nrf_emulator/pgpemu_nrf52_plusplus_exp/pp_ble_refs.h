#pragma once

// Header-included BLE composition builders. These bind file-local
// firmware state from the .ino translation unit.


static BleSetupCallbacks makeBleSetupCallbacks()
{
  BleSetupCallbacks callbacks = {
    cccd_write_callback,
    read_authorize_callback,
    service_write_callback
  };
  return callbacks;
}

static BleIdentityConfig makeBleIdentityConfig()
{
  BleIdentityConfig config = {
    DEVICE_NAME,
    REAL_PLUSPLUS_ADDR_LE,
    static_cast<uint16_t>(sizeof(REAL_PLUSPLUS_ADDR_LE)),
    BLE_APPEARANCE_UNKNOWN
  };
  return config;
}

static BleGapPreferenceConfig makeBleGapPreferenceConfig()
{
  BleGapPreferenceConfig config = {
    REAL_PLUSPLUS_CONN_INTERVAL_MIN_UNITS,
    REAL_PLUSPLUS_CONN_INTERVAL_MAX_UNITS,
    REAL_PLUSPLUS_CONN_TIMEOUT_UNITS
  };
  return config;
}

static BleAdvertisingConfig makeBleAdvertisingConfig()
{
  BleAdvertisingConfig config = {
    plusplusManufacturerData,
    static_cast<uint16_t>(sizeof(plusplusManufacturerData)),
    PLUSPLUS_SERVICE_DATA_32,
    static_cast<uint16_t>(sizeof(PLUSPLUS_SERVICE_DATA_32)),
    REAL_PLUSPLUS_ADV_INTERVAL_UNITS
  };
  return config;
}

static BleTopologyRefs makeBleTopologyRefs()
{
  BleTopologyRefs refs = {
    &serviceA,
    &batteryService,
    &serviceB,
    &serviceC,
    &serviceD,
    &serviceE,
    &authService,
    &batteryLevelChar,
    &serviceAChar1,
    &serviceAChar2,
    &serviceAChar3,
    &serviceAChar4,
    &serviceAChar5,
    &serviceBChar1,
    &serviceBChar2,
    &serviceBChar3,
    &serviceCChar1,
    &serviceCChar2,
    &serviceDChar1,
    &serviceDChar2,
    &serviceDChar3,
    &ledWriteChar,
    &buttonNotifyChar,
    &serviceEChar3,
    &serviceEChar4,
    &versionChar,
    &configChar,
    &authControl,
    &authBlob,
    serviceARead3,
    sizeof(serviceARead3),
    serviceARead4,
    sizeof(serviceARead4),
    serviceAStatus,
    sizeof(serviceAStatus),
    &BATTERY_LEVEL_DEFAULT,
    sizeof(BATTERY_LEVEL_DEFAULT),
    serviceBStatus1,
    sizeof(serviceBStatus1),
    serviceBStatus2,
    sizeof(serviceBStatus2),
    serviceBRead3,
    sizeof(serviceBRead3),
    serviceCState,
    sizeof(serviceCState),
    serviceDStatus,
    sizeof(serviceDStatus),
    serviceDReadBlob,
    20,
    sizeof(serviceDReadBlob),
    buttonState,
    sizeof(buttonState),
    reinterpret_cast<const uint8_t*>(VERSION_STRING),
    static_cast<uint16_t>(sizeof(VERSION_STRING) - 1),
    configState,
    sizeof(configState),
    PLUSPLUS_AUTH_CTRL_REPLY_25,
    sizeof(PLUSPLUS_AUTH_CTRL_REPLY_25),
    authBlobVisibleValue,
    sizeof(authBlobVisibleValue),
    LONG_VALUE_LEN,
    SHORT_VALUE_LEN,
    MEDIUM_VALUE_LEN
  };
  return refs;
}
