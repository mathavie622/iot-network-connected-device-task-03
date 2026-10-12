# Task 03: Network-Connected IoT Device

## Project Overview
This project uses an ESP32 and an HC-SR04 ultrasonic sensor to measure object distance and publish readings over Wi-Fi using MQTT.

## Objectives
- Measure distance using an ultrasonic sensor.
- Send sensor readings to an MQTT broker.
- Reconnect when Wi-Fi or MQTT connectivity is lost.
- Buffer readings while offline and publish them after reconnection.

## Components
- ESP32 development board
- HC-SR04 ultrasonic distance sensor
- LED
- 220-ohm resistor
- Jumper wires

## Pin Connections
| Component | ESP32 Pin |
|---|---|
| HC-SR04 TRIG | GPIO 5 |
| HC-SR04 ECHO | GPIO 18 |
| HC-SR04 VCC | 5V / VIN |
| HC-SR04 GND | GND |
| LED anode (+) | GPIO 2 through 220-ohm resistor |
| LED cathode (-) | GND |

## Software and Tools
- Arduino C++
- Wokwi ESP32 Simulator
- Wi-Fi
- MQTT
- PubSubClient library
- GitHub

## How It Works
1. The ESP32 connects to the configured Wi-Fi network.
2. It connects to the MQTT broker.
3. The ultrasonic sensor measures distance every five seconds.
4. Valid readings are stored in a circular buffer.
5. When MQTT is connected, buffered readings are published to the MQTT topic.
6. The device retries Wi-Fi and MQTT connections when necessary.

## MQTT Configuration
- Broker: `broker.hivemq.com`
- Port: `1883`
- Topic: `mathavi/task03/distance`

This is a public test broker. Use a unique topic for your own tests and do not send private information.

## Buffer Limitations
The device stores up to 20 readings in RAM. If the buffer becomes full, the oldest reading is removed. Buffered data is lost if the ESP32 restarts or loses power.

## How to Run
1. Open the project in the Wokwi ESP32 simulator.
2. Add the ESP32, HC-SR04 sensor and LED.
3. Connect the components according
4. 
