#pragma once

#include <Arduino.h>
#include <bluefruit.h>

void ppPrintHex(const uint8_t* data, uint16_t len);
void ppPrintBuildTag(const char* buildTag, const char* context);
