#include <SimpleDHT.h>
#include <ESP8266WiFi.h>       // Wi-Fi library (built into ESP8266 Arduino core)
#include <ESP8266WebServer.h>  // Web server library (built into ESP8266 Arduino core)

// ---- Wi-Fi credentials ----
// Replace these with your actual network name and password before uploading
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ---- Web server listening on port 80 ----
ESP8266WebServer server(80);

// Tracks whether Wi-Fi connected successfully; used to guard web server calls
bool wifiConnected = false;

// Define pins
#define TEMP_HUMIDITY_PIN D1  // Temperature and Humidity sensor
#define PHOTOCELL_PIN A0      // Photocell (Light sensor)
#define TRIG_PIN D3           // Ultrasonic TRIG
#define ECHO_PIN D2           // Ultrasonic ECHO
#define BUZZER_PIN D0         // Buzzer
#define LED_PIN D8            // LED

// Motor control pins
const int MOTOR_PIN = D5;      // Speed control
const int DIRA = D6;           // Direction A
const int DIRB = D7;           // Direction B

SimpleDHT11 dht11;

// ---- Sensor variables ----
byte temperature = 0;
byte humidity = 0;
int lightLevel = 0;
long distance = 0;

// ---- Actuator state tracking (used by the dashboard) ----
bool fanState     = false;  // true = fan is ON
bool ledState     = false;  // true = LED is ON
bool buzzerActive = false;  // true = buzzer is currently triggered

// ---- Manual override flags ----
// When true, web commands control the actuator instead of automatic sensor logic.
// They stay in manual mode until the board is reset (simple and beginner-friendly).
bool fanManual = false;
bool ledManual = false;

// Function prototypes
void readSensors();
void controlActuators();
long measureDistance();
void turnOffMotor();
void controlMotor(bool on, int speed, bool direction);
// Web handler prototypes
void handleRoot();
void handleSensorsAPI();
void handleFanOn();
void handleFanOff();
void handleLedOn();
void handleLedOff();

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);

  // Sensor pin modes
  pinMode(PHOTOCELL_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Actuator pin modes
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // Motor pin modes
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(DIRA, OUTPUT);
  pinMode(DIRB, OUTPUT);

  // Initial states
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  turnOffMotor();

  // ---- Connect to Wi-Fi ----
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {  // 30 x 500ms = 15s timeout
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi connection failed. Running without web dashboard.");
  } else {
    wifiConnected = true;
    Serial.print("Wi-Fi connected! Dashboard available at: http://");
    Serial.println(WiFi.localIP());

    // ---- Register web server routes ----
    server.on("/",            handleRoot);        // HTML dashboard
    server.on("/api/sensors", handleSensorsAPI);  // JSON sensor data API
    server.on("/fan/on",      handleFanOn);       // Turn fan ON (manual override)
    server.on("/fan/off",     handleFanOff);      // Turn fan OFF (manual override)
    server.on("/led/on",      handleLedOn);       // Turn LED ON (manual override)
    server.on("/led/off",     handleLedOff);      // Turn LED OFF (manual override)

    // Start the web server
    server.begin();
    Serial.println("Web server started.");
  }
}

void loop() {
  // Handle any incoming web requests first (non-blocking)
  if (wifiConnected) {
    server.handleClient();
  }

  // Step 1: Read all sensors
  readSensors();

  // Step 2: Actuate devices based on sensor data (respects manual overrides)
  controlActuators();

  // Step 3: Add delay for stable reading
  delay(1000);
}

void readSensors() {
  // Measure temperature and humidity
  dht11.read(TEMP_HUMIDITY_PIN, &temperature, &humidity, NULL);

  // Read photocell
  lightLevel = analogRead(PHOTOCELL_PIN);

  // Measure distance using ultrasonic sensor
  distance = measureDistance();

  // Debugging output
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" C, Humidity: ");
  Serial.print(humidity);
  Serial.print(" %, Light: ");
  Serial.print(lightLevel);
  Serial.print(", Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
}

void controlActuators() {
  // ---- Buzzer: motion detection (always automatic, no manual override) ----
  if (distance > 0 && distance < 15) {
    buzzerActive = true;
    // Start a beeping pattern for the buzzer to create an alarm-like sound
    for (int i = 0; i < 10; i++) {  // Repeat 10 times
      tone(BUZZER_PIN, 1000);  // Set the frequency to 1000Hz
      delay(100);               // Sound on for 100ms
      noTone(BUZZER_PIN);       // Turn off buzzer
      delay(100);               // Sound off for 100ms
    }
  } else {
    buzzerActive = false;
    noTone(BUZZER_PIN);  // Ensure buzzer is off if the distance is greater than 15cm
  }

  // ---- LED: light control (skipped when manual override is active) ----
  if (!ledManual) {
    if (lightLevel < 500) {
      digitalWrite(LED_PIN, HIGH);  // Turn on LED
      ledState = true;
    } else {
      digitalWrite(LED_PIN, LOW);   // Turn off LED
      ledState = false;
    }
  }

  // ---- Fan: temperature control (skipped when manual override is active) ----
  if (!fanManual) {
    if (temperature > 22) {
      controlMotor(true, 1023, true);  // Turn on fan at full speed in one direction
      fanState = true;
      Serial.println("Fan ON");
    } else {
      turnOffMotor();  // Turn off fan
      fanState = false;
      Serial.println("Fan OFF");
    }
  }
}

