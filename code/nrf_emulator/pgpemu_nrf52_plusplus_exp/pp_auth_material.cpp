#include <Arduino.h>
#include <nrf_soc.h>
#include <string.h>

#include "pp_auth_material.h"
#include "pp_protocol_data.h"

namespace {

static bool plusplusAesEcbEncrypt(const uint8_t* key, const uint8_t* cleartext, uint8_t* ciphertext)
{
  nrf_ecb_hal_data_t ecb;
  memcpy(ecb.key, key, sizeof(ecb.key));
  memcpy(ecb.cleartext, cleartext, sizeof(ecb.cleartext));
  uint32_t status = sd_ecb_block_encrypt(&ecb);
  if (status != NRF_SUCCESS) {
    Serial.print("Plus+ AES ECB failed status=0x");
    Serial.println(status, HEX);
    memset(ciphertext, 0, 16);
    return false;
  }
  memcpy(ciphertext, ecb.ciphertext, sizeof(ecb.ciphertext));
  return true;
}

static void plusplusXor16(uint8_t* dst, const uint8_t* src)
{
  for (uint8_t i = 0; i < 16; ++i) {
    dst[i] ^= src[i];
  }
}

static void plusplusInitCcmB0(uint8_t* block, const uint8_t* nonce13, uint16_t msgLen)
{
  memset(block, 0, 16);
  block[0] = 0x39;
  memcpy(block + 1, nonce13, 13);
  block[14] = static_cast<uint8_t>(msgLen >> 8);
  block[15] = static_cast<uint8_t>(msgLen & 0xff);
}

static void plusplusInitCcmCtr(uint8_t* ctr, const uint8_t* nonce13, uint16_t counter)
{
  memset(ctr, 0, 16);
  ctr[0] = 0x01;
  memcpy(ctr + 1, nonce13, 13);
  ctr[14] = static_cast<uint8_t>(counter >> 8);
  ctr[15] = static_cast<uint8_t>(counter & 0xff);
}

static bool plusplusAesCcmEncrypt(const uint8_t* key,
                                  const uint8_t* nonce13,
                                  const uint8_t* input,
                                  uint16_t len,
                                  uint8_t* ciphertext,
                                  uint8_t* tag16)
{
  uint8_t mac[16];
  uint8_t block[16];
  uint8_t stream[16];
  uint8_t scratch[16];

  plusplusInitCcmB0(block, nonce13, len);
  if (!plusplusAesEcbEncrypt(key, block, mac)) {
    return false;
  }

  for (uint16_t pos = 0; pos < len; pos += 16) {
    memset(block, 0, sizeof(block));
    uint16_t blockLen = min<uint16_t>(16, len - pos);
    memcpy(block, input + pos, blockLen);
    plusplusXor16(block, mac);
    if (!plusplusAesEcbEncrypt(key, block, scratch)) {
      return false;
    }
    memcpy(mac, scratch, sizeof(mac));
  }

  plusplusInitCcmCtr(block, nonce13, 0);
  if (!plusplusAesEcbEncrypt(key, block, stream)) {
    return false;
  }
  for (uint8_t i = 0; i < 16; ++i) {
    tag16[i] = mac[i] ^ stream[i];
  }

  uint16_t counter = 1;
  for (uint16_t pos = 0; pos < len; pos += 16, ++counter) {
    plusplusInitCcmCtr(block, nonce13, counter);
    if (!plusplusAesEcbEncrypt(key, block, stream)) {
      return false;
    }
    uint16_t blockLen = min<uint16_t>(16, len - pos);
    for (uint16_t i = 0; i < blockLen; ++i) {
      ciphertext[pos + i] = input[pos + i] ^ stream[i];
    }
  }

  return true;
}

static void plusplusInitNonceCtr(const uint8_t* nonce, uint8_t* ctr)
{
  memset(ctr, 0, 16);
  memcpy(ctr + 1, nonce, 13);
  ctr[0] = 1;
}

static void plusplusIncrementCtr(uint8_t* ctr)
{
  ctr[15]++;
  if (ctr[15] == 0) {
    ctr[14]++;
  }
}

static void plusplusAesCtr(const uint8_t* key,
                           const uint8_t* nonce,
                           const uint8_t* input,
                           uint16_t len,
                           uint8_t* output)
{
  uint8_t ctr[16];
  uint8_t encryptedCtr[16];
  plusplusInitNonceCtr(nonce, ctr);

  for (uint16_t pos = 0; pos < len; pos += 16) {
    plusplusIncrementCtr(ctr);
    plusplusAesEcbEncrypt(key, ctr, encryptedCtr);
    uint16_t blockLen = min<uint16_t>(16, len - pos);
    for (uint16_t i = 0; i < blockLen; ++i) {
      output[pos + i] = input[pos + i] ^ encryptedCtr[i];
    }
  }
}

static void plusplusEncryptBlock(const uint8_t* key,
                                 const uint8_t* nonceIv,
                                 const uint8_t* nonce,
                                 uint8_t* output)
{
  uint8_t ctr[16];
  uint8_t encryptedCtr[16];
  plusplusInitNonceCtr(nonce, ctr);
  plusplusAesEcbEncrypt(key, ctr, encryptedCtr);
  for (uint8_t i = 0; i < 16; ++i) {
    output[i] = encryptedCtr[i] ^ nonceIv[i];
  }
}

static void writeServiceDReadBlob(const AuthMaterialRefs& refs, uint16_t len)
{
  *refs.serviceDReadPendingLen = len;
  refs.serviceDChar3->write(refs.serviceDReadBlob, len);
  refs.serviceDCertState->readPrepared = true;
}

// Older-lane read builders stay grouped here because they depend on the auth
// material and persisted ServiceD records used to generate cert payloads.
static uint16_t buildGeneratedOlderServiceDOpener(const AuthMaterialRefs& refs)
{
  uint8_t stage1Header[16] = { 0x00 };
  uint8_t stage1Ciphertext[16] = { 0x00 };
  uint8_t stage1Tag[16] = { 0x00 };
  uint8_t stage2Header[16] = { 0x00 };
  uint8_t stage2Ciphertext[80] = { 0x00 };
  uint8_t stage2Tag[16] = { 0x00 };
  uint8_t stage2Plaintext[80] = { 0x00 };
  uint8_t zeroBlock[16] = { 0x00 };
  uint8_t record2Padded[256] = { 0x00 };
  uint8_t record4Zero[16] = { 0x00 };

  memcpy(record2Padded, PLUSPLUS_FACTORY_CERT_BLOCK, sizeof(PLUSPLUS_FACTORY_CERT_BLOCK));

  plusplusEncryptBlock(record4Zero, zeroBlock, zeroBlock, stage1Header);
  if (!plusplusAesCcmEncrypt(record4Zero, stage1Header, zeroBlock, sizeof(zeroBlock), stage1Ciphertext, stage1Tag)) {
    return 0;
  }

  memcpy(stage2Plaintext + 0, REAL_PLUSPLUS_ADDR_LE, sizeof(REAL_PLUSPLUS_ADDR_LE));
  memcpy(stage2Plaintext + 6, record4Zero, sizeof(record4Zero));
  memcpy(stage2Plaintext + 22, stage1Header, sizeof(stage1Header));
  memcpy(stage2Plaintext + 38, stage1Ciphertext, sizeof(stage1Ciphertext));
  memcpy(stage2Plaintext + 54, stage1Tag, sizeof(stage1Tag));
  memcpy(stage2Plaintext + 70,
         *refs.persistedServiceDRecord6Valid ? refs.persistedServiceDRecord6 : PLUSPLUS_FACTORY_SELECTOR6,
         sizeof(PLUSPLUS_FACTORY_SELECTOR6));

  plusplusEncryptBlock(refs.liveCertKey, zeroBlock, zeroBlock, stage2Header);
  if (!plusplusAesCcmEncrypt(refs.liveCertKey, stage2Header, stage2Plaintext, sizeof(stage2Plaintext), stage2Ciphertext, stage2Tag)) {
    return 0;
  }

  memset(refs.serviceDReadBlob, 0, 378);
  memcpy(refs.serviceDReadBlob + 4, stage2Header, sizeof(stage2Header));
  memcpy(refs.serviceDReadBlob + 20, stage2Ciphertext, sizeof(stage2Ciphertext));
  memcpy(refs.serviceDReadBlob + 100, stage2Tag, sizeof(stage2Tag));
  memcpy(refs.serviceDReadBlob + 116, REAL_PLUSPLUS_ADDR_LE, sizeof(REAL_PLUSPLUS_ADDR_LE));
  memcpy(refs.serviceDReadBlob + 122, record2Padded, sizeof(record2Padded));
  return 378;
}

static uint16_t buildGeneratedOlderServiceDType1Payload(const AuthMaterialRefs& refs)
{
  uint8_t nonce[16] = { 0x00 };
  uint8_t ciphertext[16] = { 0x00 };
  uint8_t tag[16] = { 0x00 };
  uint8_t record4Key[16] = { 0x00 };
  memcpy(record4Key,
         *refs.persistedServiceDSelector4Valid ? refs.persistedServiceDSelector4 : FIRMWARE_RECORD4_DEFAULT,
         sizeof(record4Key));
  plusplusEncryptBlock(record4Key, FIRMWARE_RECORD4_DEFAULT, FIRMWARE_RECORD4_DEFAULT, nonce);
  if (!plusplusAesCcmEncrypt(record4Key, nonce, PLUSPLUS_OLDER_TYPE1_CONST, sizeof(PLUSPLUS_OLDER_TYPE1_CONST), ciphertext, tag)) {
    return 0;
  }
  memset(refs.serviceDReadBlob, 0, 378);
  refs.serviceDReadBlob[0] = 0x01;
  memcpy(refs.serviceDReadBlob + 4, nonce, sizeof(nonce));
  memcpy(refs.serviceDReadBlob + 20, ciphertext, sizeof(ciphertext));
  memcpy(refs.serviceDReadBlob + 36, tag, sizeof(tag));
  return 52;
}

static uint16_t buildGeneratedOlderServiceDType2Payload(const AuthMaterialRefs& refs)
{
  uint8_t plain[16] = { 0x00 };
  const uint8_t* record4Key = *refs.persistedServiceDSelector4Valid ? refs.persistedServiceDSelector4 : FIRMWARE_RECORD4_DEFAULT;
  if (!*refs.lastOlderServiceDType2WriteValid) {
    return 0;
  }
  plusplusAesCtr(record4Key,
                 refs.lastOlderServiceDType2Write + 4,
                 refs.lastOlderServiceDType2Write + 20,
                 sizeof(plain),
                 plain);
  memset(refs.serviceDReadBlob, 0, 378);
  refs.serviceDReadBlob[0] = 0x02;
  memcpy(refs.serviceDReadBlob + 4, plain, sizeof(plain));
  return 20;
}

}  // namespace

