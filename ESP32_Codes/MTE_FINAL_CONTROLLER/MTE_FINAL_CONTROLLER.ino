#include <RF24.h>

#define CE_PIN 4
#define CSN_PIN 5
RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

#define BUTTON_FORWARD 32
#define BUTTON_BACKWARD 12 
#define BUTTON_LEFT 27
#define BUTTON_RIGHT 14
#define BUTTON_WATER 13

int speedLevel = 0;
int servoAngle = 90;
bool waterState = false;

unsigned long lastPressTimeForward = 0;
unsigned long lastPressTimeBackward = 0;
unsigned long lastPressTimeLeft = 0;
unsigned long lastPressTimeRight = 0;
unsigned long lastPressTimeWater = 0;
const unsigned long debounceDelay = 200;
const unsigned long stopDelay = 3000;

#define led_pin 2


void setup() 
{
  Serial.begin(115200);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_LOW);
  radio.stopListening();
  
  pinMode(BUTTON_FORWARD, INPUT_PULLUP);
  pinMode(BUTTON_BACKWARD, INPUT_PULLUP);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_WATER, INPUT_PULLUP);

  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);

  Serial.println("ESP32 CONTROLLER READY");
}

void loop() {
  int8_t buffer[3];  
  
  delay(10);

  unsigned long currentTime = millis();

    if (digitalRead(BUTTON_FORWARD) == HIGH) 
    {
    if (speedLevel < 100) 
    {
      speedLevel += 5;
      lastPressTimeForward = currentTime;
    }
  } 
  else if (currentTime - lastPressTimeForward >= stopDelay) 
  {
    if (speedLevel > 0) speedLevel -= 5; 
  }

  
  if (digitalRead(BUTTON_BACKWARD) == HIGH) 
  {
    if (speedLevel > -100) 
    {
      speedLevel -= 5;
      lastPressTimeBackward = currentTime;
    }
  } 
  else if (currentTime - lastPressTimeBackward >= stopDelay) 
  {
    if (speedLevel < 0) speedLevel += 5; 
  }

  if (digitalRead(BUTTON_LEFT) == HIGH && (currentTime - lastPressTimeLeft > debounceDelay)) {
    if (servoAngle > 0) servoAngle -= 10;
    lastPressTimeLeft = currentTime;
    Serial.print("Servo Angle: ");
    Serial.println(servoAngle);
  }

  if (digitalRead(BUTTON_RIGHT) == HIGH && (currentTime - lastPressTimeRight > debounceDelay)) {
    if (servoAngle < 180) servoAngle += 10;
    lastPressTimeRight = currentTime;
    Serial.print("Servo Angle: ");
    Serial.println(servoAngle);
  }
  if(digitalRead(BUTTON_WATER) == HIGH && (currentTime - lastPressTimeWater > debounceDelay))
  {
    waterState = !waterState; 
    lastPressTimeWater = currentTime;
    Serial.print("Water State: ");
    Serial.println(waterState ? "1 (On)" : "0 (Off)");

  }

  buffer[0] = speedLevel;
  buffer[1] = servoAngle;
  buffer[2] = waterState;
  
  sendBuffer(buffer);

  delay(50);
}

void sendBuffer(int8_t *buffer) {
  radio.stopListening();
  bool send_status = radio.write(buffer, sizeof(int8_t) * 3);

  if (send_status) {
    Serial.print("Data Sent: X = ");
    Serial.print((int)buffer[0]);
    Serial.print(", Y = ");
    Serial.print((int)buffer[1]);
    Serial.print(", Water = ");
    Serial.println((int)buffer[2]);
    digitalWrite(led_pin, HIGH);
  } else {
    Serial.println("Data could not be sent.");
    digitalWrite(led_pin, LOW);
  }
}