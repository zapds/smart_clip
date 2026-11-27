#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <DNSServer.h>
#include "dashboard.h"

// Wi-Fi AP configuration
constexpr const char* AP_SSID = "SmartClip";
constexpr const char* AP_PASS = "smartclip123";
constexpr const char* MDNS_HOST = "smartclip";
constexpr uint16_t SERVER_PORT = 80;
constexpr uint16_t DNS_PORT = 53;

// Pin definitions
constexpr uint8_t DHT_PIN = 4;
constexpr uint8_t DHT_TYPE = DHT22;

constexpr uint8_t RAIN_AO_PIN = 34; // Analog output (input-only GPIO34)
constexpr uint8_t RAIN_DO_PIN = 27; // Digital output (unused when using analog threshold)
constexpr int RAIN_ANALOG_THRESHOLD = 2500; // Lower than this = raining (4095 = dry)

constexpr unsigned long LOG_INTERVAL_MS = 5000;
constexpr size_t HISTORY_SIZE = 60; // Last 5 minutes at 5s intervals

struct Reading {
  float temp;
  float hum;
  int rainAo;
  bool raining;
  bool validTemp;
  bool validHum;
};

DHT dht(DHT_PIN, DHT_TYPE);
WebServer server(SERVER_PORT);
DNSServer dnsServer;

Reading history[HISTORY_SIZE];
size_t historyCount = 0;
size_t historyHead = 0;
unsigned long lastLogMs = 0;

void recordReading() {
  int rainAnalog = analogRead(RAIN_AO_PIN);
  bool isRaining = (rainAnalog < RAIN_ANALOG_THRESHOLD);

  float tempC = dht.readTemperature();
  float humidity = dht.readHumidity();

  Reading r;
  r.temp = tempC;
  r.hum = humidity;
  r.rainAo = rainAnalog;
  r.raining = isRaining;
  r.validTemp = !isnan(tempC);
  r.validHum = !isnan(humidity);

  history[historyHead] = r;
  historyHead = (historyHead + 1) % HISTORY_SIZE;
  if (historyCount < HISTORY_SIZE) {
    historyCount++;
  }

  Serial.print("raining=");
  Serial.print(r.raining ? "true" : "false");
  Serial.print(" | rain_ao=");
  Serial.print(r.rainAo);
  Serial.print(" | temp_c=");
  if (!r.validTemp) {
    Serial.print("ERR");
  } else {
    Serial.print(r.temp, 1);
  }
  Serial.print(" | humidity=");
  if (!r.validHum) {
    Serial.println("ERR");
  } else {
    Serial.print(r.hum, 1);
    Serial.println("%");
  }
}

void handleRoot() {
  server.send_P(200, "text/html", DASHBOARD_HTML);
}

void handleApiData() {
  String json;
  json.reserve(2048);

  size_t latestIdx = (historyHead + HISTORY_SIZE - 1) % HISTORY_SIZE;
  Reading cur = historyCount > 0 ? history[latestIdx] : Reading{0, 0, 4095, false, false, false};

  json += "{\"current\":{\"raining\":";
  json += cur.raining ? "true" : "false";
  json += ",\"rain_ao\":";
  json += cur.rainAo;
  json += ",\"temp\":";
  json += cur.validTemp ? String(cur.temp, 1) : "null";
  json += ",\"hum\":";
  json += cur.validHum ? String(cur.hum, 1) : "null";
  json += "},\"history\":[";

  size_t startIdx = (historyCount < HISTORY_SIZE) ? 0 : historyHead;
  for (size_t i = 0; i < historyCount; i++) {
    if (i > 0) json += ",";
    const Reading& r = history[(startIdx + i) % HISTORY_SIZE];
    json += "{\"temp\":";
    json += r.validTemp ? String(r.temp, 1) : "null";
    json += ",\"hum\":";
    json += r.validHum ? String(r.hum, 1) : "null";
    json += "}";
  }
  json += "]}";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(RAIN_DO_PIN, INPUT);
  pinMode(RAIN_AO_PIN, INPUT);
  dht.begin();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  IPAddress ip = WiFi.softAPIP();

  dnsServer.start(DNS_PORT, "smartclip.local", ip);
  if (MDNS.begin(MDNS_HOST)) {
    MDNS.addService("http", "tcp", SERVER_PORT);
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/data", HTTP_GET, handleApiData);
  server.begin();

  Serial.print("Smart Clip AP started: ");
  Serial.print(AP_SSID);
  Serial.print(" | http://");
  Serial.print(ip);
  Serial.println(" | http://smartclip.local");

  recordReading();
  lastLogMs = millis();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  unsigned long now = millis();
  if (now - lastLogMs >= LOG_INTERVAL_MS) {
    lastLogMs = now;
    recordReading();
  }
}
