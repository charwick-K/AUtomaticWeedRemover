/*
====================================================
 Automatic Weed Remover - IoT Controller
 Hardware: ESP32 / Arduino + ESP8266
 Project: YOLOv9 based Smart Weed Removal Robot
 Author: <Your Name>

 Features:
 - Receives weed detection from AI system
 - Controls robot movement
 - Controls servo cutter arm
 - Controls relay for blade/sprayer
 - Sends telemetry (IoT)
====================================================
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <Servo.h>

// =======================
// WIFI CONFIG
// =======================
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// Server running Python YOLO
const char* serverURL = "http://192.168.1.100:5000/data";

// =======================
// MOTOR PINS (L298N)
// =======================
#define ENA 14
#define IN1 27
#define IN2 26
#define ENB 25
#define IN3 33
#define IN4 32

// =======================
// SERVO + RELAY
// =======================
#define SERVO_PIN 13
#define RELAY_PIN 12

Servo cutterServo;

// =======================
// GLOBAL VARIABLES
// =======================
int weedX = -1;
int weedY = -1;
int confidence = 0;

bool weedDetected = false;
unsigned long lastRequestTime = 0;

// =======================
// FUNCTION PROTOTYPES
// =======================
void connectWiFi();
void fetchWeedData();
void parseData(String payload);

void moveForward();
void moveBackward();
void turnLeft();
void turnRight();
void stopRobot();

void activateCutter();
void deactivateCutter();
void lowerArm();
void raiseArm();

void processWeedLogic();
void sendTelemetry(String msg);

// =======================
// SETUP
// =======================
void setup() {
  Serial.begin(115200);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  cutterServo.attach(SERVO_PIN);
  cutterServo.write(90);

  connectWiFi();

  Serial.println("Automatic Weed Remover IoT System Ready");
}

// =======================
// MAIN LOOP
// =======================
void loop() {

  if (millis() - lastRequestTime > 2000) {
    fetchWeedData();
    lastRequestTime = millis();
  }

  if (weedDetected) {
    processWeedLogic();
  } else {
    moveForward(); // keep searching
  }
}

// =======================
// WIFI
// =======================
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// =======================
// FETCH DATA FROM YOLO
// =======================
void fetchWeedData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      parseData(payload);
    }

    http.end();
  }
}

// Expected JSON:
// {"x":320,"y":240,"conf":85}
void parseData(String payload) {
  int xIndex = payload.indexOf("x");
  int yIndex = payload.indexOf("y");
  int cIndex = payload.indexOf("conf");

  weedX = payload.substring(xIndex + 3).toInt();
  weedY = payload.substring(yIndex + 3).toInt();
  confidence = payload.substring(cIndex + 6).toInt();

  if (confidence > 60) {
    weedDetected = true;
  } else {
    weedDetected = false;
  }

  Serial.print("Weed X: "); Serial.println(weedX);
  Serial.print("Weed Y: "); Serial.println(weedY);
  Serial.print("Confidence: "); Serial.println(confidence);
}

// =======================
// ROBOT MOVEMENT
// =======================
void moveForward() {
  analogWrite(ENA, 180);
  analogWrite(ENB, 180);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void moveBackward() {
  analogWrite(ENA, 180);
  analogWrite(ENB, 180);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turnLeft() {
  analogWrite(ENA, 120);
  analogWrite(ENB, 180);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void turnRight() {
  analogWrite(ENA, 180);
  analogWrite(ENB, 120);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void stopRobot() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

// =======================
// CUTTER MECHANISM
// =======================
void activateCutter() {
  digitalWrite(RELAY_PIN, HIGH);
}

void deactivateCutter() {
  digitalWrite(RELAY_PIN, LOW);
}

void lowerArm() {
  for (int i = 90; i <= 160; i++) {
    cutterServo.write(i);
    delay(10);
  }
}

void raiseArm() {
  for (int i = 160; i >= 90; i--) {
    cutterServo.write(i);
    delay(10);
  }
}

// =======================
// MAIN WEED LOGIC
// =======================
void processWeedLogic() {

  stopRobot();
  delay(500);

  // Centering logic
  if (weedX < 300) {
    turnLeft();
    delay(300);
  } else if (weedX > 340) {
    turnRight();
    delay(300);
  }

  stopRobot();
  delay(300);

  // Move closer
  moveForward();
  delay(800);
  stopRobot();

  // Cut weed
  lowerArm();
  activateCutter();
  delay(1500);
  deactivateCutter();
  raiseArm();

  sendTelemetry("Weed removed");

  weedDetected = false;
}

// =======================
// SEND DATA TO CLOUD
// =======================
void sendTelemetry(String msg) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://api.thingspeak.com/update?api_key=YOUR_KEY&field1=" + msg);
    http.GET();
    http.end();
  }
}
