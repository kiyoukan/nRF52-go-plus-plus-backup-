#include <Arduino.h>
#include <string.h>

#include "pp_auth_blob.h"

void ppClearAuthBlobStage(const AuthBlobRuntimeRefs& refs)
{
  refs.authBlobStage->ready = false;
  refs.authBlobStage->stageClass = 0;
  refs.authBlobStage->totalLen = 0;
  refs.authBlobStage->offset = 0;
  refs.authBlobStage->streamFlags = 0;
  refs.authBlobStage->lastOpcode = 0;
  refs.authParserOutput->outputClass = 0;
  refs.authParserOutput->outputLen = 0;
  refs.authParserOutput->outputOffset = 0;
  refs.authParserOutput->streamPending = false;
  refs.authParserOutput->notifyInFlight = false;
}

void ppStageVisibleAuthBlob(const AuthBlobRuntimeRefs& refs,
                            const uint8_t* payload,
                            uint16_t payloadLen,
                            uint8_t stageClass,
                            uint8_t lastOpcode)
{
  uint16_t clampedLen = min<uint16_t>(payloadLen, static_cast<uint16_t>(refs.authBlobVisibleValueLen));
  uint8_t stagedCopy[AUTH_BLOB_VISIBLE_LEN];
  memset(stagedCopy, 0, sizeof(stagedCopy));
  memcpy(stagedCopy, payload, clampedLen);
  memset(refs.authBlobVisibleValue, 0, refs.authBlobVisibleValueLen);
  memcpy(refs.authBlobVisibleValue, stagedCopy, clampedLen);
  refs.authBlobStage->ready = true;
  refs.authBlobStage->stageClass = stageClass;
  refs.authBlobStage->totalLen = clampedLen;
  refs.authBlobStage->offset = 0;
  refs.authBlobStage->streamFlags = 0;
  refs.authBlobStage->lastOpcode = lastOpcode;
  refs.setAuthParserSchedulerState(1, stageClass, clampedLen, 0, 1, 2, lastOpcode, true, true, false);
  refs.authBlob->write(refs.authBlobVisibleValue, clampedLen);
  Serial.print("Auth blob stage ready class=");
  Serial.print(refs.authBlobStage->stageClass);
  Serial.print(" len=0x");
  Serial.print(refs.authBlobStage->totalLen, HEX);
  Serial.print(" offset=0x");
  Serial.println(refs.authBlobStage->offset, HEX);
}

void ppStageVisibleAuthBlob106(const AuthBlobRuntimeRefs& refs)
{
  ppStageVisibleAuthBlob(refs,
                         refs.authBlobVisibleValue,
                         static_cast<uint16_t>(refs.authBlobVisibleValueLen),
                         6,
                         0x25);
}

void ppNotifyStagedAuthBlob106(const AuthBlobRuntimeRefs& refs, const char* reason)
{
  if (!refs.authBlobStage->ready) {
    Serial.print("Auth blob notify skipped via ");
    Serial.print(reason);
    Serial.println(": no staged blob");
    return;
  }
  refs.authBlobStage->streamFlags |= 0x01;
  refs.authBlobStage->offset = 0;
  refs.authParserOutput->schedulerState = 2;
  refs.authParserOutput->notifyInFlight = true;
  refs.authParserOutput->outputOffset = 0;
  refs.authBlob->write(refs.authBlobVisibleValue, refs.authBlobStage->totalLen);
  refs.authBlob->notify(refs.authBlobVisibleValue, refs.authBlobStage->totalLen);
  refs.authBlobStage->offset = refs.authBlobStage->totalLen;
  refs.authBlobStage->streamFlags &= static_cast<uint8_t>(~0x01);
  refs.authParserOutput->outputOffset = refs.authParserOutput->outputLen;
  refs.authParserOutput->streamPending = false;
  refs.authParserOutput->notifyInFlight = false;
  Serial.print("Auth blob notify via ");
  Serial.print(reason);
  Serial.print(" class=");
  Serial.print(refs.authBlobStage->stageClass);
  Serial.print(" len=0x");
  Serial.print(refs.authBlobStage->totalLen, HEX);
  Serial.print(" flags=0x");
  Serial.print(refs.authBlobStage->streamFlags, HEX);
  Serial.print(" lastOpcode=0x");
  Serial.println(refs.authBlobStage->lastOpcode, HEX);
}
