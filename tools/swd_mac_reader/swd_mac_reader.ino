// ==============================================================================
// Pokemon GO Plus + Non-Destructive SWD Public BLE MAC Reader
// ==============================================================================
// Reads the genuine Nintendo Public BLE MAC address from a non-booting / dead
// Pokemon GO Plus + (Nordic nRF52832 QFN48) over SWD without erasing flash/UICR.
//
// IMPORTANT:
// - Do NOT use NRF_FICR->DEVICEADDR (0x100000A4..0x100000A8). That register
//   contains Nordic's generic silicon Random Static address and will FAIL
//   Pokemon GO certification against your external flash FBCT certificate.
// - The real Nintendo IEEE Public BLE MAC is stored in NRF_UICR->CUSTOMER[0..1]
//   at 0x10001080..0x10001087 in little-endian byte order.
//
// Default wiring (Teyleten Robot Pro Micro / SuperMini nRF52840 -> nRF52832 QFN48):
//   Board GND                  -----> Donor Ground (USB-C outer metal shell)
//   Board VCC (3.3V)           -----> Donor QFN48 Pin 48 (VDD)
//   Board P0.06 (silkscreen 006) ---> Donor QFN48 Pin 25 (SWDCLK)
//   Board P0.08 (silkscreen 008) ---> Donor QFN48 Pin 26 (SWDIO)
//
// Adjust PIN_SWDCLK / PIN_SWDIO below if using a different nRF52840 board.
// Strictly READ-ONLY: never writes to NVMC, flash, or UICR.
// ==============================================================================

#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <nrf_gpio.h>

#define PIN_SWDCLK    NRF_GPIO_PIN_MAP(0, 6)   // P0.06 ("006" on SuperMini nRF52840)
#define PIN_SWDIO     NRF_GPIO_PIN_MAP(0, 8)   // P0.08 ("008" on SuperMini nRF52840)
#define PIN_VCC_GATE  NRF_GPIO_PIN_MAP(0, 13)  // P0.13 enables external VCC on SuperMini

// SWD Request Headers (LSB first)
#define SWD_DP_RD_IDCODE    0xA5
#define SWD_DP_WR_ABORT     0x81
#define SWD_DP_RD_CTRLSTAT  0x8D
#define SWD_DP_WR_CTRLSTAT  0xA9
#define SWD_DP_WR_SELECT    0xB1
#define SWD_DP_RD_RDBUFF    0xBD
#define SWD_AP_WR_CSW       0xA3
#define SWD_AP_WR_TAR       0x8B
#define SWD_AP_RD_DRW       0x9F

#define CSW_32BIT_NOINC     0x23000002UL
#define CSW_32BIT_AUTOINC   0x23000012UL

static bool g_fast_block_ok = false;
static bool g_initial_read_done = false;

static inline void swd_delay() {
  delayMicroseconds(1);
}

static inline void clk_pulse() {
  nrf_gpio_pin_clear(PIN_SWDCLK);
  swd_delay();
  nrf_gpio_pin_set(PIN_SWDCLK);
  swd_delay();
}

static inline void write_bit(uint8_t bit) {
  if (bit) {
    nrf_gpio_pin_set(PIN_SWDIO);
  } else {
    nrf_gpio_pin_clear(PIN_SWDIO);
  }
  clk_pulse();
}

static inline uint8_t read_bit() {
  nrf_gpio_pin_clear(PIN_SWDCLK);
  swd_delay();
  uint8_t b = (uint8_t)nrf_gpio_pin_read(PIN_SWDIO);
  nrf_gpio_pin_set(PIN_SWDCLK);
  swd_delay();
  return b;
}

static inline void swdio_as_output() {
  nrf_gpio_cfg_output(PIN_SWDIO);
}

static inline void swdio_as_input() {
  nrf_gpio_cfg_input(PIN_SWDIO, NRF_GPIO_PIN_PULLUP);
}

static void line_reset() {
  swdio_as_output();
  for (int i = 0; i < 64; i++) {
    write_bit(1);
  }
}

static void swd_init_sequence() {
  line_reset();
  // JTAG-to-SWD select sequence (0xE79E, LSB first)
  uint16_t seq = 0xE79E;
  for (int i = 0; i < 16; i++) {
    write_bit((seq >> i) & 1);
  }
  line_reset();
  for (int i = 0; i < 16; i++) {
    write_bit(0);
  }
}

