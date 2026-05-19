#pragma once

#include <stddef.h>
#include <stdint.h>

struct AuthBootstrapRefs {
  uint8_t* liveCertKey;
  size_t liveCertKeyLen;
  const uint8_t* factorySelector3;
  void (*seedAuthReplyIndexTable)();
  void (*clearAuthBlobStage)();
  void (*buildGeneratedInitialAuthBlob)();
};

void ppSeedLongReadBlob(uint8_t* serviceDReadBlob,
                        size_t blobLen,
                        const char* versionString);
void ppSeedAuthBlob(const AuthBootstrapRefs& refs);
