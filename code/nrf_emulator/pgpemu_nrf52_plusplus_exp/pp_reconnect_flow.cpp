#include "pp_reconnect_flow.h"

void ppReconnectFlowRefreshAuthReply24(const ReconnectRuntimeRefs& refs, const char* reason)
{
  ppRefreshReconnectAuthReply24(refs, reason);
}

void ppReconnectFlowHandleAuthWrite24(const ReconnectRuntimeRefs& refs,
                                      uint8_t selectorIndex,
                                      const char* reason)
{
  ppHandleReconnectAuthWrite24(refs, selectorIndex, reason);
}

void ppReconnectFlowHandleAuthWrite25(const ReconnectRuntimeRefs& refs,
                                      const uint8_t* authReply25,
                                      size_t authReply25Len,
                                      const char* blobReason)
{
  ppHandleReconnectAuthWrite25(refs, authReply25, authReply25Len, blobReason);
}

void ppReconnectFlowBeginCompactServiceD(const ReconnectCompactServiceDRefs& refs,
                                         const uint8_t* statusStart,
                                         uint32_t waitForWriteMs)
{
  ppBeginReconnectCompactServiceDFlow(refs, statusStart, waitForWriteMs);
}

bool ppReconnectFlowHandleCompactServiceDWrite(const ReconnectCompactServiceDRefs& refs,
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
  return ppHandleReconnectCompactServiceDWrite(refs,
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

bool ppReconnectFlowProcessCompactCompletion(const ReconnectCompactServiceDRefs& refs)
{
  return ppProcessReconnectCompactCompletion(refs);
}
