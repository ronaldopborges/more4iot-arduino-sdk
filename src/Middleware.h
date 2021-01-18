
#ifndef Middleware_Arduino_Sdk_h
#define Middleware_Arduino_Sdk_h

#define DATA_HEADER_FIELDS_AMT 3

#include <vector>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "ArduinoJson/Polyfills/type_traits.hpp"

class MiddlewareDefaultLogger;
using Logger = MiddlewareDefaultLogger;

class DataAttribute
{
  friend class Middleware;

public:
  inline DataAttribute()
      : dataAtTypeEnum(TYPE_NONE), dataAtName(NULL), dataAtValue() {}

  inline DataAttribute(const char *name, int value)
      : dataAtTypeEnum(TYPE_INT), dataAtName(name), dataAtValue() { dataAtValue.integer = value; }

  inline DataAttribute(const char *name, bool value)
      : dataAtTypeEnum(TYPE_BOOL), dataAtName(name), dataAtValue() { dataAtValue.boolean = value; }

  inline DataAttribute(const char *name, double value)
      : dataAtTypeEnum(TYPE_REAL), dataAtName(name), dataAtValue() { dataAtValue.real = value; }

  inline DataAttribute(const char *name, const char *value)
      : dataAtTypeEnum(TYPE_STR), dataAtName(name), dataAtValue() { dataAtValue.str = value; }

  const char *toStringValueStr()
  {
    if (dataAtTypeEnum == TYPE_STR)
    {
      return (String(dataAtName) + String(": ") + String(dataAtValue.str)).c_str();
    }
    return NULL;
  }

private:
  union DataAttributeValueUnion
  {
    const char *str;
    bool boolean;
    int integer;
    float real;
  };

  enum DataAttributeTypeEnum
  {
    TYPE_NONE,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_REAL,
    TYPE_STR,
  };

  DataAttributeTypeEnum dataAtTypeEnum;
  const char *dataAtName;
  DataAttributeValueUnion dataAtValue;

  bool serializeDataAt(JsonObject &jsonObj) const;
};

class Middleware
{
public:
  inline Middleware(Client &client) : mqtt_client(client) {}
  inline ~Middleware() {}

  bool mqttConnect(const char *host, int port = 1883)
  {
    if (!host)
    {
      return false;
    }
    mqtt_client.setServer(host, port);
    return mqtt_client.connect("inputCommunicator");
  }

  inline void mqttDisconnect()
  {
    mqtt_client.disconnect();
  }

  inline bool mqttConnected()
  {
    return mqtt_client.connected();
  }

  inline void mqttLoop()
  {
    mqtt_client.loop();
  }

  bool mqttPublish()
  {
    String data = getDataObjectJson();
    mqtt_client.publish("inputCommunicator", data.c_str());
    Serial.println(data.c_str());
  }

  bool newDataObject(const char *token, int dataFieldsAmount, double lon = 0.0, double lat = 0.0)
  {
    this->fieldsAmount = dataFieldsAmount + DATA_HEADER_FIELDS_AMT;
    this->memoryAllocated = JSON_OBJECT_SIZE(this->fieldsAmount);
    dataHeader.clear();
    dataFields.clear();
    dataHeader.push_back(DataAttribute("deviceUuid", token));
    dataHeader.push_back(DataAttribute("lat", lat));
    dataHeader.push_back(DataAttribute("lon", lon));
    Serial.println("Data Object created...");
    return true;
  }

  template <typename T>
  bool addField(const char *nameField, T value)
  {
    dataFields.push_back(DataAttribute(nameField, value));
    return true;
  }

private:
  inline size_t jsonObjectSize(size_t tam) { return tam * sizeof(ARDUINOJSON_NAMESPACE::VariantSlot); }
  inline void setFieldsAmount(size_t fieldsAmount) { this->fieldsAmount = fieldsAmount; }

  String getDataObjectJson()
  {
    if (this->fieldsAmount < (dataFields.size() + DATA_HEADER_FIELDS_AMT))
    {
      Serial.println("too much JSON fields passed");
      return "";
    }
    String payload;
    DynamicJsonDocument jsonBuffer(memoryAllocated);
    JsonObject jsonRoot = jsonBuffer.to<JsonObject>();

    for (DataAttribute d : dataHeader)
    {
      if (d.serializeDataAt(jsonRoot) == false)
      {
        Serial.println("unable to serialize data");
        return "";
      }
    }
    JsonObject jsonData = jsonRoot.createNestedObject("data");
    for (DataAttribute d : dataFields)
    {
      if (d.serializeDataAt(jsonData) == false)
      {
        Serial.println("unable to serialize data");
        return "";
      }
    }
    if (measureJson(jsonBuffer) > memoryAllocated - 1)
    {
      Serial.println("too small buffer for JSON data");
      return "";
    }
    serializeJson(jsonRoot, payload);

    return payload;
  }

  PubSubClient mqtt_client;

  //dataobject
  std::vector<DataAttribute> dataHeader;
  std::vector<DataAttribute> dataFields;
  int fieldsAmount;
  size_t memoryAllocated;
};

class MiddlewareDefaultLogger
{
public:
  static void log(const char *msg);
};

#endif