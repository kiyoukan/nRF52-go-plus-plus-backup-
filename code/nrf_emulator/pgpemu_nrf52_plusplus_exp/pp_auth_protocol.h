#pragma once

#include <stddef.h>
#include <stdint.h>

#include "pp_pair_first_time.h"
#include "pp_pair_reconnect.h"

bool ppAuthProtocolIsAuthenticatedSleepOpcode(uint8_t opcode);
bool ppAuthProtocolIsIndexed24Write(const uint8_t* data, uint16_t len);
bool ppAuthProtocolIsAuth25Write(const uint8_t* data, uint16_t len);
bool ppAuthProtocolIsFirstPairControlWrite(const uint8_t* data, uint16_t len);

bool ppAuthProtocolHandleFirstPairWrite(const FirstPairRuntimeRefs& refs,
                                        const uint8_t* data,
                                        uint16_t len);
void ppAuthProtocolRefreshReconnectReply24(const ReconnectRuntimeRefs& refs,
                                           const char* reason);
void ppAuthProtocolHandleReconnectWrite24(const ReconnectRuntimeRefs& refs,
                                          uint8_t selectorIndex,
                                          const char* reason);
void ppAuthProtocolHandleReconnectWrite25(const ReconnectRuntimeRefs& refs,
                                          const uint8_t* authReply25,
                                          size_t authReply25Len,
                                          const char* blobReason);
