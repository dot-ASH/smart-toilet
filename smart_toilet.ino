#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Stepper.h>
#include <Servo.h>
#include "index.h"
#include "pass.h"

#define SERVO_PIN D1
#define FLUSH_SERVO_PIN D2
#define TRIG_PIN D3
#define ECHO_PIN D4
#define TOLERANCE 10
#define DISTANCE_THRESHOLD 15
#define CLEAR 0
#define SPEED 60
#define stepsPerRevolution 2038

Stepper myStepper = Stepper(stepsPerRevolution, D5, D6, D7, D8);
Servo servo;
Servo flushServo;

int pos = 0;
int previous = 0;
float duration_us, distance_cm;
int count;
int quickCount = 0;
bool isQuickFlush = false;
int flushState = 0;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// NOTIFY THE CLIENT
void notifyClients() {
  ws.textAll(String(flushState));
}

// HANDSHAKES AND ALERT
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char *)data, "toggle") == 0) {
      flushState = !flushState;
      notifyClients();
    }
  }
}

// EVENT FUNCTION
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

// INITIALIZE THE SOCKET
void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

// PASSING VARIABLE
String processor(const String &var) {
  Serial.println(var);
  if (var == "STATE") {
    if (!flushState)
      return "You haven't flushed the toilet in a while";
  } else
    return "Recently you've flushed the toilet";
  return String();
}

// LED STATE
void setIndicatorLED(int led, int state) {
  digitalWrite(led, state);
}

// WIFI
void connectWifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
}

// SETUP
void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  flushServo.attach(FLUSH_SERVO_PIN);
  flushServo.write(CLEAR);
  servo.attach(SERVO_PIN);
  pos = 0;
  servo.write(pos);

  Serial.begin(115200);
  turnOffRoller();
  connectWifi();
  initWebSocket();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.begin();
}

// FLUSH
void flush() {
  delay(1000);
  Serial.println("Toilet Flushed");
  count = CLEAR;
  flushServo.write(180);
  delay(2000);
  for (int pos = 180; pos >= 0; pos -= 5) {
    flushServo.write(pos);
    delay(60);
  }
}

void lidOn() {
  for (int i = 0; i < 120; i += 5) {
    servo.write(i);
    delay(SPEED);
    pos = 120;
  }
}

void lidOff() {
  for (int i = 120; i > 0; i -= 5) {
    servo.write(i);
    delay(SPEED);
    pos = 0;
  }
  flushState = 0;
}

void turnTheRoller() {

  myStepper.setSpeed(10);
  myStepper.step(stepsPerRevolution);
  myStepper.setSpeed(10);
  myStepper.step(stepsPerRevolution);
  myStepper.setSpeed(10);
  myStepper.step(stepsPerRevolution);
  delay(1000);
}

void turnOffRoller() {
  digitalWrite(D5, LOW);
  digitalWrite(D6, LOW);
  digitalWrite(D7, LOW);
  digitalWrite(D8, LOW);
}

// LOOP
void loop() {
  ws.cleanupClients();
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration_us = pulseIn(ECHO_PIN, HIGH);
  distance_cm = (duration_us * .0343) / 2;
  Serial.print("Distance: ");
  Serial.println(distance_cm);
  if (distance_cm < DISTANCE_THRESHOLD && distance_cm != 0 && pos == 0) {
    lidOn();
  } else if (distance_cm > DISTANCE_THRESHOLD && pos == 120) {
    delay(1000);
    flush();
    delay(1000);
    turnTheRoller();
    turnOffRoller();
    lidOff();
  }
  if (flushState == 1) {
    lidOn();
    delay(1000);
    flush();
    delay(1000);
    turnTheRoller();
    turnOffRoller();
    lidOff();
  }
  delay(500);
}