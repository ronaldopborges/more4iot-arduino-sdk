#include "more4iot.h"
#include "secrets.h" //wifi and server credentials
#include "WiFi.h"

#define TIME_TO_SEND 10000
#define SERVER 192,168,0,186 //MORE4IOT
//#define PORT 1883 //Optional. Standad value is 1883.
#define UUID "deviceXX"

IPAddress serverIp(SERVER);

WiFiClient espClient;

More4iot * md = new More4iotMqtt(espClient, serverIp);
// OR   
// More4iotMqtt md(espClient, serverIp);
// More4iotMqtt md(espClient, serverIp, PORT);
// More4iotMqtt md(espClient, serverIp, PORT, USER, PASS);
// PORT, USER and PASS are opitionals. If not informed standard values are used.

void setup()
{
  Serial.begin(115200);
  delay(10);
  
  Serial.print("\n\nConnecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.print("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());

  md->connect();
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED){
    Serial.println("Connecting to AP ...");
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    return;
  }

  if(!md->connected()){
    Serial.println("disconnected to MORE4IoT...");
    md->connect();
    Serial.println("Reconected!");
    delay(1000);
    return;
  }
  
  // UUID, latitude and longitude (can be 0.0, 0.0)
  md->newDataPacket(UUID, 1.0, -2.0);
  // data fields
  md->addField("temperature", 25);
  md->send();

  md->loop();

  Serial.println("data sent");
  delay(TIME_TO_SEND);
}