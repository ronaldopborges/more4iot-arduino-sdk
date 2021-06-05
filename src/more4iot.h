
#ifndef more4iot_arduino_sdk_h
#define more4iot_arduino_sdk_h

#define DATA_HEADER_FIELDS_AMT 3

#include <vector>
#include <ArduinoJson.h>
#include "ArduinoJson/Polyfills/type_traits.hpp"

#include <ArduinoHttpClient.h>
#include <PubSubClient.h>

class More4iotDefaultLogger
{
public:
  static void log(const char *msg);
};
using Logger = More4iotDefaultLogger;

class DataAttribute
{
  friend class DataObjectImpl;

public:
  inline DataAttribute()
      : dataType(TYPE_NONE), dataName(NULL), dataValue() {}

  inline DataAttribute(const char *name, int value)
      : dataType(TYPE_INT), dataName(name), dataValue() { dataValue.integer = value; }

  inline DataAttribute(const char *name, bool value)
      : dataType(TYPE_BOOL), dataName(name), dataValue() { dataValue.boolean = value; }

  inline DataAttribute(const char *name, double value)
      : dataType(TYPE_REAL), dataName(name), dataValue() { dataValue.real = value; }

  inline DataAttribute(const char *name, const char *value)
      : dataType(TYPE_STR), dataName(name), dataValue() { dataValue.str = value; }

  const char *toStringValueStr()
  {
    if (dataType == TYPE_STR)
    {
      return (String(dataName) + String(": ") + String(dataValue.str)).c_str();
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

  DataAttributeTypeEnum dataType;
  const char *dataName;
  DataAttributeValueUnion dataValue;
};

class DataObjectImpl
{
protected:
  std::vector<DataAttribute> dataHeader;
  std::vector<DataAttribute> dataFields;
  int fieldsAmount;
  size_t memoryAllocated;

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

public:
  inline void setFieldsAmount(size_t fieldsAmt) { fieldsAmount = fieldsAmt; }

  bool newDataObject(const char *uuid, int dataFieldsAmount, double lon = 0.0, double lat = 0.0)
  {
    this->fieldsAmount = dataFieldsAmount + DATA_HEADER_FIELDS_AMT;
    this->memoryAllocated = JSON_OBJECT_SIZE(this->fieldsAmount);
    dataHeader.clear();
    dataFields.clear();
    dataHeader.push_back(DataAttribute("uuid", uuid));
    dataHeader.push_back(DataAttribute("lat", lat));
    dataHeader.push_back(DataAttribute("lon", lon));
    Serial.println("Data object created...");
    return true;
  }

  template <typename T>
  bool addField(const char *nameField, T value)
  {
    dataFields.push_back(DataAttribute(nameField, value));
    return true;
  }
};

class More4iot
{
public:
  virtual bool connect(const char *host, int port);
  virtual inline void disconnect();
  virtual inline void connected();
  virtual bool send();
};

class More4iotMqtt : public DataObjectImpl
{
private:
  inline size_t jsonObjectSize(size_t tam) { return tam * sizeof(ARDUINOJSON_NAMESPACE::VariantSlot); }
  PubSubClient mqtt_client;
  String topicPublish = "input";
  String user = "more4iot";
  String pass = "1234";

public:
  inline More4iotMqtt(Client &client)
      : mqtt_client(client) {}
  inline ~More4iotMqtt() {}

  bool connect(const char *host, int port = 1883)
  {
    if (!host)
    {
      Serial.println("Failed to connect: host not found...");
      return false;
    }
    Serial.println("putting mqtt host and port...");
    Serial.println(host);
    Serial.println(port);
    mqtt_client.setServer(host, port);
    Serial.println("connecting mqtt host with user and pass...");
    Serial.println(user);
    Serial.println(pass);
    return mqtt_client.connect("resource_id", user.c_str(), pass.c_str());
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

  bool send()
  {
    if (!this->connected())
    {
      Serial.println("not send...");
      return false;
    }
    String data = getDataObjectJson();
    mqtt_client.publish(topicPublish.c_str(), data.c_str());
    Serial.println(data.c_str());
    return true;
  }
};

class More4iotHttp : public DataObjectImpl
{
public:
  inline More4iotHttp(Client &client,
                      const char *host, int port = 80)
      : httpClient(client, host, port), host(host), port(port) {}
  inline ~More4iotHttp() {}

  bool sendJson()
  {
    if (!httpClient.connected())
    {
      if (!httpClient.connect(host, port))
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
  const char *host;
  int port;
};

#endif