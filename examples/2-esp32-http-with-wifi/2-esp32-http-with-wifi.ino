
#include "Middleware.h"
#include "secrets.h"

#include "WiFi.h"

#define TIME_TO_SEND 10000

char ssid[] = WIFI_SSID;
char password[] = WIFI_PASSWORD;
char token[] = TOKEN;
char server[] = SERVER;
int port = PORT;

WiFiClient espClient;
MiddlewareHttp md(espClient, server, port);

void setup()
{
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

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
}

void loop()
{
  delay(TIME_TO_SEND);

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connecting to AP ...");
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    return;
  }

  md.newDataObject(TOKEN, 4, 1.0, -2.0);
  md.addField("temperature", 25);
  md.sendJson();

  Serial.println("Sending data...");
}
