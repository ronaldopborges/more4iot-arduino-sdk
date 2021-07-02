
#include "more4iot.h"
#include "secrets.h"

#include "WiFi.h"
#include "WiFiUdp.h"

// application config
#define pinSensor 34
#define pinActuator 2
char ssid[] = WIFI_SSID;
char password[] = WIFI_PASSWORD;
int soilMoisture;
int valueA;
int minValue = 800;
int maxValue = 4095;
int drySoil = 30;
// application config

// MORE4IoT Config
#define TIME_TO_SEND 10000
#define UUID "651e29c0-da70-11eb-b8e1-b757525fcd00"
IPAddress ip(192,168,0,5);
int coap_port = 5683;
int mqtt_port = 1883;

WiFiClient tcpClient;
WiFiUDP udpClient;
More4iotMqtt mqtt(tcpClient, ip, mqtt_port);
More4iotCoap coap(udpClient, ip, coap_port);
// ---- //

// callback for water-pump
void waterPump(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("[Light] ON/OFF");

  drySoil = coap.getCommand<int>(packet.payload, "dry-soil");

  soilMoisture = coap.getData<int>(packet.payload, "soil-moisture");
  if (soilMoisture < drySoil){
    digitalWrite(pinActuator, HIGH);
  } else {
    digitalWrite(pinActuator, LOW);
  }
}

// CoAP client response callback
void response(CoapPacket &packet, IPAddress ip, int port)
{
  // 
}

void setup()
{
  // analog D04 esp32
  pinMode(pinSensor, INPUT);
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

  coap.server(waterPump, "soil-moisture");
  coap.response(response);

  // mqtt connect to publish
  mqtt.connect();
  // coap connect to receiver
  coap.connect();
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

  if(!mqtt.connected()){
    Serial.println("mqtt disconnected...");
    mqtt.connect();
    delay(1000);
    return;
  }

  // soil_moisture = analog(800-4095) to percentage (0-100)
  // 4095 -> 0
  // 800 -> 100
  valueA = analogRead(pinSensor);
  soilMoisture = map(valueA, maxValue, minValue, 0, 100);
  
  // UUID, latitude and longitude (can be 0.0, 0.0)
  mqtt.newDataObject(UUID, 0.0, 0.0);
  // data fields
  mqtt.addField("soil-moisture",soilMoisture);
  mqtt.send();

  delay(1000);
  coap.loop();
  delay(TIME_TO_SEND);
}
