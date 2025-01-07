#include <SimpleDHT.h>

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

// Variables
byte temperature = 0.0;
byte humidity = 0.0;
int lightLevel = 0;
long distance = 0;

// Function prototypes
void readSensors();
void controlActuators();
long measureDistance();
void turnOffMotor();
void controlMotor(bool on, int speed, bool direction);

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
}

void loop() {
  // Step 1: Read all sensors
  readSensors();

  // Step 2: Actuate devices based on sensor data
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
  // Motion detection with ultrasonic sensor
  if (distance > 0 && distance < 15) {
    // Start a beeping pattern for the buzzer to create an alarm-like sound
    for (int i = 0; i < 10; i++) {  // Repeat 10 times
      tone(BUZZER_PIN, 1000);  // Set the frequency to 1000Hz
      delay(100);               // Sound on for 100ms
      noTone(BUZZER_PIN);       // Turn off buzzer
      delay(100);               // Sound off for 100ms
    }
  } else {
    noTone(BUZZER_PIN);  // Ensure buzzer is off if the distance is greater than 15cm
  }

  // Light control using photocell
  if (lightLevel < 500) { 
    digitalWrite(LED_PIN, HIGH); // Turn on LED
  } else {
    digitalWrite(LED_PIN, LOW); // Turn off LED
  }

  // Temperature control for fan (motor)
  if (temperature > 22) { 
    controlMotor(true, 1023, true); // Turn on fan at full speed in one direction
    Serial.println("Fan ON");
  } else {
    turnOffMotor(); // Turn off fan
    Serial.println("Fan OFF");
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