static uint8_t send_header(uint8_t req) {
  swdio_as_output();
  for (int i = 0; i < 8; i++) {
    write_bit((req >> i) & 1);
  }
  swdio_as_input();
  clk_pulse();

  uint8_t ack = 0;
  for (int i = 0; i < 3; i++) {
    ack |= (read_bit() << i);
  }
  return ack;
}

static bool swd_read(uint8_t req, uint32_t* out_val) {
  for (int wait_try = 0; wait_try < 8; wait_try++) {
    uint8_t ack = send_header(req);
    if (ack == 1) {
      uint32_t data = 0;
      uint8_t calc_parity = 0;
      for (int i = 0; i < 32; i++) {
        uint8_t b = read_bit();
        if (b) {
          data |= (1UL << i);
          calc_parity ^= 1;
        }
      }
      uint8_t rx_parity = read_bit();

      swdio_as_output();
      clk_pulse();
      for (int i = 0; i < 8; i++) {
        write_bit(0);
      }

      if (rx_parity != calc_parity) {
        return false;
      }
      *out_val = data;
      return true;
    }

    swdio_as_output();
    clk_pulse();
    for (int i = 0; i < 8; i++) {
      write_bit(0);
    }
    if (ack == 2) {
      delayMicroseconds(20);
      continue;
    }
    return false;
  }
  return false;
}

static bool swd_write(uint8_t req, uint32_t val) {
  for (int wait_try = 0; wait_try < 8; wait_try++) {
    uint8_t ack = send_header(req);
    if (ack == 1) {
      swdio_as_output();
      clk_pulse();

      uint8_t parity = 0;
      for (int i = 0; i < 32; i++) {
        uint8_t b = (val >> i) & 1;
        write_bit(b);
        parity ^= b;
      }
      write_bit(parity);

      for (int i = 0; i < 8; i++) {
        write_bit(0);
      }
      return true;
    }

    swdio_as_output();
    clk_pulse();
    for (int i = 0; i < 8; i++) {
      write_bit(0);
    }
    if (ack == 2) {
      delayMicroseconds(20);
      continue;
    }
    return false;
  }
  return false;
}

static bool swd_connect_and_enable_ahb() {
  swd_init_sequence();
  uint32_t idcode = 0;
  if (!swd_read(SWD_DP_RD_IDCODE, &idcode) || idcode == 0 || idcode == 0xFFFFFFFFUL) {
    return false;
  }
  swd_write(SWD_DP_WR_ABORT, 0x0000001EUL);
  swd_write(SWD_DP_WR_CTRLSTAT, 0x50000000UL);
  delay(5);
  swd_write(SWD_DP_WR_SELECT, 0x00000000UL);
  swd_write(SWD_AP_WR_CSW, CSW_32BIT_NOINC);
  return true;
}

static bool mem_read32_safe(uint32_t addr, uint32_t* out_val) {
  for (int attempt = 0; attempt < 4; attempt++) {
    if (swd_write(SWD_AP_WR_CSW, CSW_32BIT_NOINC) &&
        swd_write(SWD_AP_WR_TAR, addr)) {
      uint32_t dummy = 0;
      if (swd_read(SWD_AP_RD_DRW, &dummy) &&
          swd_read(SWD_DP_RD_RDBUFF, out_val)) {
        return true;
      }
    }
    swd_connect_and_enable_ahb();
  }
  *out_val = 0xFFFFFFFFUL;
  return false;
}

static bool mem_read_1kb(uint32_t aligned_1kb_addr, uint32_t* words256) {
  if (g_fast_block_ok) {
    bool ok = swd_write(SWD_AP_WR_CSW, CSW_32BIT_AUTOINC) &&
              swd_write(SWD_AP_WR_TAR, aligned_1kb_addr);
    uint32_t dummy = 0;
    if (ok) ok = swd_read(SWD_AP_RD_DRW, &dummy);
    for (int i = 0; i < 255 && ok; i++) {
      ok = swd_read(SWD_AP_RD_DRW, &words256[i]);
    }
    if (ok) ok = swd_read(SWD_DP_RD_RDBUFF, &words256[255]);
    if (ok) {
      swd_write(SWD_AP_WR_CSW, CSW_32BIT_NOINC);
      return true;
    }
    swd_connect_and_enable_ahb();
  }

  for (int i = 0; i < 256; i++) {
    if (!mem_read32_safe(aligned_1kb_addr + (uint32_t)(i * 4), &words256[i])) {
      return false;
    }
  }
  return true;
}

static void print_hex8_lower(uint8_t v) {
  const char hex[] = "0123456789abcdef";
  Serial.print(hex[(v >> 4) & 0xF]);
  Serial.print(hex[v & 0xF]);
}

