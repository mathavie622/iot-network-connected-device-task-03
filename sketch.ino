#include <WiFi.h>
#include <PubSubClient.h>

// Wokwi virtual Wi-Fi
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// Public MQTT test broker
const char* MQTT_SERVER = "broker.hivemq.com";
const int MQTT_PORT = 1883;

// Use your own unique topic name
const char* MQTT_TOPIC = "mathavi/task03/distance";

const int TRIG_PIN = 5;
const int ECHO_PIN = 18;
const int LED_PIN = 2;

const unsigned long SENSOR_INTERVAL = 5000;
const unsigned long WIFI_RETRY_INTERVAL = 10000;
const unsigned long MQTT_RETRY_INTERVAL = 5000;

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

// Store readings while offline
struct Reading {
  float distance;
  unsigned long id;
};

const int BUFFER_SIZE = 20;
Reading bufferData[BUFFER_SIZE];

int head = 0;
int tail = 0;
int bufferedCount = 0;

unsigned long readingId = 0;
unsigned long lastSensorTime = 0;
unsigned long lastWiFiAttempt = 0;
unsigned long lastMqttAttempt = 0;

void saveReading(float distance) {
  if (bufferedCount == BUFFER_SIZE) {
    // Keep the newest data if buffer is full
    head = (head + 1) % BUFFER_SIZE;
    bufferedCount--;
    Serial.println("Buffer full: oldest reading removed");
  }

  bufferData[tail].distance = distance;
  bufferData[tail].id = ++readingId;

  tail = (tail + 1) % BUFFER_SIZE;
  bufferedCount++;

  Serial.print("Reading buffered. Pending: ");
  Serial.println(bufferedCount);
}

void connectWiFi() {
  unsigned long now = millis();

  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  if (now - lastWiFiAttempt >= WIFI_RETRY_INTERVAL) {
    lastWiFiAttempt = now;
    Serial.println("Attempting Wi-Fi connection...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);
  }
}

void connectMQTT() {
  unsigned long now = millis();

  if (WiFi.status() != WL_CONNECTED || mqtt.connected()) {
    return;
  }

  if (now - lastMqttAttempt >= MQTT_RETRY_INTERVAL) {
    lastMqttAttempt = now;

    String clientId = "MathaviESP32-" +
                      String((uint32_t)ESP.getEfuseMac(), HEX);

    Serial.println("Attempting MQTT connection...");

    if (mqtt.connect(clientId.c_str())) {
      Serial.println("MQTT connected");
    } else {
      Serial.print("MQTT failed, state: ");
      Serial.println(mqtt.state());
    }
  }
}

float readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return -1;
  }

  return duration * 0.0343 / 2.0;
}

void publishBufferedReading() {
  if (!mqtt.connected() || bufferedCount == 0) {
    return;
  }

  Reading r = bufferData[head];

  char payload[100];
  snprintf(payload, sizeof(payload),
           "{\"id\":%lu,\"distance_cm\":%.2f}",
           r.id, r.distance);

  if (mqtt.publish(MQTT_TOPIC, payload)) {
    Serial.print("Published: ");
    Serial.println(payload);

    head = (head + 1) % BUFFER_SIZE;
    bufferedCount--;
  } else {
    Serial.println("Publish failed; reading kept in buffer");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(TRIG_PIN, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);

  Serial.println("Task 03 IoT device starting");
}

void loop() {
  connectWiFi();
  connectMQTT();

  if (WiFi.status() == WL_CONNECTED && mqtt.connected()) {
    mqtt.loop();
  }

  unsigned long now = millis();

  if (now - lastSensorTime >= SENSOR_INTERVAL) {
    lastSensorTime = now;

    float distance = readDistance();

    if (distance >= 0) {
      Serial.print("Distance: ");
      Serial.print(distance);
      Serial.println(" cm");

      digitalWrite(LED_PIN, distance <= 20 ? HIGH : LOW);
      saveReading(distance);
    } else {
      Serial.println("No valid sensor reading");
      digitalWrite(LED_PIN, LOW);
    }
  }

  // Send at most one queued reading per loop pass
  publishBufferedReading();
}
