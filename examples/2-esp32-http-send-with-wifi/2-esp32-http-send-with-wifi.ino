
#include "more4iot.h"
#include "secrets.h"

#include "WiFi.h"

#define TIME_TO_SEND 10000

#define HOST "192.168.0.186"
#define PORT 3333
#define UUID "deviceXX"

char ssid[] = WIFI_SSID;
char password[] = WIFI_PASSWORD;
char uuid[] = UUID;
char server[] = HOST;
int port = PORT;

WiFiClient espClient;
More4iot * md = new More4iotHttp(espClient, server, port);
//or 
// More4iotHttp md(espClient, server, port);

void setup()
{
  Serial.begin(115200);
  delay(10);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connecting to AP ...");
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    return;
  }

  md->newDataPacket(uuid, 1.0, -2.0);
  md->addField("temperature", 25);
  md->addField("humidity", 60);
  md->send();

  delay(TIME_TO_SEND);
}
