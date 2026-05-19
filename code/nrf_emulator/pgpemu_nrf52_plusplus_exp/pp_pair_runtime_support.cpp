#include <Arduino.h>
#include <string.h>

#include "pp_pair_runtime_support.h"

const char* ppFirstPairCertStateLabel(uint8_t state)
{
  switch (state) {
    case CERT_STATE_IDLE: return "idle";
    case CERT_STATE_24_SENT: return "24_sent";
    case CERT_STATE_25_AND_BLOB_SENT: return "25_blob_sent";
    case CERT_STATE_APP_NONCE_OBSERVED: return "app_nonce_observed";
    case CERT_STATE_DEVICE_18_ACKED: return "device_18_acked";
    case CERT_STATE_APP_16_OBSERVED: return "app_16_observed";
    case CERT_STATE_DEVICE_16_ACKED: return "device_16_acked";
    case CERT_STATE_REAL_36_OBSERVED: return "real_36_observed";
    case CERT_STATE_REAL_28_OBSERVED: return "real_28_observed";
    case CERT_STATE_LOCAL_CERT_ACTIVE: return "local_cert_active";
    case CERT_STATE_AUTH_COMPLETE: return "auth_complete";
    default: return "unknown";
  }
}

void ppSetFirstPairCertState(const PairRuntimeSupportRefs& refs,
                             uint8_t nextState,
                             const char* reason)
{
  if (*refs.firstPairCertState == nextState) {
    return;
  }
  Serial.print("First-pair cert state ");
  Serial.print(ppFirstPairCertStateLabel(*refs.firstPairCertState));
  Serial.print(" -> ");
  Serial.print(ppFirstPairCertStateLabel(nextState));
  Serial.print(" via ");
  Serial.println(reason);
  *refs.firstPairCertState = nextState;
}

void ppResetDonorAuthRuntime(const PairRuntimeSupportRefs& refs)
{
  memset(refs.donorAuthRuntime, 0, sizeof(*refs.donorAuthRuntime));
}

void ppResetAuthParserOutput(const PairRuntimeSupportRefs& refs)
{
  memset(refs.authParserOutput, 0, sizeof(*refs.authParserOutput));
  refs.setAuthParserSchedulerState(0, 0, 0, 0, AUTH_RESULT_IDLE, 0, 0, false, false, false);
}

void ppLogDonorAuthRuntime(const PairRuntimeSupportRefs& refs, const char* reason)
{
  const DonorAuthRuntimeState& s = *refs.donorAuthRuntime;
  Serial.print("Donor auth runtime via ");
  Serial.print(reason);
  Serial.print(": mode=");
  Serial.print(s.mode);
  Serial.print(" auth18Selector=");
  Serial.print(s.auth18Selector);
  Serial.print(" auth18NoncePrefix=");
  refs.printHex(s.auth18NoncePrefix, sizeof(s.auth18NoncePrefix));
  Serial.print(" auth18Payload=");
  Serial.print(s.auth18Payload0, HEX);
  Serial.print("/");
  Serial.print(s.auth18Payload1, HEX);
  Serial.print("/");
  Serial.print(s.auth18Payload2, HEX);
  Serial.print(" auth18Observed=");
  Serial.print(s.auth18Observed ? "true" : "false");
  Serial.print(" worker=");
  Serial.print(s.workerActive ? "true" : "false");
  Serial.print(" ready=");
  Serial.print(s.readinessByte);
  Serial.print(" armed=");
  Serial.print(s.specialArmed ? "true" : "false");
  Serial.print(" latch=");
  Serial.print(s.pendingLatch ? "true" : "false");
  Serial.print(" app16=");
  Serial.print(s.app16Latched ? "true" : "false");
  Serial.print(" 29fam=0x");
  Serial.print(s.localFamily, HEX);
  Serial.print(" 29status=0x");
  Serial.print(s.localStatus, HEX);
  Serial.print(" 29cmd=0x");
  Serial.println(s.localCommandCode, HEX);
}

uint8_t ppDonorLocal29CommandCodeForFamily(uint8_t family)
{
  switch (family) {
    case 0x00:
    case 0x10:
      return 0x00;
    case 0x01:
      return 0x67;
    case 0x02:
    case 0x20:
      return 0x68;
    case 0x03:
    case 0x21:
      return 0x69;
    case 0x11:
      return 0x64;
    case 0x12:
      return 0x66;
    default:
      return 0x00;
  }
}

uint8_t ppDeriveDonorLocalFamilyFromParserBytes(const PairRuntimeSupportRefs& refs)
{
  uint8_t family = refs.donorAuthRuntime->auth18NoncePrefix[0];
  if ((family & 0x20) == 0x20) {
    return static_cast<uint8_t>(0x20 | (family & 0x01));
  }
  if ((family & 0x10) == 0x10) {
    return static_cast<uint8_t>(0x10 | (family & 0x03));
  }
  return static_cast<uint8_t>(family & 0x03);
}

void ppQueueDonorAuthRuntimeStep(const PairRuntimeSupportRefs& refs,
                                 uint8_t nextStep,
                                 uint32_t delayMs,
                                 const char* reason)
{
  *refs.freshAuthStep = nextStep;
  *refs.freshAuthAt = millis() + delayMs;
  Serial.print("Queued donor auth runtime step ");
  Serial.print(nextStep);
  Serial.print(" via ");
  Serial.println(reason);
}

void ppSeedAuthReplyIndexTable(const PairRuntimeSupportRefs& refs,
                               const uint8_t* defaultReply2401)
{
  memset(refs.authReplyIndexTable, 0, refs.authReplyIndexTableLen * sizeof(AuthReplyIndexSlotState));
  memcpy(refs.authReplyIndexTable[1].value, defaultReply2401 + 2, 3);
  refs.authReplyIndexTable[1].valid = true;
  refs.authReplyIndexTable[1].touched = true;
}

void ppUpdateAuthReplyIndexSlot(const PairRuntimeSupportRefs& refs,
                                uint8_t index,
                                const uint8_t* value3,
                                const char* reason)
{
  if (index >= refs.authReplyIndexTableLen || value3 == nullptr) {
    return;
  }
  memcpy(refs.authReplyIndexTable[index].value, value3, 3);
  refs.authReplyIndexTable[index].valid = true;
  refs.authReplyIndexTable[index].touched = true;
  Serial.print("Auth reply slot[");
  Serial.print(index);
  Serial.print("] via ");
  Serial.print(reason);
  Serial.print(" -> ");
  refs.printHex(refs.authReplyIndexTable[index].value, 3);
  Serial.print(" valid=");
  Serial.print(refs.authReplyIndexTable[index].valid ? "true" : "false");
  Serial.print(" touched=");
  Serial.print(refs.authReplyIndexTable[index].touched ? "true" : "false");
  Serial.println();
}

void ppBuildIndexedAuthReply24(const PairRuntimeSupportRefs& refs,
                               uint8_t index,
                               uint8_t* out5,
                               const char* reason)
{
  if (out5 == nullptr) {
    return;
  }
  out5[0] = 0x24;
  out5[1] = 0x00;
  if (index < refs.authReplyIndexTableLen && refs.authReplyIndexTable[index].valid) {
    memcpy(out5 + 2, refs.authReplyIndexTable[index].value, 3);
  } else {
    memset(out5 + 2, 0, 3);
  }
  Serial.print("Indexed auth 24 reply via ");
  Serial.print(reason);
  Serial.print(" idx=");
  Serial.print(index);
  Serial.print(" -> ");
  refs.printHex(out5, 5);
  Serial.println();
}
