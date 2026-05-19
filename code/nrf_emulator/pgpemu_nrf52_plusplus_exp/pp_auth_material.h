#pragma once

#include <stddef.h>
#include <stdint.h>

#include <bluefruit.h>

#include "pp_types.h"

struct AuthMaterialRefs {
  uint8_t* freshAuthInitialChallenge;
  uint8_t* freshAuthSessionKey;
  uint8_t* liveCertKey;
  ServiceDCertState* serviceDCertState;
  BLECharacteristic* serviceDChar3;
  uint8_t* serviceDReadBlob;
  uint16_t* serviceDReadPendingLen;
  uint8_t* authBlobValue;
  uint8_t* authBlobVisibleValue;
  AuthBlobStageState* authBlobStage;
  uint8_t* serviceDType3ReadPayload;
  uint8_t* serviceDType5ReadPayload;
  uint8_t* persistedServiceDSelector4;
  bool* persistedServiceDSelector4Valid;
  uint8_t* persistedServiceDRecord6;
  bool* persistedServiceDRecord6Valid;
  uint8_t* lastOlderServiceDType2Write;
  bool* lastOlderServiceDType2WriteValid;
  DonorAuthRuntimeState* donorAuthRuntime;
  void (*refreshServiceDLocalPayloadsFromRuntime)();
  void (*stageVisibleAuthBlob106)();
  void (*stageVisibleAuthBlob)(const uint8_t* payload, uint16_t payloadLen, uint8_t stageClass, uint8_t lastOpcode);
  void (*printHex)(const uint8_t* data, uint16_t len);
};

void ppBuildServiceDType5PayloadFromLiveWrite(const AuthMaterialRefs& refs, const uint8_t* write36);
void ppPrepareOlderServiceDReadStage(const AuthMaterialRefs& refs, uint8_t stage);
void ppPrepareSfidaReadChallenge(const AuthMaterialRefs& refs, uint8_t stage);
void ppBuildGeneratedInitialAuthBlob(const AuthMaterialRefs& refs);
void ppBuildDonorReconnectAuthBlob(const AuthMaterialRefs& refs);
bool ppGenerateReconnectResponse(const AuthMaterialRefs& refs, const uint8_t* challenge32, uint8_t* response16);
