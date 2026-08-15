#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <ArduinoJson.h>

const char* ssid = "localhost";
const char* password = "MatKhauSieuCap";
const char* mqttServer = "192.168.1.34";
const int   mqttPort = 1883;

const char* topicValue    = "mmcl/nhom1/dht/value";
const char* topicDetected = "mmcl/nhom1/dht/detected";

const uint8_t BME_ADDR = 0x76;
const int LED_PIN = D5;

const unsigned long SEND_INTERVAL  = 5000;
const unsigned long BLINK_INTERVAL = 300;

Adafruit_BME680 bme;
WiFiClient   espClient;
PubSubClient client(espClient);

bool alarm     = false;
bool ledState  = false;
unsigned long lastSend  = 0;
unsigned long lastBlink = 0;

void onMessage(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Receive alert: ");
  Serial.println(message);

  StaticJsonDocument<128> doc;
  if (deserializeJson(doc, message)) {
    message.trim();
    message.toLowerCase();
    alarm = (message == "true" || message == "1");
  } else {
    alarm = doc["detected"] | false;
  }

  if (!alarm) {                 // het bat thuong thi tat han den
    digitalWrite(LED_PIN, LOW);
    ledState = false;
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT Broker...");
    if (client.connect("wemos-bme680-nhom1")) {
      Serial.println("Success!");
      client.subscribe(topicDetected);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" - retry in 5s");
      delay(5000);
    }
  }
}

void publishSensor() {
  if (!bme.performReading()) {
    Serial.println("Read BME680 failed, skip!");
    return;
  }

  StaticJsonDocument<192> doc;
  doc["temperature"] = bme.temperature;
  doc["humidity"]    = bme.humidity;
  doc["pressure"]    = bme.pressure / 100.0;        // hPa
  doc["gas"]         = bme.gas_resistance / 1000.0; // kOhm

  char payload[192];
  serializeJson(doc, payload);
  client.publish(topicValue, payload);

  Serial.print("Publish: ");
  Serial.println(payload);
}

// chop den kieu khong chan de client.loop() van duoc goi lien tuc
void handleBlink() {
  if (!alarm) return;
  if (millis() - lastBlink < BLINK_INTERVAL) return;

  lastBlink = millis();
  ledState  = !ledState;
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Wire.begin(D2, D1);
  if (!bme.begin(BME_ADDR)) {
    Serial.println("Not found BME680, check board again!");
    while (true) delay(1000);
  }

  // cau hinh do theo khuyen nghi cua thu vien
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);   // dot 320 do C trong 150ms

  WiFi.begin(ssid, password);
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqttServer, mqttPort);
  client.setCallback(onMessage);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  if (millis() - lastSend >= SEND_INTERVAL) {
    lastSend = millis();
    publishSensor();
  }

  handleBlink();
}
