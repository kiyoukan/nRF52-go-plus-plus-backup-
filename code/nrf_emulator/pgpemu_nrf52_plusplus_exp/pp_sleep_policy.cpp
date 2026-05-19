#include <Arduino.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

#include <string.h>

#include "pp_sleep_policy.h"

using namespace Adafruit_LittleFS_Namespace;

namespace {

static constexpr uint32_t kSleepSessionGenerationIntervalSeconds = 18UL * 60UL * 60UL;
static constexpr uint32_t kSleepGenerationPolicyMagic = 0x53525050;
static constexpr uint16_t kSleepGenerationPolicyVersion = 1;
static constexpr char kSleepGenerationPolicyPath[] = "/sleep_regen.bin";

struct PersistedSleepGenerationPolicy {
  uint32_t magic;
  uint16_t version;
  uint16_t size;
  uint8_t everGenerated;
  uint8_t reserved[3];
  uint32_t lastAnchor;
};

static bool gGeneratedSessionEverQueued = false;
static uint32_t gLastGeneratedAnchor = 0;
static bool gSleepGenerationPolicyLoaded = false;
static bool gInitialGenerationArmed = false;

static void saveSleepGenerationPolicy(const char* reason)
{
  PersistedSleepGenerationPolicy record;
  memset(&record, 0, sizeof(record));
  record.magic = kSleepGenerationPolicyMagic;
  record.version = kSleepGenerationPolicyVersion;
  record.size = sizeof(record);
  record.everGenerated = gGeneratedSessionEverQueued ? 1 : 0;
  record.reserved[0] = gInitialGenerationArmed ? 1 : 0;
  record.lastAnchor = gLastGeneratedAnchor;

  InternalFS.remove(kSleepGenerationPolicyPath);
  File file(InternalFS);
  if (!file.open(kSleepGenerationPolicyPath, FILE_O_WRITE)) {
    Serial.print("Sleep generation policy save skipped via ");
    Serial.print(reason);
    Serial.println(": InternalFS unavailable");
    return;
  }

  const uint32_t written = file.write(reinterpret_cast<const uint8_t*>(&record), sizeof(record));
  file.close();
  Serial.print("Sleep generation policy saved via ");
  Serial.print(reason);
  Serial.print(" ever=");
  Serial.print(gGeneratedSessionEverQueued ? "true" : "false");
  Serial.print(" initialArmed=");
  Serial.print(gInitialGenerationArmed ? "true" : "false");
  Serial.print(" lastAnchor=0x");
  Serial.print(gLastGeneratedAnchor, HEX);
  Serial.print(" bytes=");
  Serial.println(written);
}

}  // namespace

void ppSleepPolicyLoadIfNeeded()
{
  if (gSleepGenerationPolicyLoaded) {
    return;
  }
  gSleepGenerationPolicyLoaded = true;

  File file(InternalFS);
  if (!file.open(kSleepGenerationPolicyPath, FILE_O_READ)) {
    Serial.println("Sleep generation policy load -> no persisted file");
    return;
  }

  PersistedSleepGenerationPolicy record;
  memset(&record, 0, sizeof(record));
  const uint32_t readLen = file.read(reinterpret_cast<uint8_t*>(&record), sizeof(record));
  file.close();

  if (readLen != sizeof(record) ||
      record.magic != kSleepGenerationPolicyMagic ||
      record.version != kSleepGenerationPolicyVersion ||
      record.size != sizeof(record)) {
    Serial.print("Sleep generation policy load -> invalid len=");
    Serial.print(readLen);
    Serial.print(" magic=0x");
    Serial.print(record.magic, HEX);
    Serial.print(" version=");
    Serial.print(record.version);
    Serial.print(" size=");
    Serial.println(record.size);
    return;
  }

  gGeneratedSessionEverQueued = (record.everGenerated != 0);
  gInitialGenerationArmed = (record.reserved[0] != 0);
  gLastGeneratedAnchor = record.lastAnchor;
  Serial.print("Sleep generation policy load -> ever=");
  Serial.print(gGeneratedSessionEverQueued ? "true" : "false");
  Serial.print(" initialArmed=");
  Serial.print(gInitialGenerationArmed ? "true" : "false");
  Serial.print(" lastAnchor=0x");
  Serial.println(gLastGeneratedAnchor, HEX);
}

void ppSleepPolicyResetVolatileState()
{
  gGeneratedSessionEverQueued = false;
  gLastGeneratedAnchor = 0;
  gSleepGenerationPolicyLoaded = false;
  gInitialGenerationArmed = false;
}

void ppSleepPolicyClearPersistedState()
{
  ppSleepPolicyResetVolatileState();
  InternalFS.remove(kSleepGenerationPolicyPath);
  gSleepGenerationPolicyLoaded = true;
  Serial.println("Sleep generation policy cleared");
}

bool ppSleepPolicyShouldGenerateForAnchor(uint32_t anchor)
{
  ppSleepPolicyLoadIfNeeded();
  if (!gGeneratedSessionEverQueued) {
    return gInitialGenerationArmed;
  }
  return static_cast<uint32_t>(anchor - gLastGeneratedAnchor) >= kSleepSessionGenerationIntervalSeconds;
}

void ppSleepPolicyArmInitialGeneration(const char* reason)
{
  ppSleepPolicyLoadIfNeeded();
  gInitialGenerationArmed = true;
  saveSleepGenerationPolicy(reason);
}

void ppSleepPolicyMarkGeneratedForAnchor(uint32_t anchor, const char* reason)
{
  ppSleepPolicyLoadIfNeeded();
  gGeneratedSessionEverQueued = true;
  gLastGeneratedAnchor = anchor;
  gInitialGenerationArmed = false;
  saveSleepGenerationPolicy(reason);
}

void ppSleepPolicyMarkAnchorUpdated(uint32_t anchor, const char* reason)
{
  ppSleepPolicyLoadIfNeeded();
  if (!gGeneratedSessionEverQueued) {
    return;
  }
  gLastGeneratedAnchor = anchor;
  saveSleepGenerationPolicy(reason);
}

bool ppSleepPolicyHasGeneratedSession()
{
  ppSleepPolicyLoadIfNeeded();
  return gGeneratedSessionEverQueued;
}