void ppBuildServiceDType5PayloadFromLiveWrite(const AuthMaterialRefs& refs, const uint8_t* write36)
{
  memset(refs.serviceDType5ReadPayload, 0, 20);
  refs.serviceDType5ReadPayload[0] = 0x05;
  if (write36 == nullptr) {
    return;
  }

  if (!*refs.persistedServiceDSelector4Valid) {
    Serial.println("SFIDA type5 fallback skipped: selector4 not available");
    return;
  }

  uint8_t encryptedBlockA[16];
  plusplusAesEcbEncrypt(refs.persistedServiceDSelector4, write36 + 4, encryptedBlockA);
  for (uint8_t i = 0; i < 16; ++i) {
    refs.serviceDType5ReadPayload[4 + i] = write36[20 + i] ^ encryptedBlockA[i];
  }
  Serial.print("SFIDA AES-128/CTR-like type5 fallback payload -> ");
  refs.printHex(refs.serviceDType5ReadPayload, 20);
  Serial.print(" key(record4)=");
  refs.printHex(refs.persistedServiceDSelector4, 16);
  Serial.println();
}

void ppPrepareOlderServiceDReadStage(const AuthMaterialRefs& refs, uint8_t stage)
{
  memset(refs.serviceDReadBlob, 0, 378);
  refs.serviceDCertState->readPrepared = false;

  if (stage == 0) {
    uint16_t openerLen = buildGeneratedOlderServiceDOpener(refs);
    if (openerLen == 0) {
      Serial.println("ServiceD older-lane opener generation failed");
      return;
    }
    Serial.print("ServiceD read stage prepared: generated full older-lane opener len=0x");
    writeServiceDReadBlob(refs, openerLen);
    Serial.print(openerLen, HEX);
    Serial.print(" family=0x");
    Serial.println(refs.donorAuthRuntime->localFamily, HEX);
    return;
  }

  if (stage == 1) {
    uint16_t payloadLen = buildGeneratedOlderServiceDType1Payload(refs);
    if (payloadLen == 0) {
      Serial.println("ServiceD older-lane type1 generation failed");
      return;
    }
    Serial.println("ServiceD read stage prepared: generated older-lane type1 payload");
    writeServiceDReadBlob(refs, payloadLen);
    return;
  }

  if (stage == 2) {
    uint16_t payloadLen = buildGeneratedOlderServiceDType2Payload(refs);
    if (payloadLen == 0) {
      Serial.println("ServiceD older-lane type2 generation failed");
      return;
    }
    Serial.println("ServiceD read stage prepared: generated older-lane type2 payload");
    writeServiceDReadBlob(refs, payloadLen);
  }
}

