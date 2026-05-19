#pragma once

#include <stdint.h>

#include "pp_pair_first_time.h"

bool ppFirstPairFlowHandleAuthWrite(const FirstPairRuntimeRefs& refs,
                                    const uint8_t* data,
                                    uint16_t len);
void ppFirstPairFlowStageCandidatePair(const FirstPairRuntimeRefs& refs, const char* reason);
void ppFirstPairFlowCommitStagedPair(const FirstPairRuntimeRefs& refs, const char* reason);
void ppFirstPairFlowMarkAuthComplete(const FirstPairRuntimeRefs& refs);
void ppFirstPairFlowScheduleFreshAuthSequence(const FirstPairRuntimeRefs& refs,
                                              uint8_t firstStep,
                                              const char* reason);
void ppFirstPairFlowProcessFreshAuthSequence(const FirstPairRuntimeRefs& refs);
void ppFirstPairFlowScheduleServiceCBurst(const FirstPairRuntimeRefs& refs);
void ppFirstPairFlowProcessServiceCBurst(const FirstPairRuntimeRefs& refs);

void ppFirstPairFlowBeginOlderServiceD(const FirstPairOlderServiceDRefs& refs,
                                       const uint8_t* status0);
bool ppFirstPairFlowHandleOlderServiceDWrite(const FirstPairOlderServiceDRefs& refs,
                                             const uint8_t* data,
                                             uint16_t len,
                                             const uint8_t* status1,
                                             const uint8_t* status2,
                                             const uint8_t* status4,
                                             uint32_t waitForWriteMs);
