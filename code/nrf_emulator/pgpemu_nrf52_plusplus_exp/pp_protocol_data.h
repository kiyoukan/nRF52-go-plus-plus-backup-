#pragma once

#include <stdint.h>

#if __has_include("pp_device_material.h")
#include "pp_device_material.h"
#else
#error "Missing pp_device_material.h. Generate it with tools/flash_image_reader or copy pp_device_material.template.h and fill in your own device material."
#endif

extern const uint8_t PLUSPLUS_SERVICE_DATA_32[4];
extern const uint8_t PLUSPLUS_AUTH_CTRL_REPLY_2401[5];
extern const uint8_t PLUSPLUS_FRESH_AUTH_CTRL_REPLY_24[5];
extern const uint8_t PLUSPLUS_AUTH_CTRL_REPLY_25[2];
extern const uint8_t PLUSPLUS_AUTH_CTRL_REPLY_18[2];
extern const uint8_t PLUSPLUS_AUTH_CTRL_REPLY_18_FAIL[2];
extern const uint8_t PLUSPLUS_AUTH_CTRL_REPLY_16[6];
extern const uint8_t PLUSPLUS_AUTH_CTRL_REPLY_36[2];
extern const uint8_t PLUSPLUS_AUTH_CTRL_REPLY_28[2];
extern const uint8_t PLUSPLUS_AUTH_CTRL_REPLY_26[2];
extern const uint8_t PLUSPLUS_AUTH_CTRL_REPLY_21_WAIT[3];
extern const uint8_t PLUSPLUS_AUTH_CTRL_REPLY_21_READY[3];
extern const uint8_t PLUSPLUS_AUTH_BLOB_SHORT[8];
extern const uint8_t PLUSPLUS_SERVICE_D_STATUS0[4];
extern const uint8_t PLUSPLUS_SERVICE_D_STATUS1[4];
extern const uint8_t PLUSPLUS_SERVICE_D_STATUS2[4];
extern const uint8_t PLUSPLUS_SERVICE_D_STATUS4[4];
extern const uint8_t PLUSPLUS_SERVICE_D_STATUS3_START[4];
extern const uint8_t PLUSPLUS_SERVICE_D_STATUS4_STAGE1[4];
extern const uint8_t PLUSPLUS_SERVICE_D_STATUS5_STAGE2[4];
extern const uint8_t PLUSPLUS_SERVICE_D_STATUS4_DONE[4];
extern const uint8_t PLUSPLUS_SERVICE_D_STATUS4_FAIL[4];
extern const uint8_t PLUSPLUS_OLDER_TYPE1_CONST[16];
extern const uint8_t DONOR_SERVICE_D_READ_TYPE3[36];
extern const uint8_t DONOR_SERVICE_D_EXPECT_WRITE3_DONE[5];
extern const uint8_t FIRMWARE_RECORD4_DEFAULT[16];
