#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN D2      // Define the pin for DHT11
#define DHTTYPE DHT11  // DHT 11

#define TRIG_PIN D0    // Define the pin for the ultrasonic sensor's trigger
#define ECHO_PIN D1    // Define the pin for the ultrasonic sensor's echo
#define BUZZER_PIN D3  // Define the pin for the buzzer
#define LED_PINm D4    // Define the pin for the LED
#define sensorPin D5
#define redPin  D6
#define greenPin  D7
#define bluePin  D8
#define ldrPin  A0

DHT_Unified dht(DHTPIN, DHTTYPE);

bool warningMode = false;

const char* ssid = "zul_unifi_2.4GHz@unifi";
const char* password = "Ammz374610";

ESP8266WebServer server(80);

unsigned long previousMillis = 0;
const long interval = 1000; // Update sensor data every 1 second

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Futuristic Sensor Dashboard</title><style>";
  html += "body { font-family: 'Arial', sans-serif; background-color: #121212; color: #ffffff; text-align: center; }";
  html += ".sensor-data { padding: 20px; border-radius: 10px; background-color: #1f1f1f; margin: 20px auto; max-width: 400px; box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.2); }";
  html += ".status-box { padding: 10px; border-radius: 10px; margin-top: 20px; max-width: 200px; margin-left: auto; margin-right: auto; }";
  html += "h1 { font-size: 24px; margin-bottom: 10px; } p { font-size: 18px; margin-bottom: 8px; } .text-box { font-size: 18px; padding: 10px; border-radius: 10px; } </style></head><body><div class=\"sensor-data\"><h1>Sensor Data</h1>";
  html += "<p id=\"temperature\">Temperature: </p><p id=\"humidity\">Humidity: </p><p id=\"distance\">Distance: </p><p id=\"vibration\">Vibration: </p><p id=\"light\">Light Level: </p></div>";
  html += "<div class=\"status-box\"><div id=\"statusText\" class=\"text-box\"></div></div></body><script>";
  html += "document.addEventListener('DOMContentLoaded', function() { const temperatureElement = document.getElementById('temperature'); const humidityElement = document.getElementById('humidity'); const distanceElement = document.getElementById('distance'); const vibrationElement = document.getElementById('vibration'); const lightElement = document.getElementById('light');";
  html += "const statusTextElement = document.getElementById('statusText'); function updateSensorData() { fetch('/data').then(response => response.json()).then(data => { temperatureElement.innerText = `Temperature: ${data.temperature} °C`; humidityElement.innerText = `Humidity: ${data.humidity} %`; distanceElement.innerText = `Distance: ${data.distance} cm`; vibrationElement.innerText = `Vibration: ${data.vibration ? 'HIGH' : 'LOW'}`; lightElement.innerText = `Light Level: ${data.light}`; ";
  html += "let statusText = 'Safe Zone'; let statusColor = '#00FF00'; if (data.flood) { statusText = 'Flood Incoming'; statusColor = '#FF0000'; } if (data.earthquake) { statusText = 'Earthquake'; statusColor = '#FF0000'; } if (data.extreme_heat) { statusText = 'Extremely Hot Weather'; statusColor = '#FF0000'; } statusTextElement.innerText = statusText; statusTextElement.style.backgroundColor = statusColor; }); } setInterval(updateSensorData, 1000); });</script></html>";
  server.send(200, "text/html", html);
}


void handleData() {
  sensors_event_t event;
  float temperature = dht.temperature().getEvent(&event) ? event.temperature : -1;
  float humidity = dht.humidity().getEvent(&event) ? event.relative_humidity : -1;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration * 0.034 / 2;

  int sensorValue = digitalRead(sensorPin);
  int lightValue = analogRead(ldrPin);

  bool floodWarning = (distance < 15);
  bool earthquakeWarning = (sensorValue == HIGH);
  bool extremeHeatWarning = (temperature > 37);

  String json = "{\"temperature\": " + String(temperature) + ", \"humidity\": " + String(humidity) + ", \"distance\": " + String(distance) + ", \"vibration\": " + String(sensorValue == HIGH ? "true" : "false") + ", \"light\": " + String(lightValue) + ", \"flood\": " + String(floodWarning ? "true" : "false") + ", \"earthquake\": " + String(earthquakeWarning ? "true" : "false") + ", \"extreme_heat\": " + String(extremeHeatWarning ? "true" : "false") + "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PINm, OUTPUT);
  pinMode(sensorPin, INPUT_PULLUP);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(ldrPin, INPUT);

  dht.begin();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/data", HTTP_GET, handleData);

  server.begin();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    sensors_event_t event;
    if(dht.temperature().getEvent(&event)) {
      float temperature = event.temperature;
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println(" °C");
    } else {
      Serial.println("Error reading temperature!");
    }

    if(dht.humidity().getEvent(&event)) {
      float humidity = event.relative_humidity;
      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.println("%");
    } else {
      Serial.println("Error reading humidity!");
    }

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH);
    int distance = duration * 0.034 / 2;

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
    Serial.println(WiFi.localIP());


    int sensorValue = digitalRead(sensorPin);

    if (distance < 15 || sensorValue == HIGH) {
      digitalWrite(LED_PINm, HIGH);
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, LOW);
      digitalWrite(bluePin, LOW);
      tone(BUZZER_PIN, 1000);
      warningMode = true;
    } else {
      noTone(BUZZER_PIN);
      digitalWrite(LED_PINm, LOW);
      warningMode = false;
    }

    if (!warningMode) {
      int lightValue = analogRead(ldrPin);
      if (lightValue < 200) {
        digitalWrite(redPin, HIGH);
        digitalWrite(greenPin, HIGH);
        digitalWrite(bluePin, HIGH);
      } else {
        digitalWrite(redPin, LOW);
        digitalWrite(greenPin, LOW);
        digitalWrite(bluePin, LOW);
      }
    }
  }

  server.handleClient();
}