void ppPrepareSfidaReadChallenge(const AuthMaterialRefs& refs, uint8_t stage)
{
  memset(refs.serviceDReadBlob, 0, 378);
  refs.serviceDCertState->readPrepared = false;

  if (stage == 0) {
    memcpy(refs.serviceDReadBlob, refs.authBlobValue, 378);
    writeServiceDReadBlob(refs, 378);
    Serial.println("SFIDA read stage 0 prepared: 378-byte generated challenge");
    return;
  }

  if (stage == 1) {
    memcpy(refs.serviceDReadBlob, refs.serviceDType3ReadPayload, 36);
    writeServiceDReadBlob(refs, 36);
    Serial.println("SFIDA read stage 1 prepared: compact type3 payload");
    return;
  }

  if (stage == 2) {
    memcpy(refs.serviceDReadBlob, refs.serviceDType5ReadPayload, 20);
    writeServiceDReadBlob(refs, 20);
    Serial.println("SFIDA read stage 2 prepared: compact type5 payload");
  }
}

void ppBuildGeneratedInitialAuthBlob(const AuthMaterialRefs& refs)
{
  // First-pair auth blob material is generated from firmware/runtime state.
  uint8_t stage1Ciphertext[16];
  uint8_t stage1Tag[16];
  uint8_t stage2Plaintext[80];

  memcpy(refs.freshAuthInitialChallenge, PLUSPLUS_FACTORY_SELECTOR4, 16);
  memcpy(refs.freshAuthSessionKey, refs.liveCertKey, 16);

  memset(refs.authBlobValue, 0, 378);

  if (!plusplusAesCcmEncrypt(PLUSPLUS_FACTORY_SELECTOR4,
                             PLUSPLUS_FACTORY_SELECTOR4,
                             PLUSPLUS_FACTORY_SELECTOR4,
                             sizeof(PLUSPLUS_FACTORY_SELECTOR4),
                             stage1Ciphertext,
                             stage1Tag)) {
    return;
  }

  memcpy(stage2Plaintext + 0, REAL_PLUSPLUS_ADDR_LE, sizeof(REAL_PLUSPLUS_ADDR_LE));
  memcpy(stage2Plaintext + 6, PLUSPLUS_FACTORY_SELECTOR4, sizeof(PLUSPLUS_FACTORY_SELECTOR4));
  memcpy(stage2Plaintext + 22, PLUSPLUS_FACTORY_SELECTOR4, sizeof(PLUSPLUS_FACTORY_SELECTOR4));
  memcpy(stage2Plaintext + 38, stage1Ciphertext, sizeof(stage1Ciphertext));
  memcpy(stage2Plaintext + 54, stage1Tag, sizeof(stage1Tag));
  refs.refreshServiceDLocalPayloadsFromRuntime();
  memcpy(stage2Plaintext + 70,
         *refs.persistedServiceDRecord6Valid ? refs.persistedServiceDRecord6 : PLUSPLUS_FACTORY_SELECTOR6,
         sizeof(PLUSPLUS_FACTORY_SELECTOR6));

  memcpy(refs.authBlobValue + 4, PLUSPLUS_FACTORY_SELECTOR4, sizeof(PLUSPLUS_FACTORY_SELECTOR4));
  if (!plusplusAesCcmEncrypt(refs.liveCertKey,
                             PLUSPLUS_FACTORY_SELECTOR4,
                             stage2Plaintext,
                             sizeof(stage2Plaintext),
                             refs.authBlobValue + 20,
                             refs.authBlobValue + 100)) {
    return;
  }

  memcpy(refs.authBlobValue + 116, REAL_PLUSPLUS_ADDR_LE, sizeof(REAL_PLUSPLUS_ADDR_LE));
  memcpy(refs.authBlobValue + 122, PLUSPLUS_FACTORY_CERT_BLOCK, sizeof(PLUSPLUS_FACTORY_CERT_BLOCK));
  memcpy(refs.authBlobVisibleValue, refs.authBlobValue + 116, AUTH_BLOB_VISIBLE_LEN);
  refs.refreshServiceDLocalPayloadsFromRuntime();
  refs.stageVisibleAuthBlob106();
  refs.authBlobStage->lastOpcode = 0x25;
  ppPrepareSfidaReadChallenge(refs, 0);

  Serial.print("Generated 378-byte auth container selector4=");
  refs.printHex(PLUSPLUS_FACTORY_SELECTOR4, sizeof(PLUSPLUS_FACTORY_SELECTOR4));
  Serial.print(" selector3=");
  refs.printHex(refs.freshAuthSessionKey, 16);
  Serial.print(" stage1ct=");
  refs.printHex(stage1Ciphertext, 16);
  Serial.println();
}

