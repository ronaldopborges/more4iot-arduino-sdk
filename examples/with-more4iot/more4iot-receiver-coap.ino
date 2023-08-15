
#include "more4iot.h"
#include "secrets.h"

#include "WiFiUdp.h"

// device config
#define TIME_TO_SEND 10000
#define pinActuator 2
char ssid[] = WIFI_SSID;
char password[] = WIFI_PASSWORD;
#define UUID "651e29c0-da70-11eb-b8e1-b757525fcd00"
int drySoil = 30;
int soilMoisture = 0;
// device config

// MORE4IoT Config
IPAddress more4iotIp(192,168,0,5);
int more4iotCoapPort = 5683;

WiFiUDP udpClient;
More4iotCoap more4iot = 
  More4iotCoap(udpClient, more4iotIp, more4iotCoapPort);
// ---- //

// callback for water-pump
void waterPump(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("[Water Pump] ON/OFF");

  // get dry soil value from action
  drySoil = more4iot.getCommand<int>(packet.payload, "dry-soil");

  // get soil moisture value from action
  soilMoisture = more4iot.getData<int>(packet.payload, "soil-moisture");
  if (soilMoisture < drySoil){
    digitalWrite(pinActuator, HIGH);
  } else {
    digitalWrite(pinActuator, LOW);
  }
}

void setup()
{
  // led (water-pump)
  pinMode(pinActuator, OUTPUT);
  Serial.begin(9600);
  delay(10);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  more4iot.response(response);

  // more4iot connect and callback
  more4iot.callback(waterPump, "soil-moisture");
  more4iot.connect();
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED){
    Serial.println("Connecting to AP ...");
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    return;
  }

  delay(1000);
  more4iot.loop();
  delay(TIME_TO_SEND);
}
