# SWD Donor MAC Reader

Non-destructive SWD helper sketch for reading the genuine Nintendo **Public BLE MAC address** directly from a Pokemon GO Plus + (`nRF52832` QFN48) when the donor device cannot boot or advertise over Bluetooth.

For full hardware details, QFN48 pin locations, register layout, and `nrfjprog` / `OpenOCD` alternatives, see [`documentation/swd_mac_recovery.md`](../../documentation/swd_mac_recovery.md).

## Why This Tool Is Needed

- The donor's Public BLE MAC address is **not** stored in the 8 MB external SPI flash image.
- Generic nRF52 "MAC reader" scripts read `NRF_FICR->DEVICEADDR` (`0x100000A4`–`0x100000A8`), which only holds Nordic Semiconductor's factory `Random Static` silicon address—**not** the Nintendo IEEE Public MAC bound to your `FBCT` certificate. Using `FICR.DEVICEADDR` will fail Pokemon GO authentication.
- On the Pokemon GO Plus +, the real donor Public BLE MAC lives in **`NRF_UICR->CUSTOMER[0..1]` (`0x10001080`–`0x10001087`)** in little-endian byte order, and SWD memory-read access (`AHB-AP`) is unlocked on retail hardware.

> **Warning:** Do **not** run a chip mass-erase / `--recover` on your donor `nRF52832`. Mass-erasing wipes `UICR` and permanently destroys `CUSTOMER[0..1]`. This sketch is strictly read-only.

## Wiring (Target nRF52840 Board as Temporary SWD Probe)

You can temporarily flash this sketch onto the same `nRF52840` board (e.g., Teyleten Robot Pro Micro / SuperMini `nRF52840` or Adafruit Feather `nRF52840`) that will later run the emulator firmware:

| nRF52840 Board Pin | Donor Go Plus+ (`nRF52832` QFN48) | Purpose |
|---|---|---|
| `GND` | USB-C outer metal shield (ground) | Common ground |
| `VCC` (`3.3V`) | QFN48 **Pin 48** (`VDD`) | Powers donor `nRF52832` during SWD read |
| `P0.06` (silkscreen `006`) | QFN48 **Pin 25** (`SWDCLK`) | SWD clock |
| `P0.08` (silkscreen `008`) | QFN48 **Pin 26** (`SWDIO`) | SWD data |

*(If using different GPIOs on another `nRF52840` board, adjust `PIN_SWDCLK` and `PIN_SWDIO` at the top of `swd_mac_reader.ino`.)*

## Usage

1. Compile and upload `swd_mac_reader.ino` to your `nRF52840` board:

```bash
arduino-cli compile --fqbn adafruit:nrf52:feather52840 tools/swd_mac_reader
arduino-cli upload -p /dev/ttyACM0 --fqbn adafruit:nrf52:feather52840 tools/swd_mac_reader
```

2. Open Serial Monitor at `115200` baud:

```bash
arduino-cli monitor -p /dev/ttyACM0 -c baudrate=115200
```

3. Touch your SWD probes to the donor `nRF52832` for ~1–2 seconds. The sketch reads and double-verifies `UICR.CUSTOMER[0..1]` (`0x10001080`–`0x10001087`) and prints your ready-to-paste macro:

```c
#define PP_DEVICE_PUBLIC_ADDR_LE_BYTES \
  { 0x11, 0x22, 0x33, 0x51, 0xcf, 0x28 }
```

4. Paste that value into `code/nrf_emulator/pgpemu_nrf52_plusplus_exp/pp_device_material.h`, disconnect the SWD wires, and flash the main emulator firmware onto your `nRF52840` board.