void ppBuildDonorReconnectAuthBlob(const AuthMaterialRefs& refs)
{
  // Reconnect auth blob material intentionally stays donor-backed.
  memcpy(refs.authBlobVisibleValue, REAL_PLUSPLUS_ADDR_LE, sizeof(REAL_PLUSPLUS_ADDR_LE));
  memcpy(refs.authBlobVisibleValue + sizeof(REAL_PLUSPLUS_ADDR_LE),
         PLUSPLUS_FACTORY_CERT_BLOCK,
         sizeof(PLUSPLUS_FACTORY_CERT_BLOCK));
  refs.stageVisibleAuthBlob(refs.authBlobVisibleValue, AUTH_BLOB_VISIBLE_LEN, 6, 0x25);
  Serial.println("Prepared reconnect visible auth blob from local donor cert identity");
}

bool ppGenerateReconnectResponse(const AuthMaterialRefs& refs, const uint8_t* challenge32, uint8_t* response16)
{
  uint8_t encrypted[16];
  if (!plusplusAesEcbEncrypt(refs.liveCertKey, challenge32, encrypted)) {
    return false;
  }
  for (uint8_t i = 0; i < 16; ++i) {
    response16[i] = encrypted[i] ^ challenge32[16 + i];
  }
  return true;
}
