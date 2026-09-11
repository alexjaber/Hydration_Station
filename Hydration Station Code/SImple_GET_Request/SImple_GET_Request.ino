#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Samsung";
const char* password = "hs247!KMP";

const char* serverIP = "172.20.10.3";
const int serverPort = 4000;
String url = "http://" + String(serverIP) + ":" + String(serverPort) + "/api/sensor-data";

int cleaningFillingFlag = 0; // 0 is filling, 1 is cleaning
double soapWeight = 20;




void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting...");
  }

  Serial.println("Connected to WiFi");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = "http://" + String(serverIP) + ":" + String(serverPort) + "/api/sensor-data";
    http.begin(url);
    http.setTimeout(5000);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("x-api-key", "hydration-station-device-2026");

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
    WiFi.begin(ssid, password);
  }

  delay(1000);
}