#include <bluefruit.h>
#include <nrf_soc.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#include "pp_app_lifecycle.h"
#include "pp_auth_bootstrap.h"
#include "pp_auth_material.h"
#include "pp_auth_blob.h"
#include "pp_ble_dispatch.h"
#include "pp_ble_services.h"
#include "pp_ble_topology.h"
#include "pp_button_runtime.h"
#include "pp_cccd_seed.h"
#include "pp_connection_coordinator.h"
#include "pp_debug_ble.h"
#include "pp_debug_utils.h"
#include "pp_first_pair_flow.h"
#include "pp_pair_first_time.h"
#include "pp_pair_helpers.h"
#include "pp_pair_reconnect.h"
#include "pp_pair_runtime_support.h"
#include "pp_pair_state.h"
#include "pp_protocol_data.h"
#include "pp_reconnect_flow.h"
#include "pp_service_d_local.h"
#include "pp_sleep_export.h"
#include "pp_persistence.h"
#include "pp_types.h"

using namespace Adafruit_LittleFS_Namespace;

static const char DEVICE_NAME[] = "Pokemon GO Plus +";
static const char BUILD_TAG[] = "donor-serviceflow-v190-serviceb-parity-0517";
// Pairing-eligible advertising state discovered from gotchac3log20.
static uint8_t plusplusManufacturerData[] = {
  0x53, 0x05,
  0x01, 0xae, 0xde, 0x00, 0xf0, 0xbe, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static constexpr uint16_t SERVICE_C_NOTIFY_LEN = 74;

static constexpr uint16_t REAL_PLUSPLUS_ADV_INTERVAL_UNITS = 160;
static constexpr uint16_t REAL_PLUSPLUS_CONN_INTERVAL_MIN_UNITS = 6;
static constexpr uint16_t REAL_PLUSPLUS_CONN_INTERVAL_MAX_UNITS = 24;
static constexpr uint16_t REAL_PLUSPLUS_CONN_TIMEOUT_UNITS = 500;
static constexpr uint16_t SHORT_VALUE_LEN = 32;
static constexpr uint16_t MEDIUM_VALUE_LEN = 128;
static constexpr uint16_t LONG_VALUE_LEN = 378;
static constexpr uint8_t BATTERY_LEVEL_DEFAULT = 100;
static constexpr char VERSION_STRING[] = "3.900.1.115r1";
static constexpr uint32_t FIRSTPAIR_SERVICEC_NOTIFY1_DELAY_MS = 1200;
static constexpr uint32_t FIRSTPAIR_SERVICEC_NOTIFY2_DELAY_MS = 1650;
static constexpr uint32_t FIRSTPAIR_SERVICEC_NOTIFY3_DELAY_MS = 1750;
static constexpr uint32_t FRESH_AUTH_STEP_DELAY_MS = 90;
static constexpr uint32_t FRESH_AUTH_18_TO_REPLY_DELAY_MS = 80;
static constexpr uint32_t FRESH_AUTH_18_TO_16_DELAY_MS = 25;
static constexpr uint32_t FRESH_AUTH_WAIT_FOR_28_WRITE_MS = 650;
static constexpr uint32_t POST_AUTH_IDLE_BUTTON_FIRST_DELAY_MS = 1320;
static constexpr uint32_t POST_AUTH_IDLE_BUTTON_PERIOD_MS = 500;
static constexpr uint8_t POST_AUTH_IDLE_BUTTON_COUNT = 48;
static constexpr uint32_t RUNTIME_BUTTON_IDLE_PERIOD_MS = 500;
static constexpr uint32_t RUNTIME_BUTTON_FIRST_IDLE_DELAY_MS = 500;
static constexpr uint32_t RUNTIME_BUTTON_ACK_TIMEOUT_MS = 350;
static constexpr uint8_t RUNTIME_BUTTON_IDLE_COUNT_BEFORE_ACTION = 10;
static constexpr uint8_t RUNTIME_BUTTON_PRESTATE_SEND_COUNT = 3;
static constexpr uint32_t RUNTIME_BUTTON_PRESTATE_SEND_GAP_MS = 15;
static constexpr uint32_t POST_AUTH_READY_18_DELAY_MS = 20300;
static constexpr uint32_t PHYSICAL_RESET_BUTTON_RAW_PIN = NRF_GPIO_PIN_MAP(0, 17);
static constexpr uint32_t PHYSICAL_RESET_BUTTON_HOLD_MS = 5000;
static constexpr uint32_t DISCONNECTED_SLEEP_TIMEOUT_MS = 15UL * 60UL * 1000UL;
DebugBLEService serviceA("addc3e26-4aa5-4c1a-8a6a-735db4e01c6c");
DebugBLEService batteryService(0x180F);
DebugBLEService serviceB("2bbe7f7c-7304-4466-8407-8eaf89f8ce45");
DebugBLEService serviceC("6b7f9c98-8588-4624-8a94-38c9bce32685");
DebugBLEService serviceD("bbe87709-5b89-4433-ab7f-8b8eef0d8e37");
DebugBLEService serviceE("21c50462-67cb-63a3-5c4c-82b5b9939aeb");
DebugBLEService authService("138c35b6-23a0-459c-9869-acb9228be64f");

DebugBLECharacteristic batteryLevelChar(0x2A19, "batteryLevelChar");

BLECharacteristic serviceAChar1("addc3e26-4aa5-4c1a-8a6a-735db4e01c6d");
BLECharacteristic serviceAChar2("addc3e26-4aa5-4c1a-8a6a-735db4e01c6e");
BLECharacteristic serviceAChar3("addc3e26-4aa5-4c1a-8a6a-735db4e01c6f");
BLECharacteristic serviceAChar4("addc3e26-4aa5-4c1a-8a6a-735db4e01c70");
BLECharacteristic serviceAChar5("addc3e26-4aa5-4c1a-8a6a-735db4e01c71");

BLECharacteristic serviceBChar1("2bbe7f7c-7304-4466-8407-8eaf89f8ce46");
BLECharacteristic serviceBChar2("2bbe7f7c-7304-4466-8407-8eaf89f8ce47");
BLECharacteristic serviceBChar3("2bbe7f7c-7304-4466-8407-8eaf89f8ce49");

DebugBLECharacteristic serviceCChar1("68282de6-e643-4caa-a6b4-e96f46ed988a", "serviceCChar1");
DebugBLECharacteristic serviceCChar2("290278ac-5eb9-47df-899d-e904ab62b4b3", "serviceCChar2");

DebugBLECharacteristic serviceDChar1("bbe87709-5b89-4433-ab7f-8b8eef0d8e38", "serviceDChar1");
DebugBLECharacteristic serviceDChar2("bbe87709-5b89-4433-ab7f-8b8eef0d8e39", "serviceDChar2");
DebugBLECharacteristic serviceDChar3("bbe87709-5b89-4433-ab7f-8b8eef0d8e3a", "serviceDChar3");

DebugBLECharacteristic ledWriteChar("21c50462-67cb-63a3-5c4c-82b5b9939aec", "ledWriteChar");
DebugBLECharacteristic buttonNotifyChar("21c50462-67cb-63a3-5c4c-82b5b9939aed", "buttonNotifyChar");
DebugBLECharacteristic serviceEChar3("21c50462-67cb-63a3-5c4c-82b5b9939aee", "serviceEChar3");
DebugBLECharacteristic serviceEChar4("21c50462-67cb-63a3-5c4c-82b5b9939aef", "serviceEChar4");
DebugBLECharacteristic versionChar("21c50462-67cb-63a3-5c4c-82b5b9939af0", "versionChar");
DebugBLECharacteristic configChar("1e9d6f80-9b71-4bbf-9ba3-1d4d97212b49", "configChar");

DebugBLECharacteristic authControl("7e11e71f-97f6-4578-aa23-2f0b2f8a946e", "authControl");
DebugBLECharacteristic authBlob("125266b9-bcfd-4904-8be0-530ef0b93372", "authBlob");

static uint16_t activeConnHandle = BLE_CONN_HANDLE_INVALID;
static bool disconnectedSleepArmed = false;
static uint32_t disconnectedSleepAt = 0;
static bool buttonPulseActive = false;
static uint32_t buttonPulseReleaseAt = 0;
static bool runtimeButtonEngineActive = false;
static bool runtimeButtonActionArmed = false;
static bool runtimeButtonAwaitingAck = false;
static bool runtimeButtonSettlingToIdle = false;
static uint8_t runtimeButtonFamilyIndex = 0;
static uint8_t runtimeButtonCurrentFamily = 0;
static uint8_t runtimeButtonIdleCount = 0;
static uint32_t runtimeButtonAt = 0;
static PairingStage pairingStage = PAIR_STAGE_DISCOVERY;
static uint8_t pairEventClass = 0x21;
static uint8_t activePair[2] = { 0x00, 0x00 };
static uint8_t candidatePair[2] = { 0x01, 0x02 };
static uint8_t stagedPair[2] = { 0x00, 0x00 };
static bool stagedPairValid = false;
static uint32_t connStartedAt = 0;
static uint8_t serviceCTransferState = 0x00;
static bool servicesReady = false;
static uint8_t serviceCBurstStep = 0;
static uint32_t serviceCBurstAt = 0;
static bool serviceCNotifyEnabled = false;
static bool serviceBNotifyEnabled = false;
static bool buttonNotifyEnabled = false;
static bool serviceDNotifyEnabled = false;
static bool authControlNotifyEnabled = false;
static bool authBlobNotifyEnabled = false;
static uint8_t freshAuthStep = 0;
static uint32_t freshAuthAt = 0;
static bool freshAuthStarted = false;
static bool freshAuthBlobSent = false;
static bool userCccdsSeeded = false;
static uint8_t postAuthIdleButtonRemaining = 0;
static uint32_t postAuthIdleButtonAt = 0;
static bool postAuthReady18Pending = false;
static uint32_t postAuthReady18At = 0;
static bool persistentReconnectReady = false;
static bool reconnectServiceDArmed = false;
static bool reconnectCompactServiceDMode = false;
static uint8_t reconnectAuthReply24[sizeof(PLUSPLUS_AUTH_CTRL_REPLY_2401)] = {
  0x24, 0x00, 0x23, 0xf4, 0x09
};
static AuthReplyIndexSlotState authReplyIndexTable[32] = { { { 0x00, 0x00, 0x00 }, false, false } };
static uint8_t currentAttemptCentralNonce[4] = { 0x00, 0x00, 0x00, 0x00 };
static bool currentAttemptCentralNonceValid = false;
static uint8_t persistedAcceptedCentralNonce[4] = { 0x00, 0x00, 0x00, 0x00 };
static bool persistedAcceptedCentralNonceValid = false;
static bool firstPairSawReal36 = false;
static bool firstPairSawReal28 = false;

static DonorAuthRuntimeState donorAuthRuntime = { 0 };
static AuthParserOutputState authParserOutput = { 0 };

static uint8_t firstPairCertState = CERT_STATE_IDLE;

static uint8_t serviceARead3[6] = { 0x00 };
static uint8_t serviceARead4[8] = { 0x00 };
static uint8_t serviceAStatus[2] = { 0x00, 0x00 };
static bool persistentStoreReady = false;
static bool physicalResetButtonActive = false;
static bool physicalResetButtonTriggered = false;
static uint32_t physicalResetButtonPressedAt = 0;

static uint8_t serviceBStatus1[4] = { 0x00, 0x00, 0x00, 0x00 };
static uint8_t serviceBStatus2[4] = { 0x00, 0x00, 0x00, 0x00 };
static uint8_t serviceBRead3[2] = { 0x01, 0x00 };
static bool postServiceDHandoffActive = false;
static bool postServiceDServiceBReadSeen = false;
static uint8_t postServiceDServiceBValue = 0x04;

static uint8_t serviceCState[SERVICE_C_NOTIFY_LEN] = { 0x00 };
static uint8_t serviceDStatus[4] = { 0x00, 0x00, 0x00, 0x00 };
static uint8_t serviceDReadBlob[LONG_VALUE_LEN];
static uint16_t serviceDReadPendingLen = 0;
static uint8_t serviceDType3ReadPayload[36] = { 0x03, 0x00, 0x00, 0x00 };
static uint8_t serviceDType5ReadPayload[20] = { 0x05, 0x00, 0x00, 0x00 };
static uint8_t persistedServiceDSelector4[16] = { 0x00 };
static bool persistedServiceDSelector4Valid = false;
static uint8_t persistedServiceDSelector5 = 0x00;
static bool persistedServiceDSelector5Valid = false;
static uint8_t persistedServiceDRecord6[10] = { 0x00 };
static bool persistedServiceDRecord6Valid = false;
static uint32_t persistedServiceDRecord1A = 0;
static bool persistedServiceDRecord1AValid = false;
static uint8_t serviceDRuntimeBlock[0x80] = { 0x00 };
static uint8_t lastOlderServiceDType2Write[52] = { 0x00 };
static bool lastOlderServiceDType2WriteValid = false;
static uint8_t buttonState[2] = { 0x00, 0x00 };
static uint8_t configState[1] = { 0x01 };
static uint8_t authBlobValue[LONG_VALUE_LEN];
static uint8_t authBlobVisibleValue[AUTH_BLOB_VISIBLE_LEN];
static AuthBlobStageState authBlobStage = { false, 0, 0, 0, 0, 0 };

static ServiceDCertState serviceDCertState = { false, false, false, SERVICE_D_FLOW_NONE, 0, 0, 0, 0xff, 0, { 0x00, 0x00, 0x00, 0x00 } };

#include "pp_wiring.h"


static void cccd_write_callback(uint16_t conn_handle, BLECharacteristic* chr, uint16_t value)
{
  ppCccdWriteCallback(makeBleDispatchRefs(), conn_handle, chr, value);
}

static void read_authorize_callback(uint16_t conn_handle, BLECharacteristic* chr, ble_gatts_evt_read_t* request)
{
  ppReadAuthorizeCallback(makeBleDispatchRefs(), conn_handle, chr, request);
}

static void service_write_callback(uint16_t conn_handle, BLECharacteristic* chr, uint8_t* data, uint16_t len)
{
  ppServiceWriteCallback(makeBleDispatchRefs(), conn_handle, chr, data, len);
}

static void setup_services()
{
  ppSetupPokemonGoPlusTopology(makeBleTopologyRefs(), makeBleSetupCallbacks());
}

static void configure_identity()
{
  ppConfigureIdentity(makeBleIdentityConfig());
}

static void configure_gap_preferences()
{
  ppConfigureGapPreferences(makeBleGapPreferenceConfig());
}

static void start_advertising()
{
  ppStartAdvertising(makeBleAdvertisingConfig());
}

static void connect_callback(uint16_t conn_handle)
{
  ppHandleConnect(makeAppLifecycleRefs(), conn_handle);
}

static void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  ppHandleDisconnect(makeAppLifecycleRefs(), conn_handle, reason);
}

void setup()
{
  ppRunBootSequence(makeAppBootRefs());
}

void loop()
{
  ppRunMainLoopTick(makeAppLoopRefs());
}

