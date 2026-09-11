#include <WiFi.h>
#include <HTTPClient.h>
#include "alex_secrets.h"


String url = "http://" + String(SERVER_IP) + ":" + String(SERVER_PORT) + "/api/sensor-data";

int cleaningFillingFlag = 0; // 0 is filling, 1 is cleaning
double soapWeight = 20;

void setup() {
  Serial.begin(115200);

  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting...");
  }

  Serial.println("Connected to WiFi");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(url);
    http.setTimeout(5000);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("x-api-key", API_KEY);

    String body = "{\"cleaningOrFilling\": " + String(cleaningFillingFlag) + 
              ", \"soapWeight\": " + String(soapWeight) + "}";

    int httpResponseCode = http.POST(body);

    Serial.print("Response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(response);
    }

    http.end();
  } else {
    Serial.println("WiFi disconnected, reconnecting...");
    WiFi.begin(SSID, PASSWORD);
  }

  delay(1000);
}