#include <Arduino.h>
#include <string.h>

#include "pp_sleep_export.h"
#include "pp_sleep_policy.h"

namespace {

static constexpr uint8_t kSleepReply02Ok[2] = { 0x02, 0x00 };
static constexpr uint8_t kSleepReply15Ok[2] = { 0x15, 0x00 };
static constexpr uint8_t kSleepReply27Ok[2] = { 0x27, 0x00 };
static constexpr uint8_t kSleepReply30Ok[2] = { 0x30, 0x00 };
static constexpr uint8_t kSleepReply31Ok[2] = { 0x31, 0x00 };

static constexpr size_t kSelectorState3Len = 0x3ec;
static constexpr size_t kWainaDataCommandLen = 0x04;
static constexpr size_t kWainaSleepLogRecordHeaderLen = 0x0a;
static constexpr size_t kWainaSleepLogRecordDataLen = kSelectorState3Len;
static constexpr size_t kSelectorDataMessageLen = kWainaDataCommandLen + kWainaSleepLogRecordDataLen;
static constexpr size_t kSleepStreamBufferLen = 0x400;
static_assert(kSelectorDataMessageLen <= kSleepStreamBufferLen, "selector data message must fit staged stream buffer");
static constexpr size_t kCompactExportLen = 0x130;
static constexpr size_t kPersistedRecord0x0eLen = 0x100;
static constexpr size_t kSelectorSlotLen = 0x400;
static constexpr size_t kSelectorSlotMetaLen = 0x0b;
static constexpr size_t kSelectorSeedLen = 0x0e;
static constexpr size_t kSelectorPreludeLen = 0x08;
static constexpr size_t kSelectorSummaryLen = 0x1c;
static constexpr uint8_t kSelectorHiddenPrelude[kSelectorPreludeLen] = {
  0x4d, 0x45, 0x54, 0x41, 0x00, 0x00, 0x00, 0x00
};
static constexpr uint8_t kQueueSelectorUnread = 0x01;
static constexpr uint32_t kSelectorReadbackLen = 0x3ec;
static constexpr uint16_t kSelectorAdvertisedLen = 0x03f4;
static constexpr bool kUseAdvertisedLenForControlReply01 = false;
static constexpr uint32_t kSelectorRecordMask = 1u << kQueueSelectorUnread;
static constexpr uint32_t kSleepStreamChunkGapMs = 8;
static constexpr uint32_t kSelectorPatchSentinel = 0x00000008;
static constexpr uint16_t kSelectorLengthHalfword = 0x01ef;
static constexpr uint16_t kDonorSleepLogEndMinute = 496;
static constexpr uint8_t kDonorRecord21[0x20] = {
  0xFB, 0x2B, 0x00, 0x00, 0x25, 0x2A, 0x00, 0x00,
  0x22, 0x13, 0x00, 0x00, 0xC8, 0x11, 0x00, 0x00,
  0xEE, 0x05, 0x00, 0x00, 0x7E, 0x05, 0x00, 0x00,
  0xFB, 0x2B, 0x00, 0x00, 0x25, 0x2A, 0x00, 0x00
};
static constexpr uint8_t kDonorRecord22[0x20] = {
  0x22, 0x13, 0x00, 0x00, 0xC8, 0x11, 0x00, 0x00,
  0xEE, 0x05, 0x00, 0x00, 0x7E, 0x05, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x32, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static constexpr uint8_t kDonorRecord0x0e[kPersistedRecord0x0eLen] = { 0 };
static constexpr uint8_t kDonorRecord0x1f[9] = {
  0x00, 0x00, 0x00, 0x00, 0x32, 0x01, 0x00, 0x00, 0x00
};
static constexpr uint8_t kDonorRecord0x2a[9] = {
  0x01, 0x00, 0x00, 0x00, 0x32, 0x01, 0x00, 0x00, 0x00
};
static constexpr uint8_t kDonorRecord0x2f[9] = {
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static constexpr uint8_t kDonorSleepLogTransitions[0x20] = {
  0x00, 0x40, 0x17, 0x10, 0x18, 0x00, 0x40, 0x10,
  0x4F, 0x00, 0x94, 0x10, 0x95, 0x40, 0x9B, 0x10,
  0xDC, 0x00, 0x39, 0x11, 0x47, 0x01, 0x59, 0x11,
  0x6C, 0x41, 0x75, 0x11, 0xA1, 0x41, 0xF0, 0x41
};
struct MockSleepExportState {
  uint32_t runtimeAnchor;
  uint32_t unreadCount;
  uint32_t incompleteFlag;
  uint32_t expandedEntryCount;
  uint32_t groupCount;
  uint32_t segmentCount;
  uint32_t record0x0d;
  uint32_t record0x1e;
  uint32_t record0x25;
  uint32_t record0x26;
  uint32_t record0x27;
  uint32_t record0x28Bitmap;
  uint32_t record0x29;
  uint32_t record0x32;
  uint32_t selectorRecord0x23;
  uint8_t record0x1f[9];
  uint8_t record0x2a[9];
  uint8_t record0x2f[9];
  uint8_t selectorRecord0x21[sizeof(kDonorRecord21)];
  uint8_t selectorRecord0x22[sizeof(kDonorRecord22)];
  uint8_t record0x0e[kPersistedRecord0x0eLen];
};

static const MockSleepExportState kDefaultMockSleepState = {
  0x6a8b1c20,
  1,
  0,
  141,
  3,
  6,
  0x00005c1c,
  1,
  1,
  1,
  1,
  0x00000007,
  0,
  0xffffffff,
  0,
  { 0x00, 0x00, 0x00, 0x00, 0x32, 0x01, 0x00, 0x00, 0x00 },
  { 0x01, 0x00, 0x00, 0x00, 0x32, 0x01, 0x00, 0x00, 0x00 },
  { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
  {
    0xFB, 0x2B, 0x00, 0x00, 0x25, 0x2A, 0x00, 0x00,
    0x22, 0x13, 0x00, 0x00, 0xC8, 0x11, 0x00, 0x00,
    0xEE, 0x05, 0x00, 0x00, 0x7E, 0x05, 0x00, 0x00,
    0xFB, 0x2B, 0x00, 0x00, 0x25, 0x2A, 0x00, 0x00
  },
  {
    0x22, 0x13, 0x00, 0x00, 0xC8, 0x11, 0x00, 0x00,
    0xEE, 0x05, 0x00, 0x00, 0x7E, 0x05, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x32, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  },
  { 0 }
};
static MockSleepExportState gMockSleepState = kDefaultMockSleepState;

struct SelectorSlotState {
  uint32_t marker;
  uint16_t length;
  uint8_t generation;
  uint32_t selectorMask;
  uint8_t storage[kSelectorSlotLen];
};

struct SleepStreamStage {
  bool active;
  uint8_t opcode;
  uint8_t outputClass;
  uint16_t totalLen;
  uint16_t offset;
  uint16_t chunkLen;
  uint32_t nextAt;
  uint8_t payload[kSleepStreamBufferLen];
};

static SelectorSlotState gSelectorSlots[8] = {};
static uint8_t gSelectorActiveSlot = 0;
static uint8_t gSelectorCurrentSlot = 0;
static uint8_t gSelectorGeneration = 0x20;
static bool gSelectorSlotsInitialized = false;
static bool gUnreadQueuePrepared = false;
static SleepStreamStage gSleepStreamStage = { false, 0, 0, 0, 0, 0, 0, { 0 } };

static void writeLe32(uint8_t* out, uint32_t value)
{
  out[0] = static_cast<uint8_t>(value & 0xff);
  out[1] = static_cast<uint8_t>((value >> 8) & 0xff);
  out[2] = static_cast<uint8_t>((value >> 16) & 0xff);
  out[3] = static_cast<uint8_t>((value >> 24) & 0xff);
}

static uint32_t readLe32(const uint8_t* data)
{
  return static_cast<uint32_t>(data[0]) |
         (static_cast<uint32_t>(data[1]) << 8) |
         (static_cast<uint32_t>(data[2]) << 16) |
         (static_cast<uint32_t>(data[3]) << 24);
}

static void notifyAuthControl(BLECharacteristic* chr, const uint8_t* data, size_t len)
{
  chr->notify(data, len);
}

static void notifyFramedControlReply(BLECharacteristic* chr,
                                     uint8_t opcode,
                                     const uint8_t* payload,
                                     size_t payloadLen)
{
  uint8_t framed[2 + 4] = { 0 };
  framed[0] = opcode;
  framed[1] = 0x00;
  if (payload != nullptr && payloadLen > 0) {
    memcpy(framed + 2, payload, min(payloadLen, sizeof(framed) - 2));
  }
  chr->notify(framed, min(sizeof(framed), payloadLen + 2));
}

static void clearSleepStreamStage()
{
  gSleepStreamStage.active = false;
  gSleepStreamStage.opcode = 0;
  gSleepStreamStage.outputClass = 0;
  gSleepStreamStage.totalLen = 0;
  gSleepStreamStage.offset = 0;
  gSleepStreamStage.chunkLen = 0;
  gSleepStreamStage.nextAt = 0;
}

static bool emitNextSleepStreamChunk(const SleepExportRuntimeRefs& refs)
{
  if (!gSleepStreamStage.active || refs.authBlob == nullptr || gSleepStreamStage.offset >= gSleepStreamStage.totalLen) {
    clearSleepStreamStage();
    return false;
  }

  const uint16_t remaining = static_cast<uint16_t>(gSleepStreamStage.totalLen - gSleepStreamStage.offset);
  const uint16_t chunkLen = min<uint16_t>(refs.authBlobChunkMaxLen, remaining);
  refs.authBlob->notify(gSleepStreamStage.payload + gSleepStreamStage.offset, chunkLen);
  gSleepStreamStage.offset = static_cast<uint16_t>(gSleepStreamStage.offset + chunkLen);
  gSleepStreamStage.chunkLen = chunkLen;
  gSleepStreamStage.nextAt = millis() + kSleepStreamChunkGapMs;
  refs.setAuthParserSchedulerState(2,
                                   gSleepStreamStage.outputClass,
                                   gSleepStreamStage.totalLen,
                                   gSleepStreamStage.offset,
                                   (gSleepStreamStage.offset < gSleepStreamStage.totalLen) ? AUTH_RESULT_CONTINUE
                                                                                           : AUTH_RESULT_SUCCESS,
                                   0,
                                   gSleepStreamStage.opcode,
                                   true,
                                   gSleepStreamStage.offset < gSleepStreamStage.totalLen,
                                   gSleepStreamStage.offset < gSleepStreamStage.totalLen);
  if (gSleepStreamStage.offset >= gSleepStreamStage.totalLen) {
    Serial.print("Auth sleep stream complete opcode=0x");
    Serial.print(gSleepStreamStage.opcode, HEX);
    Serial.print(" total=0x");
    Serial.println(gSleepStreamStage.totalLen, HEX);
    clearSleepStreamStage();
  } else {
    Serial.print("Auth sleep stream chunk opcode=0x");
    Serial.print(gSleepStreamStage.opcode, HEX);
    Serial.print(" sent=0x");
    Serial.print(gSleepStreamStage.offset, HEX);
    Serial.print("/");
    Serial.println(gSleepStreamStage.totalLen, HEX);
  }
  return true;
}

static void startSleepStream(const SleepExportRuntimeRefs& refs,
                             uint8_t opcode,
                             uint8_t outputClass,
                             const uint8_t* payload,
                             uint16_t payloadLen)
{
  clearSleepStreamStage();
  if (payload == nullptr || payloadLen == 0) {
    return;
  }
  memcpy(gSleepStreamStage.payload, payload, payloadLen);
  gSleepStreamStage.active = true;
  gSleepStreamStage.opcode = opcode;
  gSleepStreamStage.outputClass = outputClass;
  gSleepStreamStage.totalLen = payloadLen;
  gSleepStreamStage.offset = 0;
  gSleepStreamStage.nextAt = millis();
  refs.setAuthParserSchedulerState(1,
                                   outputClass,
                                   payloadLen,
                                   0,
                                   AUTH_RESULT_CONTINUE,
                                   0,
                                   opcode,
                                   true,
                                   payloadLen > 0,
                                   false);
  emitNextSleepStreamChunk(refs);
}

static uint16_t selectorQueuedDescriptorCount(uint8_t selector)
{
  const uint32_t mask = 1u << selector;
  uint16_t count = 0U;
  uint8_t slotIndex = gSelectorCurrentSlot;
  bool started = false;
  for (uint8_t i = 0; i < 8; ++i) {
    const SelectorSlotState& slot = gSelectorSlots[slotIndex];
    const bool matches = ((slot.selectorMask & mask) != 0U) && slot.length != 0U;
    if (!started) {
      if (matches) {
        started = true;
        ++count;
      }
    } else {
      if (!matches) {
        break;
      }
      ++count;
    }
    slotIndex = static_cast<uint8_t>((slotIndex + 1U) & 0x07U);
  }
  return count;
}

static void buildCompactExport(uint8_t* out)
{
  memset(out, 0, kCompactExportLen);
  writeLe32(out, 0);
}

static void writeInitialSelectorHead(uint8_t* out)
{
  if (out == nullptr) {
    return;
  }
  memset(out, 0, kSelectorSeedLen);
  writeLe32(out + 0x00, kSelectorPatchSentinel);
  writeLe32(out + 0x04, 0xffffffffU);
  out[0x08] = static_cast<uint8_t>(kSelectorLengthHalfword & 0xff);
  out[0x09] = static_cast<uint8_t>((kSelectorLengthHalfword >> 8) & 0xff);
}

static void writeSelectorSummaryTail(uint8_t* out, uint8_t selector)
{
  if (out == nullptr) {
    return;
  }

  const size_t base = kSelectorSeedLen;
  writeLe32(out + base + 0x00, gMockSleepState.runtimeAnchor);
  writeLe32(out + base + 0x04, gMockSleepState.expandedEntryCount);
  writeLe32(out + base + 0x08, gMockSleepState.groupCount);
  writeLe32(out + base + 0x0c, selectorQueuedDescriptorCount(selector));
  writeLe32(out + base + 0x10, gMockSleepState.unreadCount);
  writeLe32(out + base + 0x14, gMockSleepState.segmentCount);
  writeLe32(out + base + 0x18, gMockSleepState.incompleteFlag);
}

static void buildInitialSelectorVisiblePayload(uint8_t* out)
{
  memset(out, 0, kSelectorState3Len);
  writeInitialSelectorHead(out);
  writeSelectorSummaryTail(out, kQueueSelectorUnread);

  const uint8_t* primaryRecord =
    (gMockSleepState.selectorRecord0x23 == 0U) ? gMockSleepState.selectorRecord0x21
                                               : gMockSleepState.selectorRecord0x22;
  const uint8_t* siblingRecord =
    (gMockSleepState.selectorRecord0x23 == 0U) ? gMockSleepState.selectorRecord0x22
                                               : gMockSleepState.selectorRecord0x21;

  memcpy(out + 10U, primaryRecord, sizeof(uint32_t));

  size_t cursor = kSelectorSeedLen + kSelectorSummaryLen;
  memcpy(out + cursor, primaryRecord, sizeof(gMockSleepState.selectorRecord0x21));
  cursor += sizeof(gMockSleepState.selectorRecord0x21);
  memcpy(out + cursor, siblingRecord, sizeof(gMockSleepState.selectorRecord0x22));
  cursor += sizeof(gMockSleepState.selectorRecord0x22);

  memcpy(out + cursor, gMockSleepState.record0x1f, sizeof(gMockSleepState.record0x1f));
  cursor += sizeof(gMockSleepState.record0x1f);
  memcpy(out + cursor, gMockSleepState.record0x2a, sizeof(gMockSleepState.record0x2a));
  cursor += sizeof(gMockSleepState.record0x2a);
  memcpy(out + cursor, gMockSleepState.record0x2f, sizeof(gMockSleepState.record0x2f));
  cursor += sizeof(gMockSleepState.record0x2f);
}

static uint8_t findSelectorSourceSlot(uint32_t mask)
{
  if ((gSelectorSlots[gSelectorActiveSlot].selectorMask & mask) != 0U &&
      gSelectorSlots[gSelectorActiveSlot].length != 0U) {
    return static_cast<uint8_t>(gSelectorActiveSlot & 0x07U);
  }
  for (uint8_t i = 0; i < 8; ++i) {
    if ((gSelectorSlots[i].selectorMask & mask) != 0U && gSelectorSlots[i].length != 0U) {
      return i;
    }
  }
  return 0;
}

static void rebuildSelectorSlotImage()
{
  if (!gSelectorSlotsInitialized) {
    memset(gSelectorSlots, 0, sizeof(gSelectorSlots));
    gSelectorSlotsInitialized = true;
  }

  SelectorSlotState& slot = gSelectorSlots[gSelectorActiveSlot];
  const bool hadVisibleState = slot.length != 0U;
  uint8_t priorVisible[kSelectorState3Len];
  if (hadVisibleState) {
    memcpy(priorVisible,
           slot.storage + kSelectorSlotMetaLen + kSelectorPreludeLen,
           sizeof(priorVisible));
  }
  slot.marker = 0;
  slot.length = (gMockSleepState.unreadCount > 0U) ? kSelectorAdvertisedLen : 0U;
  slot.generation = gSelectorGeneration++;
  slot.selectorMask = (gMockSleepState.unreadCount > 0U) ? kSelectorRecordMask : 0U;
  memset(slot.storage, 0, sizeof(slot.storage));

  uint8_t visible[kSelectorState3Len];
  if (hadVisibleState) {
    memcpy(visible, priorVisible, sizeof(visible));
  } else {
    buildInitialSelectorVisiblePayload(visible);
  }

  const uint8_t* primaryRecord =
    (gMockSleepState.selectorRecord0x23 == 0U) ? gMockSleepState.selectorRecord0x21
                                               : gMockSleepState.selectorRecord0x22;
  const uint8_t sourceSlot = findSelectorSourceSlot(kSelectorRecordMask);
  const uint8_t selectedDwordIndex = static_cast<uint8_t>(sourceSlot & 0x07U);
  memcpy(visible + 10U,
         primaryRecord + (selectedDwordIndex * sizeof(uint32_t)),
         sizeof(uint32_t));
  writeSelectorSummaryTail(visible, kQueueSelectorUnread);

  memcpy(slot.storage + kSelectorSlotMetaLen,
         kSelectorHiddenPrelude,
         sizeof(kSelectorHiddenPrelude));
  writeLe32(slot.storage + kSelectorSlotMetaLen + 0x04, gMockSleepState.runtimeAnchor);
  memcpy(slot.storage + kSelectorSlotMetaLen + kSelectorPreludeLen, visible, sizeof(visible));
}

static bool patchSelectorSlotRuntimeAnchor(uint32_t oldAnchor, uint32_t newAnchor)
{
  bool patched = false;

  for (SelectorSlotState& slot : gSelectorSlots) {
    if ((slot.selectorMask & kSelectorRecordMask) == 0U || slot.length == 0U) {
      continue;
    }

    uint8_t* hidden = slot.storage + kSelectorSlotMetaLen;
    uint8_t* visible = hidden + kSelectorPreludeLen;
    if (readLe32(hidden + 0x00) != readLe32(kSelectorHiddenPrelude + 0x00)) {
      continue;
    }
    if (readLe32(visible + 0x00) != kSelectorPatchSentinel || readLe32(visible + 0x04) != 0xffffffffU) {
      continue;
    }

    const uint32_t baseAnchor = readLe32(hidden + 0x04);
    const uint32_t patchedAnchor = baseAnchor + (newAnchor - oldAnchor);
    writeLe32(visible + 0x04, patchedAnchor);
    memset(hidden, 0, kSelectorPreludeLen);
    patched = true;
  }

  gMockSleepState.runtimeAnchor = newAnchor;
  return patched;
}

static void initializeSelectorSlots()
{
  if (gSelectorSlotsInitialized) {
    return;
  }
  gSelectorActiveSlot = 0;
  gSelectorCurrentSlot = 0;
  gSelectorGeneration = 0x20;
  memset(gSelectorSlots, 0, sizeof(gSelectorSlots));
  gSelectorSlotsInitialized = true;
  rebuildSelectorSlotImage();
}

static void resetReconnectSleepStateInternal()
{
  clearSleepStreamStage();
  gMockSleepState = kDefaultMockSleepState;
  memset(gSelectorSlots, 0, sizeof(gSelectorSlots));
  gSelectorActiveSlot = 0;
  gSelectorCurrentSlot = 0;
  gSelectorGeneration = 0x20;
  gSelectorSlotsInitialized = false;
  gUnreadQueuePrepared = false;
  ppSleepPolicyResetVolatileState();
}

static void clearSelectorSlotState()
{
  memset(gSelectorSlots, 0, sizeof(gSelectorSlots));
  gSelectorActiveSlot = 0;
  gSelectorCurrentSlot = 0;
  gSelectorGeneration = 0x20;
  gSelectorSlotsInitialized = true;
}

static void prepareReconnectSleepStateForAnchor(uint32_t anchor)
{
  clearSleepStreamStage();
  gMockSleepState = kDefaultMockSleepState;
  gMockSleepState.runtimeAnchor = anchor;
  memset(gSelectorSlots, 0, sizeof(gSelectorSlots));
  gSelectorActiveSlot = 0;
  gSelectorCurrentSlot = 0;
  gSelectorGeneration = 0x20;
  gSelectorSlotsInitialized = false;
  initializeSelectorSlots();
  gUnreadQueuePrepared = true;
  ppSleepPolicyMarkGeneratedForAnchor(anchor, "prepare sleep session");
}

static void ensureUnreadQueuePrepared()
{
  ppSleepPolicyLoadIfNeeded();
  if (gUnreadQueuePrepared) {
    initializeSelectorSlots();
    return;
  }
  clearSleepStreamStage();
  gMockSleepState = kDefaultMockSleepState;
  gMockSleepState.unreadCount = 0;
  clearSelectorSlotState();
  gUnreadQueuePrepared = true;
}

static void buildSelectorState3Payload(uint8_t* out)
{
  initializeSelectorSlots();
  memcpy(out,
         gSelectorSlots[gSelectorActiveSlot].storage + kSelectorSlotMetaLen + kSelectorPreludeLen,
         kSelectorState3Len);
}

static void buildSelectorDataMessagePayload(uint8_t* out)
{
  memset(out, 0, kSelectorDataMessageLen);
  out[0] = 'U';
  out[1] = 'L';
  out[2] = 'E';
  out[3] = 'P';
  const uint32_t sessionStart =
    gMockSleepState.runtimeAnchor - (static_cast<uint32_t>(kDonorSleepLogEndMinute) * 60U);
  writeLe32(out + 4, sessionStart);
  out[8] = 0x10;
  out[9] = 0x00;
  out[10] = 0x00;
  out[11] = 0x00;
  out[12] = 0x00;
  out[13] = 0x00;
  memcpy(out + kWainaDataCommandLen + kWainaSleepLogRecordHeaderLen,
         kDonorSleepLogTransitions,
         sizeof(kDonorSleepLogTransitions));
}

static void clearOneQueuedDescriptor(uint8_t selector)
{
  const uint32_t mask = 1u << selector;
  for (SelectorSlotState& slot : gSelectorSlots) {
    if ((slot.selectorMask & mask) == 0U || slot.length == 0U) {
      continue;
    }
    slot.selectorMask &= ~mask;
    if (slot.selectorMask == 0U) {
      slot.length = 0U;
    }
    break;
  }
  if (selector == kQueueSelectorUnread) {
    gMockSleepState.unreadCount = selectorQueuedDescriptorCount(kQueueSelectorUnread);
    writeSelectorSummaryTail(gSelectorSlots[gSelectorActiveSlot].storage + kSelectorSlotMetaLen + kSelectorPreludeLen,
                             kQueueSelectorUnread);
  }
}

static bool appendTokenValue(uint8_t token, uint8_t* out, size_t* offset, size_t maxLen)
{
  if (out == nullptr || offset == nullptr || *offset > maxLen) {
    return false;
  }

  auto appendLe32 = [&](uint32_t value) -> bool {
    if (*offset + 4 > maxLen) {
      return false;
    }
    writeLe32(out + *offset, value);
    *offset += 4;
    return true;
  };

  auto appendBytes = [&](const uint8_t* value, size_t len) -> bool {
    if (*offset + len > maxLen) {
      return false;
    }
    memcpy(out + *offset, value, len);
    *offset += len;
    return true;
  };

  switch (token) {
    case 0x00: return appendLe32(gMockSleepState.runtimeAnchor);
    case 0x02: return appendLe32(gMockSleepState.record0x0d);
    case 0x03: return appendLe32(0);
    case 0x04: return appendLe32(gMockSleepState.runtimeAnchor);
    case 0x05: return appendLe32(0);
    case 0x06: return appendLe32(gMockSleepState.runtimeAnchor + 0x600);
    case 0x08: return appendLe32(0);
    case 0x09: return appendLe32(0);
    case 0x0a: return appendLe32(0);
    case 0x0b: return appendLe32(0);
    case 0x0c: return appendLe32(gMockSleepState.record0x1e);
    case 0x0d: return appendLe32(0);
    case 0x0e: return appendLe32(0);
    case 0x0f: return appendLe32(0);
    case 0x10: return appendLe32(0);
    case 0x11: return appendLe32(gMockSleepState.unreadCount);
    case 0x12: return appendLe32(gMockSleepState.runtimeAnchor);
    case 0x13: return appendLe32(gMockSleepState.runtimeAnchor);
    case 0x14: return appendLe32(0);
    case 0x16: return appendBytes(gMockSleepState.record0x0e, sizeof(gMockSleepState.record0x0e));
    case 0x18: return appendBytes(gMockSleepState.record0x2a, sizeof(gMockSleepState.record0x2a));
    case 0x19: return appendBytes(gMockSleepState.record0x2f, sizeof(gMockSleepState.record0x2f));
    case 0x1a: return appendBytes(gMockSleepState.record0x1f, sizeof(gMockSleepState.record0x1f));
    case 0x1e: return appendLe32(0);
    case 0x1f: return appendLe32(0);
    case 0x20: return appendLe32(0);
    case 0x21: return appendLe32(0);
    case 0x22: return appendLe32(0);
    case 0x23: return appendLe32(0);
    case 0x24: return appendLe32(0);
    case 0x25: return appendLe32(0);
    case 0x27: return appendLe32(0);
    case 0x28: return appendLe32(0);
    case 0x29: return appendLe32(0);
    case 0x2a: return appendLe32(gMockSleepState.record0x25);
    case 0x2b: return appendLe32(gMockSleepState.record0x26);
    case 0x2c: return appendLe32(gMockSleepState.record0x27);
    case 0x2d:
      if (*offset + kCompactExportLen > maxLen) {
        return false;
      }
      buildCompactExport(out + *offset);
      *offset += kCompactExportLen;
      return true;
    case 0x2e: return appendLe32(gMockSleepState.record0x28Bitmap);
    case 0x2f: return appendLe32(gMockSleepState.record0x29);
    case 0x31: return appendLe32(gMockSleepState.incompleteFlag);
    default:
      return false;
  }
}

static bool buildTokenizedSleepPayload(const uint8_t* data,
                                       uint16_t len,
                                       uint8_t* out,
                                       size_t outCapacity,
                                       size_t* builtLen,
                                       uint32_t* expectedLenOut)
{
  if (data == nullptr || out == nullptr || builtLen == nullptr || expectedLenOut == nullptr || len < 6) {
    return false;
  }

  uint32_t expectedLen = readLe32(data + 1);
  uint8_t tokenCount = data[5];
  if (len != static_cast<uint16_t>(tokenCount + 6U)) {
    return false;
  }
  if (expectedLen > outCapacity) {
    return false;
  }

  size_t offset = 0;
  for (uint8_t i = 0; i < tokenCount; ++i) {
    if (!appendTokenValue(data[6 + i], out, &offset, expectedLen)) {
      return false;
    }
  }

  *builtLen = offset;
  *expectedLenOut = expectedLen;
  return true;
}

}  // namespace

SleepExportWriteResult ppHandleAuthenticatedSleepWrite(const SleepExportRuntimeRefs& refs,
                                                       const uint8_t* data,
                                                       uint16_t len)
{
  if (*refs.pairingStage != PAIR_STAGE_AUTHENTICATED || data == nullptr || len < 1) {
    return SLEEP_EXPORT_NOT_HANDLED;
  }

  ensureUnreadQueuePrepared();

  switch (data[0]) {
    case 0x01:
      // Stream the queued unread sleep record. The queue is only populated
      // after the disconnect/0x02 policy allows generation.
      if (len != 2 || data[1] != kQueueSelectorUnread) {
        Serial.println("Auth sleep write 01 rejected: expected selector 0x01");
        return SLEEP_EXPORT_HANDLED_NO_STORE;
      }
      refs.clearPostServiceDHandoff();
      {
        uint8_t lenReply[4];
        writeLe32(lenReply,
                  kUseAdvertisedLenForControlReply01 ? static_cast<uint32_t>(kSelectorAdvertisedLen)
                                                     : kSelectorReadbackLen);
        notifyFramedControlReply(refs.authControl, 0x01, lenReply, sizeof(lenReply));
      }
      refs.setAuthParserSchedulerState(1,
                                       3,
                                       static_cast<uint16_t>(kSelectorDataMessageLen),
                                       0,
                                       AUTH_RESULT_CONTINUE,
                                       0,
                                       0x01,
                                       true,
                                       true,
                                       false);
      {
        static uint8_t selectorState[kSelectorDataMessageLen];
        buildSelectorDataMessagePayload(selectorState);
        startSleepStream(refs, 0x01, 3, selectorState, static_cast<uint16_t>(sizeof(selectorState)));
      }
      *refs.freshAuthStep = 0;
      Serial.println("Auth sleep write 01 -> selector 0x01 ULEP donor sleep-log record published");
      return SLEEP_EXPORT_HANDLED_NO_STORE;

    case 0x02:
      // Runtime anchor update. Before the first authenticated disconnect this
      // keeps unread count at zero; after that disconnect it may generate the
      // first queued sleep session for the reconnect.
      if (len != 5) {
        Serial.println("Auth sleep write 02 rejected: expected 4-byte payload");
        return SLEEP_EXPORT_HANDLED_NO_STORE;
      }
      {
        const uint32_t oldAnchor = gMockSleepState.runtimeAnchor;
        const uint32_t newAnchor = readLe32(data + 1);
        if (selectorQueuedDescriptorCount(kQueueSelectorUnread) == 0U) {
          if (ppSleepPolicyShouldGenerateForAnchor(newAnchor)) {
            prepareReconnectSleepStateForAnchor(newAnchor);
            Serial.print("Auth sleep write 02 -> generated unread sleep session by policy, anchor 0x");
            Serial.println(newAnchor, HEX);
          } else if (!ppSleepPolicyHasGeneratedSession()) {
            gMockSleepState.runtimeAnchor = newAnchor;
            Serial.println("Auth sleep write 02 -> no unread sleep session before first authenticated disconnect");
          } else {
            gMockSleepState.runtimeAnchor = newAnchor;
          }
        } else {
          const bool patched = patchSelectorSlotRuntimeAnchor(oldAnchor, newAnchor);
          gMockSleepState.runtimeAnchor = newAnchor;
          if (selectorQueuedDescriptorCount(kQueueSelectorUnread) > 0U) {
            ppSleepPolicyMarkAnchorUpdated(newAnchor, "runtime anchor update");
          }
          if (!patched && selectorQueuedDescriptorCount(kQueueSelectorUnread) > 0U) {
            Serial.println("Auth sleep write 02 -> slot patch sentinel missing; preserving fallback rebuild");
            rebuildSelectorSlotImage();
          }
        }
      }
      clearSleepStreamStage();
      notifyAuthControl(refs.authControl, kSleepReply02Ok, sizeof(kSleepReply02Ok));
      refs.setAuthParserSchedulerState(1, 4, 2, 2, 1, 0, 0x02, true, false, false);
      Serial.print("Auth sleep write 02 -> runtime anchor 0x");
      Serial.println(gMockSleepState.runtimeAnchor, HEX);
      return SLEEP_EXPORT_HANDLED_NO_STORE;

    case 0x15:
      // Consume one queued unread descriptor after the app has accepted it.
      if (len != 2 || data[1] != kQueueSelectorUnread) {
        Serial.println("Auth sleep write 15 rejected: expected selector 0x01");
        return SLEEP_EXPORT_HANDLED_NO_STORE;
      }
      if (selectorQueuedDescriptorCount(kQueueSelectorUnread) == 0U) {
        Serial.println("Auth sleep write 15 rejected: queued descriptor count is zero");
        return SLEEP_EXPORT_HANDLED_NO_STORE;
      }
      clearOneQueuedDescriptor(kQueueSelectorUnread);
      clearSleepStreamStage();
      notifyAuthControl(refs.authControl, kSleepReply15Ok, sizeof(kSleepReply15Ok));
      refs.setAuthParserSchedulerState(1, 4, 2, 2, 1, 0, 0x15, true, false, false);
      Serial.print("Auth sleep write 15 -> queued descriptor count now ");
      Serial.println(selectorQueuedDescriptorCount(kQueueSelectorUnread));
      return SLEEP_EXPORT_HANDLED_NO_STORE;

    case 0x16:
      // Report how many unread sleep descriptors are currently queued.
      if (len != 2 || data[1] != kQueueSelectorUnread) {
        Serial.println("Auth sleep write 16 rejected: expected selector 0x01");
        return SLEEP_EXPORT_HANDLED_NO_STORE;
      }
      {
        uint8_t countReply[4];
        writeLe32(countReply, selectorQueuedDescriptorCount(kQueueSelectorUnread));
        notifyFramedControlReply(refs.authControl, 0x16, countReply, sizeof(countReply));
      }
      refs.setAuthParserSchedulerState(1, 4, 6, 6, 1, 0, 0x16, true, false, false);
      Serial.print("Auth sleep write 16 -> queued descriptor count ");
      Serial.println(selectorQueuedDescriptorCount(kQueueSelectorUnread));
      return SLEEP_EXPORT_HANDLED_NO_STORE;

    case 0x26:
      {
        uint8_t payload[0x180] = { 0 };
        size_t builtLen = 0;
        uint32_t expectedLen = 0;
        if (!buildTokenizedSleepPayload(data, len, payload, sizeof(payload), &builtLen, &expectedLen)) {
          Serial.println("Auth sleep write 26 rejected: malformed token request");
          return SLEEP_EXPORT_HANDLED_NO_STORE;
        }
        if (builtLen != expectedLen) {
          Serial.print("Auth sleep write 26 rejected: built len ");
          Serial.print(static_cast<uint32_t>(builtLen));
          Serial.print(" expected ");
          Serial.println(expectedLen);
          return SLEEP_EXPORT_HANDLED_NO_STORE;
        }
        refs.clearPostServiceDHandoff();
        notifyAuthControl(refs.authControl, refs.authReply26, refs.authReply26Len);
        refs.setAuthParserSchedulerState(1,
                                         7,
                                         static_cast<uint16_t>(builtLen),
                                         0,
                                         AUTH_RESULT_CONTINUE,
                                         0,
                                         0x26,
                                         true,
                                         true,
                                         false);
        startSleepStream(refs, 0x26, 7, payload, static_cast<uint16_t>(builtLen));
        *refs.freshAuthStep = 0;
        Serial.print("Auth sleep write 26 -> tokenized payload len=");
        Serial.println(static_cast<uint32_t>(builtLen));
        return SLEEP_EXPORT_HANDLED_NO_STORE;
      }

    case 0x27:
      notifyAuthControl(refs.authControl, kSleepReply27Ok, sizeof(kSleepReply27Ok));
      refs.setAuthParserSchedulerState(1, 4, 2, 2, 1, 0, 0x27, true, false, false);
      Serial.println("Auth sleep write 27 -> staged export ack");
      return SLEEP_EXPORT_HANDLED_NO_STORE;

    case 0x2d:
      {
        uint8_t classification = (selectorQueuedDescriptorCount(kQueueSelectorUnread) > 0U) ? 1 : 0;
        notifyFramedControlReply(refs.authControl, 0x2d, &classification, 1);
        refs.setAuthParserSchedulerState(1, 4, 3, 3, 1, 0, 0x2d, true, false, false);
        Serial.print("Auth sleep write 2d -> classification ");
        Serial.println(classification);
      }
      return SLEEP_EXPORT_HANDLED_NO_STORE;

    case 0x30:
      if (len != 2 || data[1] > 1) {
        Serial.println("Auth sleep write 30 rejected: expected 0/1 payload");
        return SLEEP_EXPORT_HANDLED_NO_STORE;
      }
      gMockSleepState.incompleteFlag = data[1];
      rebuildSelectorSlotImage();
      clearSleepStreamStage();
      notifyAuthControl(refs.authControl, kSleepReply30Ok, sizeof(kSleepReply30Ok));
      refs.setAuthParserSchedulerState(1, 4, 2, 2, 1, 0, 0x30, true, false, false);
      Serial.print("Auth sleep write 30 -> incomplete flag ");
      Serial.println(gMockSleepState.incompleteFlag);
      return SLEEP_EXPORT_HANDLED_NO_STORE;

    case 0x31:
      if (len != 4) {
        Serial.println("Auth sleep write 31 rejected: expected 3-byte payload");
        return SLEEP_EXPORT_HANDLED_NO_STORE;
      }
      notifyAuthControl(refs.authControl, kSleepReply31Ok, sizeof(kSleepReply31Ok));
      refs.setAuthParserSchedulerState(1, 4, 2, 2, 1, 0, 0x31, true, false, false);
      Serial.println("Auth sleep write 31 -> timing/config ack");
      return SLEEP_EXPORT_HANDLED_NO_STORE;

    default:
      return SLEEP_EXPORT_NOT_HANDLED;
  }
}

void ppProcessAuthenticatedSleepStream(const SleepExportRuntimeRefs& refs)
{
  if (!gSleepStreamStage.active) {
    return;
  }
  if (*refs.pairingStage != PAIR_STAGE_AUTHENTICATED) {
    clearSleepStreamStage();
    return;
  }
  if (static_cast<int32_t>(millis() - gSleepStreamStage.nextAt) < 0) {
    return;
  }
  emitNextSleepStreamChunk(refs);
}

void ppPrepareReconnectSleepDataForAuthenticatedDisconnect()
{
  ppSleepPolicyLoadIfNeeded();
  if (!ppSleepPolicyHasGeneratedSession()) {
    ensureUnreadQueuePrepared();
    ppSleepPolicyArmInitialGeneration("arm initial sleep generation");
    Serial.println("Authenticated disconnect -> armed first unread sleep generation for next 02 anchor");
  } else {
    ensureUnreadQueuePrepared();
    Serial.println("Authenticated disconnect -> preserving sleep queue; 18h generation policy checked on next 02 anchor");
  }
}

void ppResetReconnectSleepDataState()
{
  resetReconnectSleepStateInternal();
}

void ppClearReconnectSleepPolicyState()
{
  resetReconnectSleepStateInternal();
  ppSleepPolicyClearPersistedState();
}
