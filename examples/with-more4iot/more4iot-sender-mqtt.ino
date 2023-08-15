
#include "more4iot.h"
#include "secrets.h"

#include "WiFi.h"

// device config
#define pinSensor 34
#define TIME_TO_SEND 10000

char ssid[] = WIFI_SSID;
char password[] = WIFI_PASSWORD;
int soilMoisture;
int valueA;
int minValue = 800;
int maxValue = 4095;
// device config

// MORE4IoT Config
IPAddress more4iotIp(192,168,0,5);
int more4iotMqttPort = 1883;

WiFiClient tcpClient;
More4iot more4iot = More4iotMqtt(tcpClient, more4iotIp, more4iotMqttPort);
// ---- //

void setup()
{
  // analog D04 esp32
  pinMode(pinSensor, INPUT);
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

  // more4iot connect
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

  if(!more4iot.connected()){
    Serial.println("more4iot disconnected...");
    more4iot.connect();
    delay(1000);
    return;
  }

  // soil moisture calc
  // soil_moisture = analog(800-4095) to percentage (0-100)
  // 4095 -> 0
  // 800 -> 100
  valueA = analogRead(pinSensor);
  soilMoisture = map(valueA, maxValue, minValue, 0, 100);
  
  // create data packet with uuid, soil moisture data and send
  more4iot.newDataPacket(uuid);
  more4iot.addField("soil-moisture",soilMoisture);
  more4iot.send();

  delay(TIME_TO_SEND);
}
