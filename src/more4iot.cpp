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

//MQTT
bool More4iotMqtt::connect() 
{
  if (!ip)
  {
    Serial.println("Failed to connect: host not found...");
    return false;
  }
  Serial.println("putting mqtt host and port...");
  mqttClient.setServer(ip, port);
  Serial.println("connecting mqtt...");
  return mqttClient.connect("resource_id", user.c_str(), pass.c_str());
}
inline void More4iotMqtt::disconnect() 
{
  mqttClient.disconnect();
}
inline bool More4iotMqtt::connected() 
{
  return mqttClient.connected();
}
void More4iotMqtt::loop() 
{
  mqttClient.loop();
}
bool More4iotMqtt::send() 
{
  if (!this->connected())
  {
    Serial.println("not send...");
    return false;
  }
  String data = getDataPacketJson();
  mqttClient.publish(topicPublish.c_str(), data.c_str());
  Serial.println(data.c_str());
  return true;
}

//HTTP
bool More4iotHttp::connect()
{
  if (!httpClient.connect(host, port))
  {
    Serial.println("connect to server failed");
    return false;
  }
  return true;
};
inline void More4iotHttp::disconnect()
{
  httpClient.stop();
};
bool More4iotHttp::connected()
{
  return httpClient.connected();
};
bool More4iotHttp::send()
{

  if (!this->connected())
  {
    if (!this->connect())
    {
      return false;
    }
  }
  String payload = getDataPacketJson();
  Serial.println(payload);
  httpClient.post(route, contentType, payload);
  if (httpClient.responseStatusCode() != HTTP_SUCCESS && httpClient.responseStatusCode() != HTTP_ERROR_INVALID_RESPONSE)
  {
    Serial.print("data not sent: ");
    Serial.println(httpClient.responseStatusCode());
    Serial.println(httpClient.responseBody());
    this->disconnect();
    return false;
  }

  this->disconnect();
  Serial.println("data sent");
  return true;
}

//CoAP 
bool More4iotCoap::connect()
{
  return coap.start(port);
}
void More4iotCoap::loop()
{
  coap.loop();
}
bool More4iotCoap::send()
{
  String data = getDataPacketJson();
  Serial.print("send(): retorno do put: ");
  Serial.println(coap.put(ip, port, endpointInput.c_str(), data.c_str()));
  Serial.println(data.c_str());
  return true;
}
void More4iotCoap::response(CoapCallback c)
{
  Serial.println("response()");
  coap.response(c);
}
void More4iotCoap::server(CoapCallback c, String url)
{
  coap.server(c, url);
}
void More4iotCoap::sendResponse(IPAddress ip, int port, uint16_t messageid, const char *payload)
{
  coap.sendResponse(ip,port,messageid,payload);
}