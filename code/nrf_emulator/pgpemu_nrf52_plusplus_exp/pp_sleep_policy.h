#pragma once

#include <stdint.h>

void ppSleepPolicyLoadIfNeeded();
void ppSleepPolicyResetVolatileState();
void ppSleepPolicyClearPersistedState();

bool ppSleepPolicyShouldGenerateForAnchor(uint32_t anchor);
void ppSleepPolicyArmInitialGeneration(const char* reason);
void ppSleepPolicyMarkGeneratedForAnchor(uint32_t anchor, const char* reason);
void ppSleepPolicyMarkAnchorUpdated(uint32_t anchor, const char* reason);
bool ppSleepPolicyHasGeneratedSession();
