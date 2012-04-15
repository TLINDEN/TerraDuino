
#define WEBDUINO_FAIL_MESSAGE "<h1>Request Failed</h1>"
#include "SPI.h" // new include
#include "avr/pgmspace.h" // new include
#include "Ethernet.h"
#include "WebServer.h"

struct DATA_index {
  float temp;
  float humidity;
};

template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; }

char endl = '\n';

#include "Templates.h"

#include "DHT.h"

#define DHTPIN 8
#define DHTTYPE DHT22

static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static uint8_t ip[] = { 10, 0, 0, 2 };

float temperature;
float humidity;

DHT dht(DHTPIN, DHTTYPE);

#define PREFIX ""
WebServer webserver(PREFIX, 80);

void getSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(t) || isnan(h)) {
    Serial.println("Failed to read from DHT");
  }
  else {
     temperature = t;
     humidity    = h;
  }
}

void tempCmd(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  if (type == WebServer::POST)  {
    server.httpSeeOther("PREFIX");
    return;
  }

  server.httpSuccess();

  if (type == WebServer::GET) {
    getSensorData();
    DATA_index data;
    data.temp = temperature;
    data.humidity = humidity;
    tpl_index(server, data);
  }
}

void setup() {
  Ethernet.begin(mac, ip);

  webserver.setDefaultCommand(&tempCmd);

  webserver.begin();
  
  dht.begin();
}

void loop() {
  webserver.processConnection();
}

