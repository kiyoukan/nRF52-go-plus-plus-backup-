#include <Arduino.h>

#include "pp_auth_protocol.h"
#include "pp_pair_reconnect.h"
#include "pp_service_d_protocol.h"

void ppRefreshReconnectAuthReply24(const ReconnectRuntimeRefs& refs, const char* reason)
{
  ppAuthProtocolRefreshReconnectReply24(refs, reason);
}

void ppHandleReconnectAuthWrite24(const ReconnectRuntimeRefs& refs,
                                  uint8_t selectorIndex,
                                  const char* reason)
{
  ppAuthProtocolHandleReconnectWrite24(refs, selectorIndex, reason);
}

void ppHandleReconnectAuthWrite25(const ReconnectRuntimeRefs& refs,
                                  const uint8_t* authReply25,
                                  size_t authReply25Len,
                                  const char* blobReason)
{
  ppAuthProtocolHandleReconnectWrite25(refs, authReply25, authReply25Len, blobReason);
}

void ppBeginReconnectCompactServiceDFlow(const ReconnectCompactServiceDRefs& refs,
                                         const uint8_t* statusStart,
                                         uint32_t waitForWriteMs)
{
  ppServiceDProtocolBeginCompactLane(refs, statusStart, waitForWriteMs);
}

bool ppHandleReconnectCompactServiceDWrite(const ReconnectCompactServiceDRefs& refs,
                                           const uint8_t* data,
                                           uint16_t len,
                                           const uint8_t* expectedWrite3Done,
                                           size_t expectedWrite3DoneLen,
                                           const uint8_t* statusStage1,
                                           const uint8_t* statusStage2,
                                           const uint8_t* statusDone,
                                           const uint8_t* statusFail,
                                           uint32_t waitForWriteMs)
{
  return ppServiceDProtocolHandleCompactLaneWrite(refs,
                                                  data,
                                                  len,
                                                  expectedWrite3Done,
                                                  expectedWrite3DoneLen,
                                                  statusStage1,
                                                  statusStage2,
                                                  statusDone,
                                                  statusFail,
                                                  waitForWriteMs);
}

bool ppProcessReconnectCompactCompletion(const ReconnectCompactServiceDRefs& refs)
{
  return ppServiceDProtocolProcessCompactCompletion(refs);
}
