#include "pp_first_pair_flow.h"

bool ppFirstPairFlowHandleAuthWrite(const FirstPairRuntimeRefs& refs,
                                    const uint8_t* data,
                                    uint16_t len)
{
  return ppHandleFirstPairAuthWrite(refs, data, len);
}

void ppFirstPairFlowStageCandidatePair(const FirstPairRuntimeRefs& refs, const char* reason)
{
  ppStageCandidatePair(refs, reason);
}

void ppFirstPairFlowCommitStagedPair(const FirstPairRuntimeRefs& refs, const char* reason)
{
  ppCommitStagedPair(refs, reason);
}

void ppFirstPairFlowMarkAuthComplete(const FirstPairRuntimeRefs& refs)
{
  ppMarkAuthComplete(refs);
}

void ppFirstPairFlowScheduleFreshAuthSequence(const FirstPairRuntimeRefs& refs,
                                              uint8_t firstStep,
                                              const char* reason)
{
  ppScheduleFreshAuthSequence(refs, firstStep, reason);
}

void ppFirstPairFlowProcessFreshAuthSequence(const FirstPairRuntimeRefs& refs)
{
  ppProcessFreshAuthSequence(refs);
}

void ppFirstPairFlowScheduleServiceCBurst(const FirstPairRuntimeRefs& refs)
{
  ppScheduleServiceCFirstPairBurst(refs);
}

void ppFirstPairFlowProcessServiceCBurst(const FirstPairRuntimeRefs& refs)
{
  ppProcessServiceCFirstPairBurst(refs);
}

void ppFirstPairFlowBeginOlderServiceD(const FirstPairOlderServiceDRefs& refs,
                                       const uint8_t* status0)
{
  ppBeginFirstPairOlderServiceDFlow(refs, status0);
}

bool ppFirstPairFlowHandleOlderServiceDWrite(const FirstPairOlderServiceDRefs& refs,
                                             const uint8_t* data,
                                             uint16_t len,
                                             const uint8_t* status1,
                                             const uint8_t* status2,
                                             const uint8_t* status4,
                                             uint32_t waitForWriteMs)
{
  return ppHandleFirstPairOlderServiceDWrite(refs,
                                             data,
                                             len,
                                             status1,
                                             status2,
                                             status4,
                                             waitForWriteMs);
}
