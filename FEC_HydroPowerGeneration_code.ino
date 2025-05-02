#include <stdio.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <string.h>
#include <Wire.h>

#include "DHT.h"
#include "DFRobot_INA219.h"

#define DHTPIN A0
#define DHTTYPE DHT11

/////////////////////////////////////////////////////

// WIFI CONFIGURATION

const char* ssid = /* Enter WiFi name here as a string */;
const char* password = /* Enter WiFi password here as a string */;

/////////////////////////////////////////////////////

// Temperature Sensor Input
DHT dht(DHTPIN, DHTTYPE);

// IR Sensor Input
const int hallSensorPin = 5;

volatile int pulseCount = 0;
unsigned long lastPulseTime = 0;
const unsigned long debounceMicros = 4000;  // 4ms debounce for fast disks

unsigned long lastRPMTime = 0;
const unsigned long interval = 1000;  // 1 second in ms

float ina219Reading_mA = 1000;
float extMeterReading_mA = 1000;

// Power Sensor Input
DFRobot_INA219_IIC ina219(&Wire, INA219_I2C_ADDRESS4);

const char* mqtt_server = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);

void IRAM_ATTR handleMagnet() {
  unsigned long now = micros();
  if (now - lastPulseTime > debounceMicros) {
    pulseCount++;
    lastPulseTime = now;
  }
}

// Initialize sensors, establish WiFi connection
void setup() {

  Serial.begin(115200);

  // RPM Initialize
  pinMode(hallSensorPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(hallSensorPin), handleMagnet, RISING);

  // Temperature Initialize
  dht.begin();

  // Power Monitor Initialize
  while (ina219.begin() != true) {
    Serial.println("INA219 initialization failed. Retrying...");
    delay(100);
  }

  // Power Monitor Calibration
  ina219.linearCalibrate(ina219Reading_mA, extMeterReading_mA);
  ina219.setBRNG(DFRobot_INA219::eIna219BusVolRange_32V);

  // Connect to Internet
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Connection Failed. Retrying...");
    delay(1000);
  }

  Serial.println("WiFi Successfully Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);
  client.setBufferSize(512);
  client.setSocketTimeout(2);  // Timeout in seconds — try 2 or 3

  delay(200);
}

void loop() {

  unsigned long currentTime = millis();

  if (currentTime - lastRPMTime >= interval) {
    noInterrupts();  // prevent conflict while reading
    int count = pulseCount;
    pulseCount = 0;
    interrupts();


    // Get temperature value
    float temp = dht.readTemperature();
    Serial.print("Temperature: ");
    Serial.println(temp);

    // Get volt value
    float volt = ina219.getBusVoltage_V();
    if (volt < 0) {
      volt = volt + 33;
    }
    Serial.print("BusVoltage: ");
    Serial.print(volt);
    Serial.println("V");

    // Get current value
    float current = ina219.getCurrent_mA() / 1000;
    Serial.print("Current: ");
    Serial.print(current);
    Serial.println("A");

    // Get power value
    float power = ina219.getPower_mW() / 1000;
    Serial.print("Power: ");
    Serial.print(power, 1);
    Serial.println("W");

    // 2 magnets = 2 pulses per revolution
    float rpm = (count / 2.0) * 60.0;
    Serial.print("RPM: ");
    Serial.println(rpm);

    lastRPMTime = currentTime;

    // Convert sensor data to character arrays for MQTT publishing
    char tempc[20], voltc[20], currentc[20], powerc[20], rpmc[20];
    dtostrf(temp, 2, 2, tempc);
    dtostrf(volt, 2, 2, voltc);
    dtostrf(current, 2, 2, currentc);
    dtostrf(power, 2, 2, powerc);
    dtostrf(rpm, 2, 2, rpmc);

    // Publish the data via MQTT
    if (client.connected()) {
      client.publish("rpm_monitor", rpmc);
      client.publish("temperature_monitor", tempc);
      client.publish("volt_monitor", voltc);
      client.publish("current_monitor", currentc);
      client.publish("power_monitor", powerc);
    }

    // Attempt to reconnect to MQTT if disconnected
    if (!client.connected()) {
      reconnect();
    }

    // Keep the MQTT connection alive
    client.loop();
  }
}

void reconnect() {
  int retryCount = 0;
  const int maxRetries = 5;

  while (!client.connected() && retryCount < maxRetries) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESPCode")) {
      Serial.println("MQTT Connected.");
      // client.subscribe("your_topic");  // For future features if needed
    } else {
      Serial.print(" Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 1 second...");
      retryCount++;
      delay(1000);
    }
  }
}
