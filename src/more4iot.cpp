#include "more4iot.h"

bool DataAttribute::serializeDataAt(JsonObject &jsonObj) const
{
  if (dataName)
  {
    switch (dataType)
    {
    case TYPE_BOOL:
      jsonObj[dataName] = dataValue.boolean;
      break;
    case TYPE_INT:
      jsonObj[dataName] = dataValue.integer;
      break;
    case TYPE_REAL:
      jsonObj[dataName] = dataValue.real;
      break;
    case TYPE_STR:
      jsonObj[dataName] = dataValue.str;
      break;
    }
  }
  return true;
}

void More4iotDefaultLogger::log(const char *msg)
{
  Serial.print(F("[MORE4IoT] "));
  Serial.println(msg);
}