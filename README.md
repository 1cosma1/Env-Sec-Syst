All-In-One: Environmental and Security System (IoT)


1. Introduction
   
The Smart Security and Environmental Monitoring System is an integrated IoT solution designed to enhance safety and comfort in residential or commercial spaces. By utilizing multiple sensors and actuators connected to an ESP8266 Wi-Fi microcontroller, the system provides real-time monitoring and remote control capabilities for environmental conditions and security threats — accessible from any device on the local network via a built-in web dashboard. This document provides a comprehensive overview of the project, including its components, implementation, and functionalities.


2. Objectives
   
•	Monitor environmental parameters such as temperature, humidity, and light intensity.

•	Detect motion using an ultrasonic sensor.

•	Provide immediate alerts using a buzzer.

•	Control a motorized fan for temperature regulation.

•	Activating the light of a LED based on low light conditions.

•	Integrate and automate all components into a cohesive system.

•	Enable remote monitoring of all sensor readings through a web dashboard.

•	Enable remote control of actuators (fan, LED) from any browser on the local network.


3. Components
   
3.1 Hardware

•	Microcontroller: ESP8266 (NodeMCU)

•	Sensors:

   -  DHT11 (Temperature and Humidity sensor)

   -	Ultrasonic Sensor (HC-SR04)
   
   -	Photocell (Light sensor)
   
•	Actuators:

   -	Buzzer
   
   -	LED (White)
   
   -	DC Motor (Fan)
   
•	Additional Components:

   -	Motor Driver Module (L293D)

   -	Breadboard power supply

   -	Resistors 

   -	Breadboard and connecting wires

3.2 Software

•	Arduino IDE

•	Libraries:

   -	SimpleDHT.h (for DHT11 sensor)

   -	ESP8266WiFi.h (built into ESP8266 Arduino core — no extra installation needed)

   -	ESP8266WebServer.h (built into ESP8266 Arduino core — no extra installation needed)


4. System Design
   
4.1 Circuit Schematic

The system integrates sensors and actuators into the ESP8266 microcontroller. The following connections are implemented:

•	DHT11: Data pin connected to GPIO D1.

•	Ultrasonic Sensor:

   -	TRIG pin to GPIO D2
   
   -	ECHO pin to GPIO D3
   
•	Photocell: Analog output connected to A0.

•	Buzzer: Connected to GPIO D0.

•	LED: Connected to GPIO D8.

•	Motor Driver:

   -	Motor speed pin to GPIO D5
   
   -	Direction pins (DIRA, DIRB) to GPIO D6 and D7, respectively.


5. Functional Description

5.1 Sensor Readings

•	Temperature and Humidity: Captured using the DHT11 sensor and displayed via the serial monitor.

•	Light Intensity: Measured using the photocell; triggers the LED in low-light conditions.

•	Motion Detection: Ultrasonic sensor detects objects within a range of 30 cm.

5.2 Actuator Control

•	Buzzer: Activated when the ultrasonic sensor detects motion.

•	LED: Turns on in low-light conditions.

•	Motor (Fan): Activates when the temperature exceeds 20°C.

5.3 System Workflow

1.	Sensors collect data and send it to the microcontroller.

2.	Based on sensor inputs, the microcontroller makes decisions to:

  	-	Activate the buzzer for security alerts.

  	-	Turn on/off the LED based on light intensity from the environment.

  	-	Turn on/off the fan for temperature regulation.

3.	Feedback is displayed via the serial monitor for debugging and monitoring.

4.	The built-in web server continuously serves a live dashboard — open http://<device-IP> in any browser on the same network to view all readings and control actuators remotely.


6. IoT / Web Features

6.1 Wi-Fi Setup

Before uploading the sketch, open mainprogram.ino and replace the placeholder credentials near the top of the file:

   const char* WIFI_SSID     = "YOUR_WIFI_SSID";
   const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

After uploading, open the Arduino IDE Serial Monitor (115200 baud). The ESP8266 will print its IP address once connected, for example:

   Wi-Fi connected! Dashboard available at: http://192.168.1.42

6.2 Web Dashboard

Open the printed URL in any browser on the same Wi-Fi network. The dashboard:

•	Displays all sensor readings (Temperature, Humidity, Light Level, Distance) as live cards.

•	Shows the current state of each actuator (Fan ON/OFF, LED ON/OFF, Buzzer triggered/clear).

•	Auto-refreshes every 3 seconds using a background JavaScript fetch so only the data updates, not the whole page.

•	Works on mobile devices (responsive grid layout, dark theme).

6.3 REST API

A lightweight JSON endpoint is available for integrations or custom dashboards:

   GET /api/sensors

Example response:

   {"temperature":24,"humidity":55,"light":312,"distance":42,"fan":true,"led":false,"buzzer":false}

6.4 Remote Control Endpoints

The following endpoints let you control actuators from any HTTP client (browser, curl, mobile app):

   GET /fan/on    — turn the fan ON  (manual override)
   GET /fan/off   — turn the fan OFF (manual override)
   GET /led/on    — turn the LED ON  (manual override)
   GET /led/off   — turn the LED OFF (manual override)

The dashboard includes buttons for each of these. When a manual command is sent, the automatic sensor-based logic for that actuator is bypassed until the board is reset.

7. Software Implementation

7.1 Code Overview

The system’s code is written in C++ using the Arduino IDE. It includes the following functions:

•	setup():
   
   -	Initializes sensor and actuator pins.
   
   -	Configures serial communication.

   -	Connects to Wi-Fi and starts the web server.

•	loop():
   
   -	Handles incoming web requests (server.handleClient()).

   -	Reads sensor data.
   
   -	Processes data to determine actuator states.
   
   -	Controls actuators based on logic.

•	Helper Functions:

   -	readSensors(): Reads data from all sensors.

   -	controlActuators(): Controls actuators based on sensor data (respects manual overrides).
   
   -	measureDistance(): Calculates the distance using the ultrasonic sensor.

   -	handleRoot(): Serves the HTML web dashboard.

   -	handleSensorsAPI(): Returns sensor data as JSON.

   -	handleFanOn/Off(), handleLedOn/Off(): Handle remote actuator control.

7.2 Key Features

•	Modular design for easy updates and expansion.

•	Debugging outputs via the serial monitor.

•	Real-time response to environmental changes and security threats.

•	Built-in web server for remote monitoring and control with no external services required.

8. Results

Upon deployment, the system successfully:

•	Detected motion and triggered the buzzer.

•	Turned on the LED in low-light conditions.

•	Activated the fan when the temperature exceeded the threshold.

•	Served a live web dashboard accessible from any browser on the local network.

•	Provided a REST API for sensor data and remote control of actuators.


9. Future Improvements

•	Integrate a rechargeable battery for power backup.

•	Use advanced sensors for more accurate readings.


10. Conclusion

The Smart Security and Environmental Monitoring System demonstrates how simple sensors and actuators can be combined with built-in Wi-Fi to create an efficient, automated IoT solution for safety and comfort. This project can serve as a foundation for more advanced IoT-based applications.

