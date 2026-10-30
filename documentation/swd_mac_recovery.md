# Recovering the Public BLE MAC via SWD (Non-Booting Donor Device)

When cloning a Pokemon GO Plus + onto an `nRF52840` board, the donor device's 6-byte **Public BLE MAC address** is required alongside the `FBCT` (`0x2000`) and `.SAV` (`0x3000`) regions from the 8 MB external SPI flash.

If your donor Pokemon GO Plus + still powers on, you can simply read its MAC address with any Bluetooth scanner app. However, if the donor unit is **dead or non-booting** (for example due to water damage, a failed PMIC/charger, or torn battery/button pads) and was never previously paired to a phone where you have Bluetooth logs, the MAC address cannot be read from the external SPI flash chip.

Instead, you can recover the genuine Nintendo Public BLE MAC directly from the donor's main **`nRF52832` QFN48** wireless SoC over **SWD (Serial Wire Debug)**—even using the same replacement `nRF52840` board you plan to run the emulator on, with no dedicated J-Link or ST-Link required.

---

## Critical Pitfall #1: Do NOT Use `NRF_FICR->DEVICEADDR`

Many generic Nordic `nRF52` "MAC address reader" scripts read `NRF_FICR->DEVICEADDR[0..1]` (`0x100000A4`–`0x100000A8`). **That address is wrong for Pokemon GO Plus + and will fail authentication.**

Why `FICR.DEVICEADDR` fails:

1. **Generic Nordic wafer-test address:** Every `nRF52832` leaves Nordic Semiconductor's factory with a random 48-bit value burned into `FICR.DEVICEADDR[0..1]` and `FICR.DEVICEADDRTYPE` (`0x100000A0`) set to `0xFFFFFFFF` (`Random Static`). *(Note: some generic scripts mistakenly inspect `0x100000AC` for the address type, which is an unallocated register that also reads `0xFFFFFFFF`.)*
2. **Nintendo uses an IEEE Public MAC bound to `FBCT`:** Retail Pokemon GO Plus + firmware ignores `FICR.DEVICEADDR` and advertises with `BLE_GAP_ADDR_TYPE_PUBLIC` using a registered **Nintendo Co., Ltd. IEEE OUI** (for example `28:CF:51:...`).
3. **Cryptographic binding during pairing:** During the Pokemon GO handshake, the emulator encrypts `PP_DEVICE_PUBLIC_ADDR_LE_BYTES` inside the certification challenge payload that the app verifies against `FBCT` selector 2 (`0x2006` in external flash). If you use `FICR.DEVICEADDR`, your board will appear in Bluetooth scan lists as `Pokemon GO Plus +`, **but pairing/certification in Pokemon GO will fail every time.**

---

## Critical Pitfall #2: NEVER Mass-Erase (`--recover`) the Donor `nRF52832`

- Retail Pokemon GO Plus + `nRF52832` chips (`INFO.PART = 0x00052832`) **do not** have SWD `AHB-AP` read protection (`APPROTECT`) enabled. You can read `FICR`, `UICR`, `SRAM`, and internal Flash directly and non-destructively.
- **Never** run `nrfjprog --recover`, `ERASEALL`, or any unlock/mass-erase command on a donor `nRF52832`. Mass-erasing wipes `UICR` back to `0xFFFFFFFF`, **permanently destroying** the donor's Public BLE MAC address on a non-booting board.

---

## Where the Genuine Public BLE MAC Lives (`UICR.CUSTOMER[0..1]`)

Nintendo programs each unit's 6-byte IEEE Public BLE MAC address into the `nRF52832` **User Information Configuration Registers (`NRF_UICR->CUSTOMER[0..1]`)** at **`0x10001080`–`0x10001087`** in little-endian byte order:

| Register | Address | Example 32-Bit Word | Raw Little-Endian Bytes | Meaning |
|---|---|---|---|---|
| `NRF_UICR->CUSTOMER[0]` | `0x10001080` | `0x51332211` | `11 22 33 51` | `mac_le[0..3]` (lower 4 bytes of MAC; byte `3` is the 3rd OUI byte) |
| `NRF_UICR->CUSTOMER[1]` | `0x10001084` | `0x000028CF` | `CF 28 00 00` | `mac_le[4..5]` in low 16 bits (1st & 2nd OUI bytes); upper 16 bits are `0x0000` |

### Converting `UICR.CUSTOMER[0..1]` to `pp_device_material.h`

Using the synthetic example values above (`0x51332211` at `0x10001080` and `0x000028CF` at `0x10001084`):

- **Standard BLE Scanner / Display Format (Big-Endian):**
  `28:CF:51:33:22:11`
  *(Sanity check: the first 3 bytes—`28:CF:51` in this example—should match a known **Nintendo Co., Ltd.** IEEE OUI in an OUI lookup table.)*
