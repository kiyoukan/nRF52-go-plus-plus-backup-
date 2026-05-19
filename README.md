# Pokemon GO Plus + nRF52 Emulator

Experimental nRF52 firmware for emulating a Pokemon GO Plus + BLE peripheral.

This repository intentionally does not include private device material. To build
the firmware, provide your own `pp_device_material.h` from a device you own.

## What Is Included

- nRF52 Arduino firmware under `code/nrf_emulator/pgpemu_nrf52_plusplus_exp`
- a flash image reader under `tools/flash_image_reader`
- maintainer and protocol notes under `documentation`
- `pp_device_material.template.h` as the public device-material template

## What Is Not Included

- private BLE address / MAC
- cert block or selector/key material from any private device
- raw private flash images
- private app files or unrelated workspaces
- generated Arduino build output

## Device Material Setup

### Reading The External Flash

You need an external flash image from your own Plus+ device.

One working hardware setup is:

- CH341A 24/25-series EEPROM flash USB programmer
- 1.8V adapter for the CH341A
- SOP8/WSON8/QFN8/DFN8 spring-needle probe cable for WSON8 6x5 mm / 6.5 mm chips

The 1.8V adapter is important. Do not read the chip at 3.3V or 5V; higher
voltage can damage the flash chip.

Example probe cable:
https://www.aliexpress.us/item/3256811423345227.html

After reading the chip, save the external flash image file locally. Do not
commit that file to the repository.

### Generating Device Material

1. Put exactly one external flash image file in:

```text
tools/flash_image_reader/input
```

2. Run the reader:

```powershell
cd tools\flash_image_reader
python read_flash_data.py
```

3. Open `pp_device_material.generated.h`.
4. Fill `PP_DEVICE_PUBLIC_ADDR_LE_BYTES` with your device's public BLE address
   in little-endian byte order.
5. Change `PP_DEVICE_MATERIAL_CONFIGURED` to `1`.
6. Place the completed file at:

```text
code/nrf_emulator/pgpemu_nrf52_plusplus_exp/pp_device_material.h
```

The BLE address is not stored in the external flash image. You can read it from
a working device with a normal Bluetooth scanner app or phone Bluetooth device
info.

## Build

Install `arduino-cli`, the Adafruit nRF52 board package, and required Arduino
libraries, then run:

```powershell
arduino-cli compile --fqbn adafruit:nrf52:feather52840 --export-binaries "code\nrf_emulator\pgpemu_nrf52_plusplus_exp"
```

## Release Safety

Before publishing your own fork, make sure these are not committed:

- `pp_device_material.h`
- `tools/flash_image_reader/input/`
- `tools/flash_image_reader/device_material_info.txt`
- `tools/flash_image_reader/pp_device_material.generated.h`
- `code/nrf_emulator/pgpemu_nrf52_plusplus_exp/build/`

## Status

Current firmware behavior is documented in:

- `documentation/firmware_module_reference.md`
- `documentation/firmware_protocol_map.md`
- `documentation/plusplus_backup_reference.md`
