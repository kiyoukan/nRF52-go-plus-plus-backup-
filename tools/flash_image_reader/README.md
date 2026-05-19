# Flash Image Reader

This helper is meant for very simple use:

1. Put exactly one external flash image file into the `input` folder.
2. Run `read_flash_data.py`.
3. Open `device_material_info.txt`.
4. Open `pp_device_material.generated.h`, fill in the BLE address, and use it
   as the firmware's `pp_device_material.h`.

## Windows

If Python is installed, you can usually run:

```powershell
py read_flash_data.py
```

or:

```powershell
python read_flash_data.py
```

## Linux

Run:

```bash
python3 read_flash_data.py
```

## What it reads

- selector 2 / cert block
- selector 3 / key lane
- factory selector 6 used by the firmware fallback seed
- full persisted `.SAV` image
- persisted selectors 4, 5, and 6
- quick magic checks for `FBCT` and `.SAV`

It also reminds the user that the donor public BLE address is not stored in the external flash image.
The BLE address can be read from a working device with a normal Bluetooth
scanner app or basic phone Bluetooth device info. Enter it in little-endian
order in the generated header; the generated file includes an example.