static void print_hex8_upper(uint8_t v) {
  const char hex[] = "0123456789ABCDEF";
  Serial.print(hex[(v >> 4) & 0xF]);
  Serial.print(hex[v & 0xF]);
}

static void print_hex32(uint32_t v) {
  const char hex[] = "0123456789ABCDEF";
  for (int shift = 28; shift >= 0; shift -= 4) {
    Serial.print(hex[(v >> shift) & 0xF]);
  }
}

static void dump_region_hex_lines(const char* tag, uint32_t start_addr, uint32_t byte_len) {
  static uint32_t buf[256];
  Serial.print("=== BEGIN_HEX ");
  Serial.print(tag);
  Serial.print(" 0x");
  print_hex32(start_addr);
  Serial.print(" 0x");
  print_hex32(byte_len);
  Serial.println(" ===");

  for (uint32_t offset = 0; offset < byte_len; offset += 1024) {
    uint32_t addr = start_addr + offset;
    if (!mem_read_1kb(addr, buf)) {
      Serial.print("!! READ_ERROR at 0x");
      print_hex32(addr);
      Serial.println();
      break;
    }
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(buf);
    uint32_t chunk = (byte_len - offset < 1024) ? (byte_len - offset) : 1024;
    for (uint32_t line = 0; line < chunk; line += 32) {
      print_hex32(addr + line);
      Serial.print(':');
      for (uint32_t b = 0; b < 32 && (line + b) < chunk; b++) {
        print_hex8_upper(bytes[line + b]);
      }
      Serial.println();
    }
    yield();
  }

  Serial.print("=== END_HEX ");
  Serial.print(tag);
  Serial.println(" ===");
}

static bool read_donor_mac_over_swd() {
  static uint32_t kb_buf[256];

  Serial.println("\n[*] Waiting for SWD connection to donor Go Plus+ (nRF52832)...");
  swd_init_sequence();

  uint32_t idcode = 0;
  if (!swd_read(SWD_DP_RD_IDCODE, &idcode) || idcode == 0 || idcode == 0xFFFFFFFFUL) {
    Serial.println("[-] No SWD response. Touch probes (SWDCLK->Pin25, SWDIO->Pin26, VDD->Pin48, GND)...");
    return false;
  }

  swd_connect_and_enable_ahb();
  uint32_t part = 0, variant = 0;
  if (!mem_read32_safe(0x10000100UL, &part) || part == 0 || part == 0xFFFFFFFFUL) {
    Serial.println("[-] Target AHB-AP read failed. Check probe contact (or APPROTECT status)...");
    return false;
  }
  mem_read32_safe(0x10000104UL, &variant);

  // Verify fast 1 KB pipelined block mode for optional hex dumps
  uint32_t ficr_addr0 = 0, ficr_addr1 = 0, ficr_type = 0;
  mem_read32_safe(0x100000A0UL, &ficr_type);
  mem_read32_safe(0x100000A4UL, &ficr_addr0);
  mem_read32_safe(0x100000A8UL, &ficr_addr1);

  g_fast_block_ok = true;
  if (!mem_read_1kb(0x10000000UL, kb_buf) ||
      kb_buf[0x100 / 4] != part ||
      kb_buf[0x0A4 / 4] != ficr_addr0) {
    g_fast_block_ok = false;
  }

  // Read UICR->CUSTOMER[0..1] (0x10001080 .. 0x10001087) twice to guarantee zero bit-errors
  uint32_t cust0_a = 0, cust1_a = 0, cust0_b = 0, cust1_b = 0;
  if (!mem_read32_safe(0x10001080UL, &cust0_a) ||
      !mem_read32_safe(0x10001084UL, &cust1_a) ||
      !mem_read32_safe(0x10001080UL, &cust0_b) ||
      !mem_read32_safe(0x10001084UL, &cust1_b) ||
      cust0_a != cust0_b || cust1_a != cust1_b) {
    Serial.println("[-] UICR read verification mismatch (probe bounce). Retrying...");
    return false;
  }

  Serial.println("\n================================================================================");
  Serial.print("[+] Connected! Target Part: 0x");
  print_hex32(part);
  Serial.print("  Variant: 0x");
  print_hex32(variant);
  Serial.print("  IDCODE: 0x");
  print_hex32(idcode);
  Serial.println();
  Serial.println("================================================================================");

  Serial.print("  Nordic FICR.DEVICEADDR (0x100000A4..A8): 0x");
  print_hex32(ficr_addr0);
  Serial.print(" 0x");
  print_hex32(ficr_addr1);
  Serial.println("  [DO NOT USE - silicon Random Static]");

  Serial.print("  UICR.CUSTOMER[0]       (0x10001080):     0x");
  print_hex32(cust0_a);
  Serial.println();
  Serial.print("  UICR.CUSTOMER[1]       (0x10001084):     0x");
  print_hex32(cust1_a);
  Serial.println();

  if (cust0_a == 0xFFFFFFFFUL && cust1_a == 0xFFFFFFFFUL) {
    Serial.println("\n[!] WARNING: UICR.CUSTOMER[0..1] is erased (0xFFFFFFFF)!");
    Serial.println("    Was a mass-erase / --recover run on this chip previously?");
    return true;
  }

  uint8_t mac_le[6];
  mac_le[0] = (uint8_t)((cust0_a >> 0) & 0xFF);
  mac_le[1] = (uint8_t)((cust0_a >> 8) & 0xFF);
  mac_le[2] = (uint8_t)((cust0_a >> 16) & 0xFF);
  mac_le[3] = (uint8_t)((cust0_a >> 24) & 0xFF);
  mac_le[4] = (uint8_t)((cust1_a >> 0) & 0xFF);
  mac_le[5] = (uint8_t)((cust1_a >> 8) & 0xFF);

  Serial.println("\n********************************************************************************");
  Serial.println("SUCCESS! GENUINE DONOR PUBLIC BLE MAC RECOVERED FROM UICR (0x10001080):");
  Serial.println("********************************************************************************");
  Serial.print("Public BLE Address: ");
  for (int i = 5; i >= 0; i--) {
    print_hex8_upper(mac_le[i]);
    if (i > 0) Serial.print(':');
  }
  Serial.println("  (Address Type: Public)");

  Serial.println("\nPaste this into code/nrf_emulator/pgpemu_nrf52_plusplus_exp/pp_device_material.h:");
  Serial.println("--------------------------------------------------------------------------------");
  Serial.print("#define PP_DEVICE_PUBLIC_ADDR_LE_BYTES \\\n  { ");
  for (int i = 0; i < 6; i++) {
    Serial.print("0x");
    print_hex8_lower(mac_le[i]);
    if (i < 5) Serial.print(", ");
  }
  Serial.println(" }");
  Serial.println("--------------------------------------------------------------------------------");

  Serial.println("\nSafe to release probes! Optional Serial commands:");
  Serial.println("  r -> Re-read donor MAC");
  Serial.println("  u -> Hex-dump full FICR (4 KB) + UICR (4 KB)");
  Serial.println("  s -> Hex-dump full Target SRAM (64 KB)");
  Serial.println("  f -> Hex-dump full Internal Flash (512 KB)");
  return true;
}

