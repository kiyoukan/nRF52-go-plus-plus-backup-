#pragma once

#include <stdint.h>

// Copy this file to pp_device_material.h or use the flash_image_reader
// generated header. Fill every value from your own Plus+ device.
#define PP_DEVICE_MATERIAL_CONFIGURED 0

// Enter the public BLE address in little-endian byte order.
// Example: AA:BB:CC:DD:EE:FF -> { 0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa }
#define PP_DEVICE_PUBLIC_ADDR_LE_BYTES \
  FILL_WITH_YOUR_DEVICE_PUBLIC_BLE_ADDRESS_LE

#define PP_DEVICE_CERT_BLOCK_BYTES \
  FILL_WITH_SELECTOR_2_CERT_BLOCK_FROM_FLASH_DUMP

#define PP_DEVICE_SELECTOR3_BYTES \
  FILL_WITH_SELECTOR_3_KEY_LANE_FROM_FLASH_DUMP

// Current protocol model value. The extractor-generated header fills this.
#define PP_DEVICE_AUTH_SELECTOR4_BYTES \
  FILL_WITH_CURRENT_AUTH_SELECTOR4_MODEL_VALUE

#define PP_DEVICE_SELECTOR6_BYTES \
  FILL_WITH_SELECTOR_6_FROM_FLASH_DUMP

extern const uint8_t REAL_PLUSPLUS_ADDR_LE[6];
extern const uint8_t PLUSPLUS_FACTORY_CERT_BLOCK[256];
extern const uint8_t PLUSPLUS_FACTORY_SELECTOR3[16];
extern const uint8_t PLUSPLUS_FACTORY_SELECTOR4[16];
extern const uint8_t PLUSPLUS_FACTORY_SELECTOR6[10];
