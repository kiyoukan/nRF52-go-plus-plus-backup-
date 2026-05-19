#include <Arduino.h>
#include <bluefruit.h>
#include <nrf_gpio.h>
#include <nrf_soc.h>

#include "pp_app_lifecycle.h"
#include "pp_sleep_export.h"

namespace {

static void resetPairingCommon(const AppLifecycleRefs& refs, const char* prefix, const char* reason)
{
  Serial.print(prefix);
  Serial.println(reason);
  refs.clearPersistentPairState(reason);
  ppClearReconnectSleepPolicyState();
  refs.resetUnpairedState();
  if (*refs.activeConnHandle != BLE_CONN_HANDLE_INVALID) {
    sd_ble_gap_disconnect(*refs.activeConnHandle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
  } else {
    refs.startAdvertising();
  }
}

static void enterDisconnectedSystemOff(const AppLifecycleRefs& refs)
{
  if (*refs.activeConnHandle != BLE_CONN_HANDLE_INVALID) {
    return;
  }

  Serial.println("Disconnected idle timeout reached -> entering System OFF");
  Bluefruit.Advertising.stop();
  nrf_gpio_cfg_sense_input(refs.physicalResetButtonRawPin,
                           NRF_GPIO_PIN_PULLUP,
                           NRF_GPIO_PIN_SENSE_LOW);
  Serial.print("Wake source armed on raw pin P0.17 (#");
  Serial.print(refs.physicalResetButtonRawPin);
  Serial.println(") low sense");
  Serial.flush();
  delay(20);

  uint32_t status = sd_power_system_off();
  Serial.print("sd_power_system_off returned status=0x");
  Serial.println(status, HEX);
  nrf_gpio_cfg_input(refs.physicalResetButtonRawPin, NRF_GPIO_PIN_PULLUP);
  ppArmDisconnectedSleepTimer(refs, "system off failed");
}

static void refreshBatteryLevel(const AppLifecycleRefs& refs, const char* reason)
{
  if (refs.batteryLevelChar == nullptr) {
    return;
  }

  refs.batteryLevelChar->write(&refs.batteryLevelDefault, 1);
  uint8_t stored = 0;
  uint16_t len = refs.batteryLevelChar->read(&stored, sizeof(stored));
  Serial.print("Battery level refreshed via ");
  Serial.print(reason);
  Serial.print(" len=");
  Serial.print(len);
  Serial.print(" value=0x");
  if (stored < 0x10) {
    Serial.print('0');
  }
  Serial.println(stored, HEX);
}

}  // namespace

void ppResetPairingFromApp(const AppLifecycleRefs& refs, const char* reason)
{
  resetPairingCommon(refs, "App-triggered pairing reset via ", reason);
}

void ppResetPairingFromPhysicalButton(const AppLifecycleRefs& refs, const char* reason)
{
  resetPairingCommon(refs, "Physical hold pairing reset via ", reason);
}

void ppDisarmDisconnectedSleepTimer(const AppLifecycleRefs& refs)
{
  *refs.disconnectedSleepArmed = false;
  *refs.disconnectedSleepAt = 0;
}

void ppArmDisconnectedSleepTimer(const AppLifecycleRefs& refs, const char* reason)
{
  if (*refs.activeConnHandle != BLE_CONN_HANDLE_INVALID) {
    return;
  }

  *refs.disconnectedSleepArmed = true;
  *refs.disconnectedSleepAt = millis() + refs.disconnectedSleepTimeoutMs;
  Serial.print("Disconnected sleep timer armed via ");
  Serial.print(reason);
  Serial.print(" timeout_ms=");
  Serial.println(refs.disconnectedSleepTimeoutMs);
}

void ppProcessDisconnectedSleepIfNeeded(const AppLifecycleRefs& refs)
{
  if (!*refs.disconnectedSleepArmed || *refs.activeConnHandle != BLE_CONN_HANDLE_INVALID) {
    return;
  }

  if (static_cast<int32_t>(millis() - *refs.disconnectedSleepAt) < 0) {
    return;
  }

  ppDisarmDisconnectedSleepTimer(refs);
  enterDisconnectedSystemOff(refs);
}

void ppProcessPhysicalResetButtonIfNeeded(const AppLifecycleRefs& refs)
{
  bool pressed = (nrf_gpio_pin_read(refs.physicalResetButtonRawPin) == 0);
  uint32_t now = millis();

  if (pressed && !*refs.physicalResetButtonActive) {
    *refs.physicalResetButtonActive = true;
    *refs.physicalResetButtonTriggered = false;
    *refs.physicalResetButtonPressedAt = now;
    Serial.print("Physical reset button press start on raw pin P0.17 (#");
    Serial.print(refs.physicalResetButtonRawPin);
    Serial.print(" hold target=");
    Serial.print(refs.physicalResetButtonHoldMs);
    Serial.println("ms");
    return;
  }

  if (!pressed && *refs.physicalResetButtonActive) {
    uint32_t heldMs = now - *refs.physicalResetButtonPressedAt;
    if (*refs.physicalResetButtonTriggered) {
      Serial.print("Physical reset button released after reset hold ms=");
      Serial.println(heldMs);
    } else {
      Serial.print("Physical reset button released early ms=");
      Serial.print(heldMs);
      Serial.print(" threshold=");
      Serial.println(refs.physicalResetButtonHoldMs);
    }
    *refs.physicalResetButtonActive = false;
    *refs.physicalResetButtonTriggered = false;
    *refs.physicalResetButtonPressedAt = 0;
    return;
  }

  if (pressed &&
      *refs.physicalResetButtonActive &&
      !*refs.physicalResetButtonTriggered &&
      (now - *refs.physicalResetButtonPressedAt) >= refs.physicalResetButtonHoldMs) {
    *refs.physicalResetButtonTriggered = true;
    Serial.print("Physical reset button held for full threshold ms=");
    Serial.println(now - *refs.physicalResetButtonPressedAt);
    ppResetPairingFromPhysicalButton(refs, "pin 17 hold");
  }
}

void ppHandleConnect(const AppLifecycleRefs& refs, uint16_t conn_handle)
{
  if (!*refs.persistentReconnectReady && *refs.pairingStage != PAIR_STAGE_DISCOVERY) {
    Serial.println("Repairing stale reconnect state -> reset to discovery");
    refs.resetUnpairedState();
  }

  ppDisarmDisconnectedSleepTimer(refs);
  *refs.activeConnHandle = conn_handle;
  *refs.connStartedAt = millis();
  *refs.serviceCNotifyEnabled = false;
  *refs.buttonNotifyEnabled = false;
  *refs.serviceDNotifyEnabled = false;
  *refs.authControlNotifyEnabled = false;
  *refs.authBlobNotifyEnabled = false;
  refs.resetVolatilePairSessionState();
  refreshBatteryLevel(refs, "connect");
  Serial.print("Connected, conn_handle=");
  Serial.println(conn_handle);
  refs.printBuildTag("ble connect");
  Serial.print("Pair stage at connect: ");
  Serial.println(refs.pairingStageLabel(*refs.pairingStage));
  refs.refreshPairingExports(false);
}

void ppHandleDisconnect(const AppLifecycleRefs& refs, uint16_t conn_handle, uint8_t reason)
{
  Serial.print("Disconnected, conn_handle=");
  Serial.print(conn_handle);
  Serial.print(" reason=0x");
  Serial.println(reason, HEX);

  *refs.activeConnHandle = BLE_CONN_HANDLE_INVALID;
  *refs.buttonPulseActive = false;
  refs.resetRuntimeButtonEngine();
  memset(refs.buttonState, 0, refs.buttonStateLen);
  memset(refs.serviceAStatus, 0, refs.serviceAStatusLen);
  *refs.connStartedAt = 0;
  *refs.serviceCNotifyEnabled = false;
  *refs.serviceBNotifyEnabled = false;
  *refs.buttonNotifyEnabled = false;
  *refs.serviceDNotifyEnabled = false;
  *refs.authControlNotifyEnabled = false;
  *refs.authBlobNotifyEnabled = false;
  refs.resetVolatilePairSessionState();
  refs.stopPostAuthRuntimeScaffolding();

  if (*refs.pairingStage != PAIR_STAGE_AUTHENTICATED) {
    Serial.println("Disconnect before auth complete -> resetting to virgin pairable state");
    refs.resetUnpairedState();
  } else {
    *refs.persistentReconnectReady = true;
    refs.prepareReconnectSleepDataOnAuthenticatedDisconnect();
    refs.setManufacturerState(refs.manufacturerReconnectState);
    refreshBatteryLevel(refs, "authenticated disconnect");
    refs.refreshPairingExports(false);
  }
  refs.startAdvertising();
  ppArmDisconnectedSleepTimer(refs, "disconnect");
}

void ppRunBootSequence(const AppBootRefs& refs)
{
  Serial.begin(115200);
  uint32_t serialWaitStart = millis();
  while (!Serial && (millis() - serialWaitStart) < 5000) {
    delay(10);
  }
  delay(1500);
  Serial.println();
  Serial.println(refs.bootBanner);
  refs.printBuildTag("boot");

  refs.seedLongReadBlob();
  refs.seedAuthBlob();

  Bluefruit.configServiceChanged(false);
  Bluefruit.configUuid128Count(40);
  Bluefruit.configAttrTableSize(8192);
  Bluefruit.configPrphConn(517, 100, 4, BLE_GATTC_WRITE_CMD_TX_QUEUE_SIZE_DEFAULT);
  Bluefruit.begin();
  Bluefruit.setName(refs.deviceName);
  Bluefruit.Periph.setConnectCallback(refs.connectCallback);
  Bluefruit.Periph.setDisconnectCallback(refs.disconnectCallback);
  nrf_gpio_cfg_input(refs.physicalResetButtonRawPin, NRF_GPIO_PIN_PULLUP);
  Serial.print("Configured physical reset button raw pin P0.17 (#");
  Serial.print(refs.physicalResetButtonRawPin);
  Serial.println(") with pull-up");

  refs.configureIdentity();
  refs.configureGapPreferences();
  refs.setupServices();
  *refs.servicesReady = true;
  *refs.persistentStoreReady = refs.beginPersistentStore();
  Serial.print("InternalFS begin -> ");
  Serial.println(*refs.persistentStoreReady ? "true" : "false");
  refs.resetUnpairedState();
  if (refs.loadPersistentPairState()) {
    refs.restorePairedStateFromPersistence();
  }
  refs.startAdvertising();
  refs.armDisconnectedSleepTimer("boot");

  Serial.println("nRF52 Plus+ prototype advertising");
}

void ppRunMainLoopTick(const AppLoopRefs& refs)
{
  refs.processPhysicalResetButtonIfNeeded();
  refs.processDisconnectedSleepIfNeeded();
  refs.processAuthenticatedSleepStream();
  refs.clearButtonPulseIfNeeded();
  refs.processRuntimeButtonEngineIfNeeded();
  refs.processPostAuthIdleButtonNotify();
  refs.processPostAuthReady18();
  refs.processServiceCFirstPairBurst();
  refs.processFreshAuthSequence();
  delay(refs.loopDelayMs);
}