void setup() {
  // Enable external 3.3V VCC pin on Teyleten SuperMini / Nice!Nano boards
  nrf_gpio_cfg_output(PIN_VCC_GATE);
  nrf_gpio_pin_set(PIN_VCC_GATE);

  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    delay(10);
  }

  nrf_gpio_cfg_output(PIN_SWDCLK);
  nrf_gpio_pin_set(PIN_SWDCLK);
  swdio_as_input();

  delay(200);
  Serial.println("\n================================================================================");
  Serial.println("Pokemon GO Plus + Non-Destructive SWD Public MAC Reader");
  Serial.println("================================================================================");
}

void loop() {
  if (!g_initial_read_done) {
    if (read_donor_mac_over_swd()) {
      g_initial_read_done = true;
    } else {
      delay(1500);
    }
    return;
  }

  if (Serial.available()) {
    char cmd = (char)Serial.read();
    if (cmd == 'r' || cmd == 'R') {
      read_donor_mac_over_swd();
    } else if (cmd == 'u' || cmd == 'U') {
      if (swd_connect_and_enable_ahb()) {
        dump_region_hex_lines("FICR", 0x10000000UL, 0x1000UL);
        dump_region_hex_lines("UICR", 0x10001000UL, 0x1000UL);
      } else {
        Serial.println("[-] SWD not connected.");
      }
    } else if (cmd == 's' || cmd == 'S') {
      if (swd_connect_and_enable_ahb()) {
        dump_region_hex_lines("SRAM", 0x20000000UL, 0x10000UL);
      } else {
        Serial.println("[-] SWD not connected.");
      }
    } else if (cmd == 'f' || cmd == 'F') {
      if (swd_connect_and_enable_ahb()) {
        dump_region_hex_lines("FLASH", 0x00000000UL, 0x80000UL);
      } else {
        Serial.println("[-] SWD not connected.");
      }
    }
  }
  delay(20);
}
