#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <PubSubClient.h>   

/* WIFI CONFIG */
const char *ssid = "HUAWEI-2.4G-6qSe";
const char *password = "wyeRy2eZ";


/** MQTT CONFIG */
const char* mqtt_server = "192.168.0.115";   
WiFiClient espClient;
PubSubClient client(espClient);

/* HARDWARE PINS */
const int RELAY1_PIN = 27;
const int RELAY2_PIN = 26;
const int STOVE_PIN  = 23;
const int SERVO1_PIN = 18;
const int SERVO2_PIN = 19;

Servo servo1, servo2;
WebServer server(80);

/******************** STATE MACHINE ********************/
enum State {
  IDLE,
  MOTOR1_RUNNING,
  MOTOR2_RUNNING,
  SERVO_RUNNING,
  STOVE_RUNNING,
  COMPLETED
};
State currentState = IDLE;

/******************** TIMERS & DATA ********************/
unsigned long motor1EndTime = 0;
unsigned long motor2EndTime = 0;
unsigned long stoveEndTime  = 0;

int motor1Duration = 0;
int motor2Duration = 0;
int servo1Rotations = 0;
int servo2Rotations = 0;
int stoveDuration  = 0;

/* MQTT STATUS PUBLISH */
void publishStatus(String state, long remaining)
{
  client.publish("tea/status/state", state.c_str());

  String json = "{\"state\":\"" + state +
                "\",\"remaining\":" + String(remaining) +
                ",\"motor1\":" + String(motor1Duration) +
                ",\"motor2\":" + String(motor2Duration) +
                ",\"servo1\":" + String(servo1Rotations) +
                ",\"servo2\":" + String(servo2Rotations) +
                ",\"stove\":"  + String(stoveDuration) + "}";

  client.publish("tea/status/full", json.c_str());
}

/* MQTT CALLBACK ***/
void mqttCallback(char* topic, byte* payload, unsigned int length)
{
  String message;
  for (int i = 0; i < length; i++)
    message += (char)payload[i];

  if (String(topic) == "tea/cmd/stop")
  {
    Serial.println("MQTT STOP RECEIVED");
    if (currentState == MOTOR1_RUNNING) digitalWrite(RELAY1_PIN, HIGH);
    if (currentState == MOTOR2_RUNNING) digitalWrite(RELAY2_PIN, HIGH);
    if (currentState == STOVE_RUNNING)  digitalWrite(STOVE_PIN, LOW);

    servo1.write(0);
    servo2.write(0);

    currentState = IDLE;
    publishStatus("idle", 0);
  }
}

/** MQTT RECONNECT ***/

void reconnectMQTT()
{
  while (!client.connected())
  {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32_TeaMaker"))
    {
      Serial.println("connected!");
      client.subscribe("tea/cmd/#");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying...");
      delay(2000);
    }
  }
}

/***  MACHINE FUNCTIONS ****/
void startMotor1(int seconds)
{
  digitalWrite(RELAY1_PIN, LOW);
  motor1EndTime = millis() + seconds * 1000;
  currentState = MOTOR1_RUNNING;
  publishStatus("motor1", seconds);
}

void stopMotor1()
{
  digitalWrite(RELAY1_PIN, HIGH);
}

void startMotor2(int seconds)
{
  digitalWrite(RELAY2_PIN, LOW);
  motor2EndTime = millis() + seconds * 1000;
  currentState = MOTOR2_RUNNING;
  publishStatus("motor2", seconds);
}

void stopMotor2()
{
  digitalWrite(RELAY2_PIN, HIGH);
}

void startServos(int r1, int r2)
{
  currentState = SERVO_RUNNING;
  publishStatus("servo", 0);

  int maxRot = max(r1, r2);
  for (int i = 0; i < maxRot; i++)
  {
    if (i < r1) servo1.write(90);
    if (i < r2) servo2.write(90);
    delay(500);
    servo1.write(0);
    servo2.write(0);
    delay(500);
  }
}

void startStove(int seconds)
{
  digitalWrite(STOVE_PIN, HIGH);
  stoveEndTime = millis() + seconds * 1000;
  currentState = STOVE_RUNNING;
  publishStatus("stove", seconds);
}

void stopStove()
{
  digitalWrite(STOVE_PIN, LOW);
}

/******************** HTTP HANDLERS ********************/
void handleStart()
{
  motor1Duration = server.arg("motor1").toInt();
  motor2Duration = server.arg("motor2").toInt();
  servo1Rotations = server.arg("servo1").toInt();
  servo2Rotations = server.arg("servo2").toInt();
  stoveDuration  = server.arg("stove").toInt();

  startMotor1(motor1Duration);
  server.send(200, "application/json", "{\"status\":\"started\"}");
}

void handleStop()
{
  mqttCallback((char*)"tea/cmd/stop", NULL, 0);
  server.send(200, "application/json", "{\"status\":\"stopped\"}");
}

/******************** SETUP ********************/
void setup()
{
  Serial.begin(115200);
  delay(2000);
  Serial.println("ESP32 BOOTED");

  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(STOVE_PIN, OUTPUT);

  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);
  digitalWrite(STOVE_PIN, LOW);

  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);

  Serial.println("Connecting to WiFi...");
WiFi.begin(ssid, password);

unsigned long startAttemptTime = millis();

while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000)
{
  delay(500);
  Serial.print(".");
}

Serial.println();

if (WiFi.status() == WL_CONNECTED)
{
  Serial.println("WiFi connected!");
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());
}
else
{
  Serial.println("WiFi FAILED to connect");
}
client.setServer(mqtt_server, 1883);
client.setCallback(mqttCallback);



  // WiFi.config(local_IP, gateway, subnet);
  // WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) delay(500);
//   Serial.println("Connecting to WiFi...");
// WiFi.begin(ssid, password);

// while (WiFi.status() != WL_CONNECTED)
// {
//   delay(500);
//   Serial.print(".");
// }

// Serial.println();
// Serial.println("WiFi connected!");
// Serial.print("ESP32 IP address: ");
// Serial.println(WiFi.localIP());


//   client.setServer(mqtt_server, 1883);
//   client.setCallback(mqttCallback);

  server.on("/start", HTTP_POST, handleStart);
  server.on("/stop",  HTTP_POST, handleStop);
  server.begin();

  // publishStatus("idle", 0);
}

/******************** LOOP ********************/
void loop()
{
  static bool sentOnce = false;

if (client.connected() && !sentOnce)
{
  publishStatus("idle", 0);
  sentOnce = true;
}

  server.handleClient();

  if (!client.connected()) reconnectMQTT();
  client.loop();

  if (currentState == MOTOR1_RUNNING && millis() >= motor1EndTime)
  {
    stopMotor1();
    startMotor2(motor2Duration);
  }

  if (currentState == MOTOR2_RUNNING && millis() >= motor2EndTime)
  {
    stopMotor2();
    startServos(servo1Rotations, servo2Rotations);
    startStove(stoveDuration);
  }

  if (currentState == STOVE_RUNNING && millis() >= stoveEndTime)
  {
    stopStove();
    currentState = COMPLETED;
    publishStatus("completed", 0);
  }
}


