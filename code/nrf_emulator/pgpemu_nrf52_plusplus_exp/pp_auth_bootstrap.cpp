#include <string.h>

#include "pp_auth_bootstrap.h"

void ppSeedLongReadBlob(uint8_t* serviceDReadBlob,
                        size_t blobLen,
                        const char* versionString)
{
  for (size_t i = 0; i < blobLen; ++i) {
    serviceDReadBlob[i] = static_cast<uint8_t>((i * 17U) ^ 0x5aU);
  }
  memcpy(serviceDReadBlob, versionString, strlen(versionString));
}

void ppSeedAuthBlob(const AuthBootstrapRefs& refs)
{
  memcpy(refs.liveCertKey, refs.factorySelector3, refs.liveCertKeyLen);
  refs.seedAuthReplyIndexTable();
  refs.clearAuthBlobStage();
  refs.buildGeneratedInitialAuthBlob();
}
