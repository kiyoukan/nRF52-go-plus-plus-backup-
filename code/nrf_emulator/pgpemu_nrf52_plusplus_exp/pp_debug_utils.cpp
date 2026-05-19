#include "pp_debug_utils.h"

#include "pp_log.h"

void ppPrintHex(const uint8_t* data, uint16_t len)
{
#if PP_LOG_VERBOSE_ENABLED
  for (uint16_t i = 0; i < len; ++i) {
    if (data[i] < 0x10) {
      Serial.print('0');
    }
    Serial.print(data[i], HEX);
    if (i + 1 != len) {
      Serial.print(' ');
    }
  }
#else
  (void) data;
  Serial.print("<hex hidden len=0x");
  Serial.print(len, HEX);
  Serial.print(">");
#endif
}

void ppPrintBuildTag(const char* buildTag, const char* context)
{
  Serial.print("Build tag");
  if (context != nullptr && context[0] != '\0') {
    Serial.print(" [");
    Serial.print(context);
    Serial.print("]");
  }
  Serial.print(": ");
  Serial.println(buildTag);
}
