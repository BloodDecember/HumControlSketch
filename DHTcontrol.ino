#include "DHT.h"
#include <ESP8266WiFi.h>
#include <EEPROM.h>

#define DHTPIN 2     // what digital pin we're connected to

//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

const char* ssid = "December";
const char* password = "23611455";
WiFiServer server(80);

String hma;
String hmi;
int hum;
int tem;

DHT dht(DHTPIN, DHTTYPE);

void write_string_EEPROM (int Addr, String Str) {
  byte lng = Str.length();
  EEPROM.begin (64);
  if (lng > 15 )  lng = 0;
  EEPROM.write(Addr , lng);
  unsigned char* buf = new unsigned char[15];
  Str.getBytes(buf, lng + 1);
  Addr++;
  for (byte i = 0; i < lng; i++) {
    EEPROM.write(Addr + i, buf[i]);
    delay(10);
  }
  EEPROM.end();
}

char *read_string_EEPROM (int Addr) {
  EEPROM.begin(64);
  byte lng = EEPROM.read(Addr);
  char* buf = new char[15];
  Addr++;
  for (byte i = 0; i < lng; i++) buf[i] = char(EEPROM.read(i + Addr));
  buf[lng] = '\x0';
  return buf;
}

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println("DHT22 Start");

  hma = read_string_EEPROM (0);
  hma = hma.toInt() / 100;
  hmi = read_string_EEPROM (0);
  hmi = hmi.toInt() % 100;

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");

  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());

  dht.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    //    return;

    Serial.println("new client");
    while (!client.available()) {
      delay(1);
    }


    String req = client.readStringUntil('\r');
    Serial.println(req);
    client.flush();


    if (req == "H") {                                          //Запрос по TCP
      hum = dht.readHumidity();
      client.print(hum);
    }
    else if (req == "T") {
      tem = dht.readTemperature();
      client.print(tem);
    }
    else if (req.indexOf("MAX") != -1) {
      client.print(hma);
    }
    else if (req.indexOf("MIN") != -1) {
      client.print(hmi);
    }
    else if (req.indexOf("HMAI") != -1) {
      hma = req.toInt() / 100;
      hmi = req.toInt() % 100;
      req = req.toInt();
      write_string_EEPROM (0, req);
    }
    else {
      //      hma = req.toInt();
      //      write_string_EEPROM (0, req);
    }
  }
  if (dht.readHumidity() < hmi.toInt()) {
    pinMode(0, OUTPUT);
    digitalWrite(0, HIGH);
  }

  if (dht.readHumidity() > hma.toInt()) {
    pinMode(0, OUTPUT);
    digitalWrite(0, LOW);
  }
}