- **`pp_device_material.h` Macro Format (Little-Endian):**
  ```c
  #define PP_DEVICE_PUBLIC_ADDR_LE_BYTES \
    { 0x11, 0x22, 0x33, 0x51, 0xcf, 0x28 }
  ```

---

## Method A: Using Your Target `nRF52840` Board as a Temporary SWD Probe

If you do not own a dedicated SWD debugger, you can temporarily flash [`tools/swd_mac_reader/swd_mac_reader.ino`](../tools/swd_mac_reader/swd_mac_reader.ino) onto your replacement `nRF52840` board (such as a Pro Micro / SuperMini `nRF52840` or Adafruit Feather `nRF52840`), touch 4 wires/probes to the donor `nRF52832` for 1–2 seconds to read `UICR`, and then flash the main emulator firmware onto the same `nRF52840` board.

### 1. Prepare the Donor PCB

1. **Disconnect the donor battery** from the Pokemon GO Plus + PCB before probing so a damaged battery or shorted charging rail does not interfere with external `3.3V` power.
2. Locate the square **`N52832` QFN48** chip on the donor PCB (48 pins total, 12 pins per side).
3. Find the **Pin 1 corner dot** etched on the top of the `N52832` package. Pins count **counter-clockwise** around the chip:
   - Side 1: Pins `1`–`12`
   - Side 2: Pins `13`–`24`
   - Side 3: **Pin `25` (`SWDCLK`)** and **Pin `26` (`SWDIO`)** are the very first two pins starting Side 3 (diagonally opposite the Pin 1 corner).
   - Side 4: **Pin `48` (`VDD`)** is the very last pin on Side 4, immediately adjacent to the Pin 1 corner.

### 2. 4-Wire SWD Hookup

