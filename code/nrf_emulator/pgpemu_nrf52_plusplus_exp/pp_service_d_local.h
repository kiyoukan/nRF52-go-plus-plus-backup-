#pragma once

#include <stddef.h>
#include <stdint.h>

#include <bluefruit.h>

#include "pp_types.h"

struct ServiceDLocalRefs {
  ServiceDCertState* serviceDCertState;
  uint16_t* activeConnHandle;
  BLECharacteristic* serviceBChar1;
  BLECharacteristic* serviceDChar2;
  BLECharacteristic* serviceDChar3;
  uint8_t* serviceDStatus;
  size_t serviceDStatusLen;
  uint8_t* serviceDReadBlob;
  uint16_t* serviceDReadPendingLen;
  uint8_t* serviceDType3ReadPayload;
  size_t serviceDType3ReadPayloadLen;
  uint8_t* serviceDType5ReadPayload;
  size_t serviceDType5ReadPayloadLen;
  bool* postServiceDHandoffActive;
  bool* postServiceDServiceBReadSeen;
  uint8_t* postServiceDServiceBValue;
  const uint8_t* donorServiceDReadType3;
  size_t donorServiceDReadType3Len;
  void (*rebuildServiceDRuntimeBlock)();
  void (*printHex)(const uint8_t* data, uint16_t len);
  void (*logServiceDCertState)(const char* reason);
  void (*refreshPairingExports)(bool notify);
  bool (*handleOlderFlowWrite)(const uint8_t* data, uint16_t len);
  bool (*handleReconnectFlowWrite)(const uint8_t* data, uint16_t len);
};

void ppBuildServiceDType3PayloadFromPersistedState(const ServiceDLocalRefs& refs);
void ppRefreshServiceDLocalPayloadsFromRuntime(const ServiceDLocalRefs& refs);
void ppNotifyServiceDStatus(const ServiceDLocalRefs& refs,
                            const uint8_t* status,
                            const char* label);
void ppPrimePostServiceDHandoff(const ServiceDLocalRefs& refs);
void ppClearPostServiceDHandoff(const ServiceDLocalRefs& refs);
bool ppHandleServiceDCertWrite(const ServiceDLocalRefs& refs,
                               const uint8_t* data,
                               uint16_t len);
