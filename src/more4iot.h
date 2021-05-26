
#ifndef more4iot_arduino_sdk_h
#define more4iot_arduino_sdk_h

#define DATA_HEADER_FIELDS_AMT 3

#include <vector>
#include <ArduinoJson.h>
#include "ArduinoJson/Polyfills/type_traits.hpp"

#include <ArduinoHttpClient.h>
#include <PubSubClient.h>

class MiddlewareDefaultLogger
{
public:
  static void log(const char *msg);
};
using Logger = MiddlewareDefaultLogger;

class DataAttribute
{
  friend class DataObjectImpl;

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

  bool serializeDataAt(JsonObject &jsonObj) const;

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
};

class DataObjectImpl
{
public:
  inline void setFieldsAmount(size_t fieldsAmt) { fieldsAmount = fieldsAmt; }

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

protected:
  String getDataObjectJson()
  {
    if (fieldsAmount < (dataFields.size() + DATA_HEADER_FIELDS_AMT))
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

    Serial.println(payload.c_str());
    return payload;
  }

  std::vector<DataAttribute> dataHeader;
  std::vector<DataAttribute> dataFields;
  int fieldsAmount;
  size_t memoryAllocated;
};

class MiddlewareMqtt : public DataObjectImpl
{
public:
  inline MiddlewareMqtt(Client &client) : mqtt_client(client) {}
  inline ~MiddlewareMqtt() {}

  bool connect(const char *host, int port = 1883)
  {
    if (!host)
    {
      return false;
    }
    mqtt_client.setServer(host, port);
    return mqtt_client.connect("input");
  }

  inline void disconnect()
  {
    mqtt_client.disconnect();
  }

  inline bool connected()
  {
    return mqtt_client.connected();
  }

  inline void loop()
  {
    mqtt_client.loop();
  }

  bool publish()
  {
    String data = getDataObjectJson();
    mqtt_client.publish("input", data.c_str());
    Serial.println(data.c_str());
  }

private:
  inline size_t jsonObjectSize(size_t tam) { return tam * sizeof(ARDUINOJSON_NAMESPACE::VariantSlot); }

  PubSubClient mqtt_client;
};

class MiddlewareHttp : public DataObjectImpl
{
public:
  inline MiddlewareHttp(Client &client,
                        const char *host, int port = 80)
      : httpClient(client, host, port), httpHost(host), httpPort(port)
  {
  }
  inline ~MiddlewareHttp() {}

  bool sendJson()
  {
    if (!httpClient.connected())
    {
      if (!httpClient.connect(httpHost, httpPort))
      {
        Serial.println("connect to server failed");
        return false;
      }
    }

    bool rc = true;
    String path = String("/inputCommunicator");
    if (!httpClient.post(path, "application/json", getDataObjectJson()) || (httpClient.responseStatusCode() != HTTP_SUCCESS))
    {
      rc = false;
    }

    httpClient.stop();
    return rc;
  }

private:
  HttpClient httpClient;
  const char *httpHost;
  int httpPort;
};

#endif