| Target `nRF52840` Board Pin | Donor Go Plus+ (`nRF52832` QFN48) | Notes |
|---|---|---|
| `GND` | USB-C outer metal shell (or any PCB ground pad) | Clip securely first for common ground |
| `VCC` (`3.3V`) | QFN48 **Pin 48** (`VDD`) | Powers the `nRF52832` core (`3.3V` is within Nordic's `1.7V`–`3.6V` VDD spec; **only** the external SPI flash is `1.8V`-only—do **not** apply `3.3V` to the 8-pin SPI flash chip!) |
| `P0.06` (silkscreen `006`) | QFN48 **Pin 25** (`SWDCLK`) | Fine probe tip / sewing needle on pin or trace |
| `P0.08` (silkscreen `008`) | QFN48 **Pin 26** (`SWDIO`) | Fine probe tip / sewing needle on pin or trace |

> **Note for SuperMini / Pro Micro `nRF52840` (Nice!Nano-style) boards:** `swd_mac_reader.ino` automatically drives `P0.13` HIGH at startup so the board's external `3.3V` `VCC` pin is enabled when powered over USB. If you use different GPIO pins for `SWDCLK`/`SWDIO`, edit `PIN_SWDCLK` and `PIN_SWDIO` at the top of [`swd_mac_reader.ino`](../tools/swd_mac_reader/swd_mac_reader.ino).

### 3. Flash `swd_mac_reader` and Read the MAC

1. Connect your `nRF52840` board over USB (with SWD probes not touching anything yet) and upload the reader sketch:

```bash
arduino-cli compile --fqbn adafruit:nrf52:feather52840 tools/swd_mac_reader
arduino-cli upload -p /dev/ttyACM0 --fqbn adafruit:nrf52:feather52840 tools/swd_mac_reader
```

*(On Windows, replace `/dev/ttyACM0` with your COM port, e.g. `COM5`. If the board does not enter bootloader mode automatically, double-tap `RST` so its UF2/DFU bootloader port appears.)*

2. Open the serial monitor at `115200` baud:

```bash
arduino-cli monitor -p /dev/ttyACM0 -c baudrate=115200
```

3. With `GND` clipped to the donor USB-C shield and `3.3V` supplying QFN48 **Pin 48** (`VDD`), hold your two probes steadily on **Pin 25 (`SWDCLK`)** and **Pin 26 (`SWDIO`)** for 1–2 seconds.

4. Once contact is made, the sketch verifies the target (`Part: 0x00052832`), reads `UICR.CUSTOMER[0..1]` twice with SWD parity checking to rule out probe bounce, and prints output like:

```text
================================================================================
[+] Connected! Target Part: 0x00052832  Variant: 0x41414230  IDCODE: 0x2BA01477
================================================================================
  Nordic FICR.DEVICEADDR (0x100000A4..A8): 0x99887766 0x0000AABB  [DO NOT USE - silicon Random Static]
  UICR.CUSTOMER[0]       (0x10001080):     0x51332211
  UICR.CUSTOMER[1]       (0x10001084):     0x000028CF

********************************************************************************
SUCCESS! GENUINE DONOR PUBLIC BLE MAC RECOVERED FROM UICR (0x10001080):
********************************************************************************
Public BLE Address: 28:CF:51:33:22:11  (Address Type: Public)

Paste this into code/nrf_emulator/pgpemu_nrf52_plusplus_exp/pp_device_material.h:
--------------------------------------------------------------------------------
#define PP_DEVICE_PUBLIC_ADDR_LE_BYTES \
  { 0x11, 0x22, 0x33, 0x51, 0xcf, 0x28 }
--------------------------------------------------------------------------------
```

5. Copy the `#define PP_DEVICE_PUBLIC_ADDR_LE_BYTES` block into `code/nrf_emulator/pgpemu_nrf52_plusplus_exp/pp_device_material.h`, remove all SWD wires from your `nRF52840` board, and compile + upload `code/nrf_emulator/pgpemu_nrf52_plusplus_exp` as normal.

---

## Method B: Using a Dedicated SWD Debugger (`J-Link`, `ST-Link`, `CMSIS-DAP`)

If you already have a standard SWD probe wired to `GND`, `VDD` (Pin 48), `SWDCLK` (Pin 25), and `SWDIO` (Pin 26), read the two 32-bit words at `0x10001080` and `0x10001084` **without** erasing the target:

### Nordic `nrfjprog` (J-Link)

```bash
nrfjprog --family NRF52 --memrd 0x10001080 --n 8
```

Example output:
```text
0x10001080: 51332211 000028CF                    |."3Q.(..|
```

### `pyOCD` (CMSIS-DAP / Picoprobe / ST-Link)

```bash
pyocd commander -t nrf52832 -c "read32 0x10001080 8"
```

### `OpenOCD`

```bash
openocd -f interface/cmsis-dap.cfg -f target/nrf52.cfg -c "init; mdw 0x10001080 2; exit"
```

From the two 32-bit words `0xB3B2B1B0` (`0x10001080`) and `0x0000B5B4` (`0x10001084`):
- Public BLE MAC: `B5:B4:B3:B2:B1:B0`
- `PP_DEVICE_PUBLIC_ADDR_LE_BYTES`: `{ 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5 }`

---

## Appendix: Verifying External SPI Flash Dumps When Using a Spring-Needle Probe

When dumping the 8 MB (`1.8V` WSON8) external SPI flash chip (`MX25U6435F` / `MX25U6433F`) by hand with a `CH341A` + **mandatory `1.8V` adapter** + spring-needle probe, slight hand movement during the ~60-second read commonly causes two consecutive dumps to have different SHA-256 hashes.

To check whether an external flash `.bin` dump is clean and safe to use:

1. **Exact file size:** Must be `8,388,608` bytes (`0x800000`).
2. **`FBCT` factory auth header (`0x2000`):**
   - Bytes `0x2000`–`0x2003` must equal `46 42 43 54` (ASCII `FBCT`).
   - Bytes `0x2006`–`0x2115` (`selector 2` cert + `selector 3` key) must contain high-entropy non-`0xFF` data, and `0x21A2`–`0x2FFF` should be cleanly erased (`0xFF`).
3. **Persisted `.SAV` state (`0x3000`):**
   - Bytes `0x3000`–`0x3003` must equal `18 53 41 56` (`\x18SAV`).
   - Bytes `0x3288`–`0x3FFF` should be cleanly erased (`0xFF`).
4. **Two-read verification for cloning (`0x0000`–`0x3FFF`):**
   - Even if a probe slips halfway through the 8 MB audio partition, all donor material used by `tools/flash_image_reader/read_flash_data.py` lives inside the first **16 KB (`0x0000`–`0x3FFF`)**—taking less than 0.2 seconds to read at the very start of the dump.
   - If the first `16,384` bytes match byte-for-byte across two separate reads (`cmp -n 16384 dump1.bin dump2.bin` on Linux/macOS), your `FBCT` and `.SAV` extraction is guaranteed bit-perfect.
5. **Full-chip archival check (optional):**
   - If you want a 100% intact 8 MB archival backup (including Pikachu/sound effects), verify that all **89 `RIFF....WAVE` audio files** between `0x008AB6` and `0x6A9360` chain end-to-end (`offset + 8 + riff_size + 4 == next_riff_offset`) right up to the `IDXS` index blocks at `0x6A9360`. A mid-read probe slip will break `RIFF` alignment or leave sudden `0xFF` / bit-shifted runs after the slip offset.