void turnOffMotor() {
  digitalWrite(MOTOR_PIN, LOW);
  digitalWrite(DIRA, LOW);
  digitalWrite(DIRB, LOW);
}

void controlMotor(bool on, int speed, bool direction) {
  if (on) {
    // Set motor speed
    analogWrite(MOTOR_PIN, speed);

    // Set motor direction
    if (direction) {
      digitalWrite(DIRA, HIGH);
      digitalWrite(DIRB, LOW);
    } else {
      digitalWrite(DIRA, LOW);
      digitalWrite(DIRB, HIGH);
    }
  } else {
    turnOffMotor();
  }
}

long measureDistance() {
  // Send trigger pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure echo pulse duration
  long duration = pulseIn(ECHO_PIN, HIGH);

  // Debugging: Check the duration
  Serial.print("Duration: ");
  Serial.println(duration);

  if (duration == 0) {
    return 0;  // Return 0 if no echo is detected
  }

  // Calculate distance in cm
  long distance = duration * 0.034 / 2;

  return distance;
}

// ============================================================
// Web Server Handlers
// ============================================================

// HTML dashboard stored in flash (PROGMEM) to avoid consuming heap RAM
const char HTML_PAGE[] PROGMEM =
  "<!DOCTYPE html>"
  "<html lang='en'>"
  "<head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<title>Env &amp; Security System</title>"
    "<style>"
      "* { box-sizing: border-box; margin: 0; padding: 0; }"
      "body { font-family: Arial, sans-serif; background: #1a1a2e; color: #e0e0e0; min-height: 100vh; padding: 20px; }"
      "h1 { text-align: center; color: #00d4ff; margin-bottom: 8px; font-size: 1.6em; }"
      ".subtitle { text-align: center; color: #888; margin-bottom: 24px; font-size: 0.9em; }"
      ".grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); gap: 16px; max-width: 700px; margin: 0 auto 24px; }"
      ".card { background: #16213e; border-radius: 12px; padding: 20px 16px; text-align: center; border: 1px solid #0f3460; }"
      ".card .label { font-size: 0.8em; color: #aaa; margin-bottom: 8px; text-transform: uppercase; letter-spacing: 1px; }"
      ".card .value { font-size: 1.8em; font-weight: bold; color: #00d4ff; }"
      ".card .unit  { font-size: 0.75em; color: #888; }"
      ".card.on  .value { color: #4caf50; }"
      ".card.off .value { color: #f44336; }"
      ".controls { max-width: 700px; margin: 0 auto; }"
      ".controls h2 { color: #00d4ff; margin-bottom: 12px; font-size: 1.1em; }"
      ".btn-row { display: flex; gap: 10px; flex-wrap: wrap; margin-bottom: 12px; }"
      ".btn { padding: 10px 20px; border: none; border-radius: 8px; cursor: pointer; font-size: 0.9em; font-weight: bold; transition: opacity .2s; }"
      ".btn:hover { opacity: 0.8; }"
      ".btn-on  { background: #4caf50; color: #fff; }"
      ".btn-off { background: #f44336; color: #fff; }"
      ".status-bar { text-align: center; color: #555; font-size: 0.75em; margin-top: 20px; }"
    "</style>"
  "</head>"
  "<body>"
    "<h1>&#127968; Env &amp; Security Dashboard</h1>"
    "<p class='subtitle'>Auto-refreshes every 3 seconds</p>"
    "<div class='grid'>"
      "<div class='card'><div class='label'>Temperature</div><div class='value' id='temp'>--</div><div class='unit'>&#176;C</div></div>"
      "<div class='card'><div class='label'>Humidity</div><div class='value' id='hum'>--</div><div class='unit'>%</div></div>"
      "<div class='card'><div class='label'>Light Level</div><div class='value' id='light'>--</div><div class='unit'>raw</div></div>"
      "<div class='card'><div class='label'>Distance</div><div class='value' id='dist'>--</div><div class='unit'>cm</div></div>"
      "<div class='card' id='fanCard'><div class='label'>Fan</div><div class='value' id='fan'>--</div></div>"
      "<div class='card' id='ledCard'><div class='label'>LED</div><div class='value' id='led'>--</div></div>"
      "<div class='card' id='buzCard'><div class='label'>Buzzer</div><div class='value' id='buz'>--</div></div>"
    "</div>"
    "<div class='controls'>"
      "<h2>Remote Control</h2>"
      "<div class='btn-row'>"
        "<button class='btn btn-on'  onclick=\"sendCmd('/fan/on')\">&#128168; Fan ON</button>"
        "<button class='btn btn-off' onclick=\"sendCmd('/fan/off')\">&#128168; Fan OFF</button>"
      "</div>"
      "<div class='btn-row'>"
        "<button class='btn btn-on'  onclick=\"sendCmd('/led/on')\">&#128161; LED ON</button>"
        "<button class='btn btn-off' onclick=\"sendCmd('/led/off')\">&#128161; LED OFF</button>"
      "</div>"
    "</div>"
    "<p class='status-bar' id='statusBar'>Loading...</p>"
    "<script>"
      // Fetch sensor data from /api/sensors and update the cards on the page
      "function updateDashboard() {"
        "fetch('/api/sensors')"
          ".then(function(res) { return res.json(); })"
          ".then(function(d) {"
            "document.getElementById('temp').textContent  = d.temperature;"
            "document.getElementById('hum').textContent   = d.humidity;"
            "document.getElementById('light').textContent = d.light;"
            "document.getElementById('dist').textContent  = d.distance;"
            "setCard('fan', d.fan, 'fanCard');"
            "setCard('led', d.led, 'ledCard');"
            "setCard('buz', d.buzzer, 'buzCard');"
            "document.getElementById('statusBar').textContent = 'Last updated: ' + new Date().toLocaleTimeString();"
          "})"
          ".catch(function() {"
            "document.getElementById('statusBar').textContent = 'Connection error - retrying...';"
          "});"
      "}"
      // Update a card's displayed value and colour based on ON/OFF state
      "function setCard(valueId, isOn, cardId) {"
        "var el   = document.getElementById(valueId);"
        "var card = document.getElementById(cardId);"
        "el.textContent  = isOn ? 'ON' : 'OFF';"
        "card.className  = 'card ' + (isOn ? 'on' : 'off');"
      "}"
      // Send a control command to the board and refresh the dashboard immediately
      "function sendCmd(path) {"
        "fetch(path).then(function() { updateDashboard(); });"
      "}"
      // Load data immediately, then refresh every 3 seconds
      "updateDashboard();"
      "setInterval(updateDashboard, 3000);"
    "</script>"
  "</body>"
  "</html>";

