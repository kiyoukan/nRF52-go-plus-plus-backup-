#include <Arduino.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

#include "pp_persistence.h"

using namespace Adafruit_LittleFS_Namespace;

bool ppSavePersistentPairState(const PersistedPairRuntimeRefs& refs, const char* reason)
{
  if (!*refs.persistentStoreReady) {
    Serial.print("Persist pair state skipped via ");
    Serial.print(reason);
    Serial.println(": InternalFS not ready");
    return false;
  }

  PersistedPairStateRecord record;
  memset(&record, 0, sizeof(record));
  record.magic = PERSISTED_PAIR_STATE_MAGIC;
  record.version = PERSISTED_PAIR_STATE_VERSION;
  record.size = sizeof(record);
  record.paired = *refs.persistentReconnectReady ? 1 : 0;
  record.acceptedNonceValid = *refs.persistedAcceptedCentralNonceValid ? 1 : 0;
  record.serviceDSelector4Valid = *refs.persistedServiceDSelector4Valid ? 1 : 0;
  record.serviceDSelector5Valid = *refs.persistedServiceDSelector5Valid ? 1 : 0;
  record.serviceDRecord6Valid = *refs.persistedServiceDRecord6Valid ? 1 : 0;
  record.serviceDRecord1AValid = *refs.persistedServiceDRecord1AValid ? 1 : 0;
  memcpy(record.acceptedNonce, refs.persistedAcceptedCentralNonce, sizeof(record.acceptedNonce));
  memcpy(record.serviceDSelector4, refs.persistedServiceDSelector4, sizeof(record.serviceDSelector4));
  record.serviceDSelector5 = *refs.persistedServiceDSelector5;
  memcpy(record.serviceDRecord6, refs.persistedServiceDRecord6, sizeof(record.serviceDRecord6));
  record.serviceDRecord1A = *refs.persistedServiceDRecord1A;

  InternalFS.remove(PERSISTED_PAIR_STATE_PATH);
  File file(InternalFS);
  if (!file.open(PERSISTED_PAIR_STATE_PATH, FILE_O_WRITE)) {
    Serial.print("Persist pair state open failed via ");
    Serial.println(reason);
    return false;
  }

  uint32_t written = file.write(reinterpret_cast<const uint8_t*>(&record), sizeof(record));
  file.close();

  bool ok = (written == sizeof(record));
  Serial.print("Persist pair state via ");
  Serial.print(reason);
  Serial.print(" bytes=");
  Serial.print(written);
  Serial.print(" -> ");
  Serial.println(ok ? "ok" : "failed");
  return ok;
}

bool ppLoadPersistentPairState(const PersistedPairRuntimeRefs& refs)
{
  if (!*refs.persistentStoreReady) {
    return false;
  }

  File file(InternalFS);
  if (!file.open(PERSISTED_PAIR_STATE_PATH, FILE_O_READ)) {
    Serial.println("Load pair state -> no persisted file");
    return false;
  }

  PersistedPairStateRecord record;
  memset(&record, 0, sizeof(record));
  uint32_t readLen = file.read(reinterpret_cast<uint8_t*>(&record), sizeof(record));
  file.close();

  if (readLen != sizeof(record) ||
      record.magic != PERSISTED_PAIR_STATE_MAGIC ||
      record.version != PERSISTED_PAIR_STATE_VERSION ||
      record.size != sizeof(record) ||
      !record.paired) {
    Serial.print("Load pair state -> invalid or empty record len=");
    Serial.print(readLen);
    Serial.print(" magic=0x");
    Serial.print(record.magic, HEX);
    Serial.print(" version=");
    Serial.print(record.version);
    Serial.print(" size=");
    Serial.print(record.size);
    Serial.print(" paired=");
    Serial.println(record.paired);
    return false;
  }

  *refs.persistentReconnectReady = true;
  *refs.persistedAcceptedCentralNonceValid = (record.acceptedNonceValid != 0);
  memcpy(refs.persistedAcceptedCentralNonce, record.acceptedNonce, sizeof(record.acceptedNonce));
  *refs.persistedServiceDSelector4Valid = (record.serviceDSelector4Valid != 0);
  memcpy(refs.persistedServiceDSelector4, record.serviceDSelector4, sizeof(record.serviceDSelector4));
  *refs.persistedServiceDSelector5Valid = (record.serviceDSelector5Valid != 0);
  *refs.persistedServiceDSelector5 = record.serviceDSelector5;
  *refs.persistedServiceDRecord6Valid = (record.serviceDRecord6Valid != 0);
  memcpy(refs.persistedServiceDRecord6, record.serviceDRecord6, sizeof(record.serviceDRecord6));
  *refs.persistedServiceDRecord1AValid = (record.serviceDRecord1AValid != 0);
  *refs.persistedServiceDRecord1A = record.serviceDRecord1A;
  refs.rebuildServiceDRuntimeBlock();
  refs.refreshReconnectAuthReply24("boot restore");

  Serial.print("Load pair state -> restored accepted nonce valid=");
  Serial.print(*refs.persistedAcceptedCentralNonceValid ? "true" : "false");
  Serial.print(" serviceD4=");
  Serial.print(*refs.persistedServiceDSelector4Valid ? "true" : "false");
  Serial.print(" serviceD1A=");
  Serial.println(*refs.persistedServiceDRecord1AValid ? "true" : "false");
  return true;
}

void ppClearPersistentPairState(const PersistedPairRuntimeRefs& refs, const char* reason)
{
  if (!*refs.persistentStoreReady) {
    return;
  }

  InternalFS.remove(PERSISTED_PAIR_STATE_PATH);
  Serial.print("Persist pair state cleared via ");
  Serial.println(reason);
}

void ppRestorePairedStateFromPersistence(const PersistedPairRuntimeRefs& refs)
{
  *refs.pairingStage = PAIR_STAGE_AUTHENTICATED;
  *refs.pairEventClass = 0x25;
  *refs.firstPairSawReal36 = *refs.persistedServiceDSelector4Valid;
  *refs.firstPairSawReal28 = *refs.persistedServiceDRecord1AValid;
  refs.setManufacturerState(MANUFACTURER_STATE_RECONNECT);
  refs.refreshPairingExports(false);
  Serial.println("Boot restore -> paired reconnect state armed");
}
