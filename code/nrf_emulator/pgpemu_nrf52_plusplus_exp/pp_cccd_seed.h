#pragma once

#include <bluefruit.h>

struct CccdSeedRefs {
  uint16_t* activeConnHandle;
  bool* userCccdsSeeded;
  bool* serviceDNotifyEnabled;
  BLECharacteristic* serviceAChar5;
  BLECharacteristic* serviceBChar1;
  BLECharacteristic* serviceBChar2;
  BLECharacteristic* serviceCChar1;
  BLECharacteristic* serviceDChar2;
  BLECharacteristic* buttonNotifyChar;
  BLECharacteristic* authControl;
  BLECharacteristic* authBlob;
};

void ppSeedUserServiceCccds(const CccdSeedRefs& refs, const char* reason);
