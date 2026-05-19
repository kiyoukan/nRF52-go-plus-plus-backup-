#pragma once

#include <stddef.h>
#include <stdint.h>

#include "pp_pair_reconnect.h"

void ppReconnectFlowRefreshAuthReply24(const ReconnectRuntimeRefs& refs, const char* reason);
void ppReconnectFlowHandleAuthWrite24(const ReconnectRuntimeRefs& refs,
                                      uint8_t selectorIndex,
                                      const char* reason);
void ppReconnectFlowHandleAuthWrite25(const ReconnectRuntimeRefs& refs,
                                      const uint8_t* authReply25,
                                      size_t authReply25Len,
                                      const char* blobReason);

void ppReconnectFlowBeginCompactServiceD(const ReconnectCompactServiceDRefs& refs,
                                         const uint8_t* statusStart,
                                         uint32_t waitForWriteMs);
bool ppReconnectFlowHandleCompactServiceDWrite(const ReconnectCompactServiceDRefs& refs,
                                               const uint8_t* data,
                                               uint16_t len,
                                               const uint8_t* expectedWrite3Done,
                                               size_t expectedWrite3DoneLen,
                                               const uint8_t* statusStage1,
                                               const uint8_t* statusStage2,
                                               const uint8_t* statusDone,
                                               const uint8_t* statusFail,
                                               uint32_t waitForWriteMs);
bool ppReconnectFlowProcessCompactCompletion(const ReconnectCompactServiceDRefs& refs);
