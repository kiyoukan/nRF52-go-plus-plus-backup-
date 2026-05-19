#pragma once

#include <stddef.h>
#include <stdint.h>

#include "pp_pair_first_time.h"
#include "pp_pair_reconnect.h"
#include "pp_types.h"

bool ppServiceDProtocolParseTypedWrite(const uint8_t* data,
                                       uint16_t len,
                                       uint8_t* typeOut);
bool ppServiceDProtocolValidateExpectedWrite(ServiceDCertState* state,
                                             uint8_t messageType,
                                             uint16_t len);

void ppServiceDProtocolBeginOlderLane(const FirstPairOlderServiceDRefs& refs,
                                      const uint8_t* status0);
bool ppServiceDProtocolHandleOlderLaneWrite(const FirstPairOlderServiceDRefs& refs,
                                            const uint8_t* data,
                                            uint16_t len,
                                            const uint8_t* status1,
                                            const uint8_t* status2,
                                            const uint8_t* status4,
                                            uint32_t waitForWriteMs);

void ppServiceDProtocolBeginCompactLane(const ReconnectCompactServiceDRefs& refs,
                                        const uint8_t* statusStart,
                                        uint32_t waitForWriteMs);
bool ppServiceDProtocolHandleCompactLaneWrite(const ReconnectCompactServiceDRefs& refs,
                                              const uint8_t* data,
                                              uint16_t len,
                                              const uint8_t* expectedWrite3Done,
                                              size_t expectedWrite3DoneLen,
                                              const uint8_t* statusStage1,
                                              const uint8_t* statusStage2,
                                              const uint8_t* statusDone,
                                              const uint8_t* statusFail,
                                              uint32_t waitForWriteMs);
bool ppServiceDProtocolProcessCompactCompletion(const ReconnectCompactServiceDRefs& refs);
