/*
  Smart Energy Meter - ESP32 + PZEM-004T v3 + EMQX
  Libraries:
    - PZEM004Tv30 (mandate/olehs)
    - PubSubClient (Nick O'Leary)
    - ArduinoJson (Benoit Blanchon)

  IMPORTANT:
  - Fill in credentials locally; never commit real secrets.
  - Add the correct root CA certificate for your EMQX TLS endpoint.
  - Confirm UART pins and LED pins for your exact ESP32 board.
  - PZEM measures hazardous AC circuits. Do not perform mains wiring
    unless qualified and using an approved isolated/enclosed installation.
*/
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <PZEM004Tv30.h>
#include <ArduinoJson.h>

// ---------- LOCAL CONFIGURATION: replace placeholders ----------
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* MQTT_HOST = "YOUR_EMQX_HOST";
const uint16_t MQTT_TLS_PORT = 8883;
const char* MQTT_USERNAME = "ESP32_DEVICE_USERNAME";
const char* MQTT_PASSWORD = "ESP32_DEVICE_PASSWORD";
const char* DEVICE_ID = "energy-meter-esp32-01";

// Paste the trusted root CA certificate for your broker here.
// Do not use setInsecure() in a production installation.
static const char ROOT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
PASTE_THE_BROKER_CA_CERTIFICATE_HERE
-----END CERTIFICATE-----
)EOF";

// ---------- Hardware configuration ----------
static const int PZEM_RX_PIN = 16; // ESP32 RX2 receives from PZEM TX
static const int PZEM_TX_PIN = 17; // ESP32 TX2 sends to PZEM RX
static const int LED_RED_PIN = 25;
static const int LED_BLUE_PIN = 26;
static const int LED_GREEN_PIN = 27;

WiFiClientSecure tlsClient;
PubSubClient mqtt(tlsClient);
PZEM004Tv30 pzem(Serial2, PZEM_RX_PIN, PZEM_TX_PIN);

const char* TOPIC_TELEMETRY = "energy/meter/data";
const char* TOPIC_LED_STATE = "energy/led/state";
const char* TOPIC_DEVICE_STATUS = "energy/device/status";
const char* TOPIC_RED_SET = "energy/led/red/set";
const char* TOPIC_BLUE_SET = "energy/led/blue/set";
const char* TOPIC_GREEN_SET = "energy/led/green/set";

bool redOn = false, blueOn = false, greenOn = false;
unsigned long lastPublish = 0;
const unsigned long PUBLISH_INTERVAL_MS = 2000;

void publishLedState() {
  StaticJsonDocument<128> doc;
  doc["red"] = redOn;
  doc["blue"] = blueOn;
  doc["green"] = greenOn;
  char payload[128];
  size_t n = serializeJson(doc, payload, sizeof(payload));
  mqtt.publish(TOPIC_LED_STATE, payload, n, true); // retained reported state
}

void applyLed(const char* topic, const String& value) {
  bool on;
  if (value == "ON") on = true;
  else if (value == "OFF") on = false;
  else return; // allow-list only ON/OFF

  if (strcmp(topic, TOPIC_RED_SET) == 0) {
    redOn = on; digitalWrite(LED_RED_PIN, on ? HIGH : LOW);
  } else if (strcmp(topic, TOPIC_BLUE_SET) == 0) {
    blueOn = on; digitalWrite(LED_BLUE_PIN, on ? HIGH : LOW);
  } else if (strcmp(topic, TOPIC_GREEN_SET) == 0) {
    greenOn = on; digitalWrite(LED_GREEN_PIN, on ? HIGH : LOW);
  } else return;

  publishLedState(); // report actual output state after applying request
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String value;
  for (unsigned int i = 0; i < length; i++) value += (char)payload[i];
  value.trim();
  applyLed(topic, value);
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWi-Fi connected");
}

void connectMQTT() {
  while (!mqtt.connected()) {
    String clientId = String(DEVICE_ID) + "-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    // Last Will marks the device offline if the connection drops unexpectedly.
    if (mqtt.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD,
                     TOPIC_DEVICE_STATUS, 1, true, "offline")) {
      mqtt.publish(TOPIC_DEVICE_STATUS, "online", true);
      mqtt.subscribe(TOPIC_RED_SET);
      mqtt.subscribe(TOPIC_BLUE_SET);
      mqtt.subscribe(TOPIC_GREEN_SET);
      publishLedState();
      Serial.println("EMQX connected");
    } else {
      Serial.printf("MQTT connection failed, state=%d\n", mqtt.state());
      delay(3000);
    }
  }
}

void publishMeterData() {
  float voltage = pzem.voltage();
  float current = pzem.current();
  float power = pzem.power();
  float energy = pzem.energy();
  float frequency = pzem.frequency();
  float pf = pzem.pf();

  StaticJsonDocument<256> doc;
  if (!isnan(voltage)) doc["voltage"] = voltage;
  if (!isnan(current)) doc["current"] = current;
  if (!isnan(power)) doc["power"] = power;
  if (!isnan(energy)) doc["energy"] = energy;
  if (!isnan(frequency)) doc["frequency"] = frequency;
  if (!isnan(pf)) doc["pf"] = pf;
  char payload[256];
  size_t n = serializeJson(doc, payload, sizeof(payload));
  mqtt.publish(TOPIC_TELEMETRY, payload, n, false);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_RED_PIN, OUTPUT); pinMode(LED_BLUE_PIN, OUTPUT); pinMode(LED_GREEN_PIN, OUTPUT);
  digitalWrite(LED_RED_PIN, LOW); digitalWrite(LED_BLUE_PIN, LOW); digitalWrite(LED_GREEN_PIN, LOW);
  Serial2.begin(9600, SERIAL_8N1, PZEM_RX_PIN, PZEM_TX_PIN);
  connectWiFi();

  tlsClient.setCACert(ROOT_CA);
  mqtt.setServer(MQTT_HOST, MQTT_TLS_PORT);
  mqtt.setCallback(mqttCallback);
  mqtt.setBufferSize(512);
  connectMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  if (millis() - lastPublish >= PUBLISH_INTERVAL_MS) {
    lastPublish = millis();
    publishMeterData();
  }
}