// Serves the HTML dashboard at http://<device-IP>/
// HTML is read directly from flash (PROGMEM) — no heap allocation needed
void handleRoot() {
  server.send_P(200, "text/html", HTML_PAGE);
}

// Returns all sensor and actuator data as JSON at /api/sensors
void handleSensorsAPI() {
  // Build a simple JSON string (no extra library needed)
  // Pre-allocate ~120 bytes to avoid repeated heap reallocations
  String json;
  json.reserve(120);
  json = "{";
  json += "\"temperature\":"  + String(temperature)                    + ",";
  json += "\"humidity\":"     + String(humidity)                       + ",";
  json += "\"light\":"        + String(lightLevel)                     + ",";
  json += "\"distance\":"     + String(distance)                       + ",";
  json += "\"fan\":"          + String(fanState     ? "true" : "false") + ",";
  json += "\"led\":"          + String(ledState     ? "true" : "false") + ",";
  json += "\"buzzer\":"       + String(buzzerActive ? "true" : "false");
  json += "}";

  server.send(200, "application/json", json);
}

// Turn the fan ON via web (overrides automatic sensor-based control)
void handleFanOn() {
  fanManual = true;
  fanState  = true;
  controlMotor(true, 1023, true);
  Serial.println("Web: Fan ON (manual)");
  server.send(200, "application/json", "{\"fan\":true}");
}

// Turn the fan OFF via web
void handleFanOff() {
  fanManual = true;
  fanState  = false;
  turnOffMotor();
  Serial.println("Web: Fan OFF (manual)");
  server.send(200, "application/json", "{\"fan\":false}");
}

// Turn the LED ON via web (overrides automatic sensor-based control)
void handleLedOn() {
  ledManual = true;
  ledState  = true;
  digitalWrite(LED_PIN, HIGH);
  Serial.println("Web: LED ON (manual)");
  server.send(200, "application/json", "{\"led\":true}");
}

// Turn the LED OFF via web
void handleLedOff() {
  ledManual = true;
  ledState  = false;
  digitalWrite(LED_PIN, LOW);
  Serial.println("Web: LED OFF (manual)");
  server.send(200, "application/json", "{\"led\":false}");
}

