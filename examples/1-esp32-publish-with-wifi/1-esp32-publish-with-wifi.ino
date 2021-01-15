
#include "MiddlewareArduinoSdk.h"
#include "secrets.h"

#include "Wifi.h"

WiFiClient espClient;
Middleware md(espClient);                    

void setup()
{
    Serial.begin(115200);
    delay(10);

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(WIFI_AP, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void loop() {
  delay(1000);

  if (status != WL_CONNECTED) {
    Serial.println("Connecting to AP ...");
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(WIFI_AP);
    status = WiFi.begin(WIFI_AP, WIFI_PASSWORD);
    return;
  }

  if (!md.mqttConnected()) {
    // Connect to the ThingsBoard
    Serial.print("Connecting to: ");
    Serial.print(MQTT_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!md.mqttConnect(MQTT_SERVER)) {
      Serial.println("Failed to connect");
      return;
    }
  }

  md.newDataObject(TOKEN, 4, 1.0, -2.0);
  md.addField("temperature", 25);
  md.mqttPublish();

  Serial.println("Sending data...");

}
