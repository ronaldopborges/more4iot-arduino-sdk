#include "more4iot.h"

// device default config

// MORE4IoT Config
IPAddress more4iotIp(MORE4IOT_IP);
int more4iotCoapPort = COAP_PORT;

WiFiUDP udpClient;
More4iot more4iot = 
  More4iotCoap(udpClient, more4iotIp, more4iotCoapPort);
// ---- //

// callback for water-pump
void waterPump(CoapPacket &packet, IPAddress ip, int port) {
  // get dry soil value from scope
  drySoil = more4iot.getCommand<int>(packet.payload, "dry-soil");
  // get soil moisture value from scope
  soilMoisture = more4iot.getData<int>(packet.payload, "soil-moisture");
  // water pump on / off
}

void setup()
{
  // pin mode config
  // wifi config
  // more4iot connect and callback
  more4iot.callback(waterPump, "soil-moisture");
  more4iot.connect();
}

void loop()
{
  // wifi connection verify
  // more4iot packet verify
  more4iot.loop();
  // delay time
}