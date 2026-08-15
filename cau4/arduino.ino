#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "localhost";
const char* password = "MatKhauSieuCap";
const char* mqttServer = "192.168.1.34";
const int mqttPort = 1883;
const char* topicLed = "mmcl/nhom1/led";

const int LED_PIN = D5;

WiFiClient   espClient;
PubSubClient client(espClient);

void onMessage(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Receive from ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(message);

  message.trim();
  message.toUpperCase();

  if (message == "ON" || message == "1") {
    digitalWrite(LED_PIN, HIGH);
  } else if (message == "OFF" || message == "0") {
    digitalWrite(LED_PIN, LOW);
  } else {
    Serial.println("Invaid state -> Skip.");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to broker...");
    if (client.connect("wemos-nhom1")) {
      Serial.println("Done!");
      client.subscribe(topicLed);
      Serial.print("Subscribed on ");
      Serial.println(topicLed);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" - retry again in 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

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
}
