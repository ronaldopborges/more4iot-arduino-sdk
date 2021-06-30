
#include "more4iot.h"
#include "secrets.h"

#include "WiFi.h"
#include "WiFiUdp.h"

// CoAP client response callback
void callback_response(CoapPacket &packet, IPAddress ip, int port);

// CoAP server endpoint url callback
void callback_light(CoapPacket &packet, IPAddress ip, int port);

char ssid[] = WIFI_SSID;
char password[] = WIFI_PASSWORD;

// more4iot coap server ip and port
IPAddress ip(192,168,0,5);
int port = 5683;

// UDP and MORE4IoT CoAP class
WiFiUDP Udp;
More4iotCoap md(Udp, ip, port);

// CoAP client response callback
void callback_response(CoapPacket &packet, IPAddress ip, int port)
{
  Serial.println("[Coap Response]");

  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = NULL;

  Serial.println(p);
}

// LED STATE
bool LEDSTATE;

// CoAP actuator
void callback_light(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("[Light] ON/OFF");

  // get command for {"commands":{"light":true}}
  if (md.commands<bool>(packet.payload, "light")){
    LEDSTATE = true;
    digitalWrite(2, HIGH);
  } else {
    LEDSTATE = false;
    digitalWrite(2, LOW);
  }
}

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

  // LED State
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  LEDSTATE = false;

  Serial.println("Setup Callback Light");
  // callback and endpoint
  md.server(callback_light, "light");

  // client response callback.
  // this endpoint is single callback.
  // Serial.println("Setup Response Callback");
  md.response(callback_response);

  // start coap client
  md.connect();
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

  // loop
  md.loop();
}