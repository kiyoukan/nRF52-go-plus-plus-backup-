#pragma once

#include <stdint.h>

static constexpr uint8_t MANUFACTURER_STATE_PAIRABLE = 0x01;
static constexpr uint8_t MANUFACTURER_STATE_RECONNECT = 0x01;
static constexpr uint8_t CONFIG_RESET_PAIRED_STATE_MAGIC = 0xA5;
static constexpr uint32_t PERSISTED_PAIR_STATE_MAGIC = 0x50475050;
static constexpr uint16_t PERSISTED_PAIR_STATE_VERSION = 1;
static constexpr uint16_t AUTH_BLOB_VISIBLE_LEN = 262;
static constexpr char PERSISTED_PAIR_STATE_PATH[] = "/pair_state.bin";

enum PairingStage : uint8_t {
  PAIR_STAGE_DISCOVERY = 0,
  PAIR_STAGE_STAGED = 1,
  PAIR_STAGE_COMMITTED = 2,
  PAIR_STAGE_AUTHENTICATED = 3
};

struct PersistedPairStateRecord {
  uint32_t magic;
  uint16_t version;
  uint16_t size;
  uint8_t paired;
  uint8_t acceptedNonceValid;
  uint8_t serviceDSelector4Valid;
  uint8_t serviceDSelector5Valid;
  uint8_t serviceDRecord6Valid;
  uint8_t serviceDRecord1AValid;
  uint8_t reserved0;
  uint8_t reserved1;
  uint8_t acceptedNonce[4];
  uint8_t serviceDSelector4[16];
  uint8_t serviceDSelector5;
  uint8_t serviceDRecord6[10];
  uint32_t serviceDRecord1A;
};

struct ServiceDCertState {
  bool active;
  bool readPrepared;
  bool completionPending;
  uint8_t laneKind;
  uint8_t laneStep;
  uint8_t expectedWriteType;
  uint16_t expectedWriteLen;
  uint8_t lastObservedType;
  uint16_t lastObservedLen;
  uint8_t lastStatus[4];
};

static constexpr uint8_t SERVICE_D_FLOW_NONE = 0;
static constexpr uint8_t SERVICE_D_FLOW_COMPACT = 1;
static constexpr uint8_t SERVICE_D_FLOW_OLDER = 2;

struct DonorAuthRuntimeState {
  uint8_t mode;
  uint8_t auth18Selector;
  uint8_t auth18NoncePrefix[3];
  uint8_t auth18Payload0;
  uint8_t auth18Payload1;
  uint8_t auth18Payload2;
  uint8_t localFamily;
  uint8_t localStatus;
  uint8_t localCommandCode;
  uint8_t readinessByte;
  bool auth18Observed;
  bool workerActive;
  bool pendingLatch;
  bool specialArmed;
  bool event30000Seen;
  bool event30001Seen;
  bool event3Seen;
  bool event4Seen;
  bool app16Latched;
};

struct AuthParserOutputState {
  uint8_t schedulerState;
  uint8_t outputClass;
  uint16_t outputLen;
  uint16_t outputOffset;
  uint8_t resultCode;
  uint8_t continuationKind;
  uint8_t lastOpcode;
  bool outputPending;
  bool streamPending;
  bool notifyInFlight;
};

struct AuthReplyIndexSlotState {
  uint8_t value[3];
  bool valid;
  bool touched;
};

enum AuthParserResultCode : uint8_t {
  AUTH_RESULT_IDLE = 0,
  AUTH_RESULT_SUCCESS = 1,
  AUTH_RESULT_CONTINUE = 2,
  AUTH_RESULT_RETRY = 3,
  AUTH_RESULT_ERROR = 4,
  AUTH_RESULT_FAILURE = 5
};

enum FirstPairCertState : uint8_t {
  CERT_STATE_IDLE = 0,
  CERT_STATE_24_SENT = 1,
  CERT_STATE_25_AND_BLOB_SENT = 2,
  CERT_STATE_APP_NONCE_OBSERVED = 3,
  CERT_STATE_DEVICE_18_ACKED = 4,
  CERT_STATE_APP_16_OBSERVED = 5,
  CERT_STATE_DEVICE_16_ACKED = 6,
  CERT_STATE_REAL_36_OBSERVED = 7,
  CERT_STATE_REAL_28_OBSERVED = 8,
  CERT_STATE_LOCAL_CERT_ACTIVE = 9,
  CERT_STATE_AUTH_COMPLETE = 10
};

enum DonorAuthRuntimeStep : uint8_t {
  DONOR_AUTH_RUNTIME_IDLE = 0,
  DONOR_AUTH_RUNTIME_30000 = 14,
  DONOR_AUTH_RUNTIME_EVENT3_WAIT = 15,
  DONOR_AUTH_RUNTIME_30001 = 16,
  DONOR_AUTH_RUNTIME_EVENT3_READY = 37,
  DONOR_AUTH_RUNTIME_EVENT4 = 38,
  DONOR_AUTH_RUNTIME_LOCAL29 = 39
};

struct AuthBlobStageState {
  bool ready;
  uint8_t stageClass;
  uint16_t totalLen;
  uint16_t offset;
  uint8_t streamFlags;
  uint8_t lastOpcode;
};
