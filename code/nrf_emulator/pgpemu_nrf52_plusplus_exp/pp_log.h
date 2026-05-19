#pragma once

// Compile-time logging policy. Override PP_LOG_LEVEL before including this
// header or via the Arduino build flags when diagnosing noisy BLE behavior.
#define PP_LOG_LEVEL_DISABLED 0
#define PP_LOG_LEVEL_PROTOCOL 1
#define PP_LOG_LEVEL_VERBOSE 2
#define PP_LOG_LEVEL_RAW_BLE 3

#ifndef PP_LOG_LEVEL
#define PP_LOG_LEVEL PP_LOG_LEVEL_PROTOCOL
#endif

#define PP_LOG_PROTOCOL_ENABLED (PP_LOG_LEVEL >= PP_LOG_LEVEL_PROTOCOL)
#define PP_LOG_VERBOSE_ENABLED (PP_LOG_LEVEL >= PP_LOG_LEVEL_VERBOSE)
#define PP_LOG_RAW_BLE_ENABLED (PP_LOG_LEVEL >= PP_LOG_LEVEL_RAW_BLE)
