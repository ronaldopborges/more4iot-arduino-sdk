
#ifndef Middleware_Arduino_Sdk_h
#define Middleware_Arduino_Sdk_h

#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "ArduinoJson/Polyfills/type_traits.hpp"

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
    String payload;
    serializeJson(jsonRoot, payload);
    mqtt_client.publish("inputCommunicator", payload.c_str());
  }

  bool newDataObject(const char *token, int dataFieldsAmount, double lon = 0.0, double lat = 0.0)
  {
    jsonRoot.clear();
    this->fieldsAmount = dataFieldsAmount + 3;
    this->memoryAllocated = JSON_OBJECT_SIZE(this->fieldsAmount);
    DynamicJsonDocument doc(this->memoryAllocated);
    jsonRoot = doc.as<JsonObject>();
    jsonRoot["deviceUuid"] = token;
    jsonRoot["lat"] = lat;
    jsonRoot["lon"] = lon;
    jsonData = jsonRoot.createNestedObject("data");
    return true;
  }

  template <typename T>
  bool addField(const char *nameField, T value)
  {
    jsonData[nameField] = value;
    return true;
  }

private:
  inline size_t jsonObjectSize(size_t tam) { return tam * sizeof(ARDUINOJSON_NAMESPACE::VariantSlot); }
  inline void setFieldsAmount(size_t fieldsAmount) { this->fieldsAmount = fieldsAmount; }

  PubSubClient mqtt_client;

  //dataobject
  JsonObject jsonRoot;
  JsonObject jsonData;
  int fieldsAmount;
  size_t memoryAllocated;
};

#endif