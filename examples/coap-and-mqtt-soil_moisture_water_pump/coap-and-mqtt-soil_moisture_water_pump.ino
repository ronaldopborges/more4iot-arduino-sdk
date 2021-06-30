
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
int seco = 4095;
float unidade = 30.95;
int result;

// MORE4IoT Config
#define TIME_TO_SEND 30000
#define UUID "6e0efe80-d9b0-11eb-ab9c-b52141224ba9"
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

  int value = coap.data<int>(packet.payload, "soil-moisture");
  if (value < 30){
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

  // soil_moisture = analog(1000-4095) to percentage (0-100)
  valueA = analogRead(pinSensor);
  Serial.println(valueA);
  soilMoisture = (int)((seco-valueA)/unidade);
  Serial.println(soilMoisture);
  
  // UUID, latitude and longitude (can be 0.0, 0.0)
  mqtt.newDataObject(UUID, 0.0, 0.0);
  // data fields
  mqtt.addField("soil-moisture",soilMoisture);
  mqtt.send();

  delay(1000);
  coap.loop();
  delay(TIME_TO_SEND);
}
