#include "more4iot.h"

// device default config

// MORE4IoT Config
IPAddress more4iotIp(MORE4IOT_IP);
int more4iotMqttPort = MQTT_PORT;

WiFiClient tcpClient;
More4iot more4iot =
  More4iotMqtt(tcpClient, more4iotIp, more4iotMqttPort);
// ---- //

void setup()
{
  // pin mode config
  // wifi config
  // more4iot connect
  more4iot.connect();
}

void loop()
{
  // wifi connection verify
  // more4iot connection verify
  if(!more4iot.connected()){
    more4iot.connect();
    return;
  }
  // soil moisture calc
  // create data packet with uuid, soil moisture data and send
  more4iot.newDataPacket(UUID);
  more4iot.addField("soil-moisture",soilMoisture);
  more4iot.send();
  // delay time
}