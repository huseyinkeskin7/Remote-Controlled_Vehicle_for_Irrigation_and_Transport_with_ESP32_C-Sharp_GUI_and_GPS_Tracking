#include <RF24.h>
#include <TinyGPS++.h>
#include <ESP32Servo.h>
#include <Arduino.h>


#define servoPin 13
Servo myservo;
#define CE_PIN 4
#define CSN_PIN 5
#define RXD2 16
#define TXD2 17

#define ENA_PIN 12
#define IN1_PIN 25
#define IN2_PIN 32

#define ENB_PIN 14
#define IN3_PIN 27
#define IN4_PIN 26

#define WaterPump 33

#define GPS_BAUD 9600

#define TRIG_PIN 15
#define ECHO_PIN 22

RF24 radio(CE_PIN, CSN_PIN);
const byte address[][6] = {"00001", "00002"};

TinyGPSPlus gps;

HardwareSerial gpsSerial(2);

#define led_pin 2

volatile bool collision = false;
volatile bool water = false;
unsigned long lastSendTime = 0; 

void IRAM_ATTR handleCollision() {
  collision = true;
}

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);
  pinMode(ENA_PIN, OUTPUT);
  pinMode(ENB_PIN, OUTPUT);
  pinMode(WaterPump, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(IN3_PIN, LOW);
  digitalWrite(IN4_PIN, LOW);

  myservo.attach(servoPin);
  myservo.write(90);


  radio.begin();
  radio.openWritingPipe(address[1]);
  radio.openReadingPipe(1, address[0]);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();

  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);

  Serial.println("CAR READY");
}

void stopMotors() {
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  analogWrite(ENA_PIN, 0);

  digitalWrite(IN3_PIN, LOW);
  digitalWrite(IN4_PIN, LOW);
  analogWrite(ENB_PIN, 0);
}

void loop() {
  unsigned long currentMillis = millis();

  if (radio.available()) {
    struct DataPacket {
      int8_t joystickX;
      byte joystickY;
      bool water;
      bool collisionFlag;
    } data;

    radio.read(&data, sizeof(data));
    Serial.print("Received Data :  ");
    Serial.print("SPEED : ");
    Serial.print(data.joystickX);
    
    float currentDistance = readDistance();
    if (currentDistance < 10.0) 
    {
      Serial.println(" Obstacle detected! Blocking forward motion.");
      if (data.joystickX > 0) 
      {
        stopMotors(); 
      } 
      else if (data.joystickX < 0) 
      {
   
        digitalWrite(IN1_PIN, LOW);
        digitalWrite(IN2_PIN, HIGH);
        analogWrite(ENA_PIN, map(data.joystickX, -100, 0, 255, 0));

        digitalWrite(IN3_PIN, LOW);
        digitalWrite(IN4_PIN, HIGH);
        analogWrite(ENB_PIN, map(data.joystickX, -100, 0, 255, 0));

        delay(10);
      } 
      else 
      {
        stopMotors();
        delay(10);
      }
    }
    else 
    {
 
    if (data.joystickX > 0) 
    {
      digitalWrite(IN1_PIN, HIGH);
      digitalWrite(IN2_PIN, LOW);
      analogWrite(ENA_PIN, map(data.joystickX, 0, 100, 0, 255));

      digitalWrite(IN3_PIN, HIGH);
      digitalWrite(IN4_PIN, LOW);
      analogWrite(ENB_PIN, map(data.joystickX, 0, 100, 0, 255));

      delay(10);
    } 
    else if (data.joystickX < 0) 
    {
      digitalWrite(IN1_PIN, LOW);
      digitalWrite(IN2_PIN, HIGH);
      analogWrite(ENA_PIN, map(data.joystickX, -100, 0, 255, 0));

      digitalWrite(IN3_PIN, LOW);
      digitalWrite(IN4_PIN, HIGH);
      analogWrite(ENB_PIN, map(data.joystickX, -100, 0, 255, 0));

      delay(10);
    } 
  else 
  {
    stopMotors();
    delay(10);
  }
}
      
    int angle = constrain(data.joystickY, 0, 180);
    Serial.print(", DIRECTION : ");
    Serial.print(angle);
    myservo.write(angle);

    if (data.water) 
    {
      digitalWrite(WaterPump, HIGH);
      Serial.println(", Water Pump: ON");
    } 
    else 
    {
      digitalWrite(WaterPump, LOW);
      Serial.println(", Water Pump: OFF");
    }

    digitalWrite(led_pin, HIGH);
  }


  if (currentMillis - lastSendTime >= 5000) 
  {
    lastSendTime = currentMillis;
    sendGPSData();
  }

  delay(50);
}


bool readGPSData() 
{
  while (gpsSerial.available() > 0) 
  {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isValid()) 
  {
    Serial.print("Latitude  : ");
    Serial.println(gps.location.lat(), 6);
    Serial.print("Longitude : ");
    Serial.println(gps.location.lng(), 6);
  } 
  else 
  {
    Serial.println("Latitude and Longitude data is invalid.");
  }

  return gps.location.isValid();
}

float readDistance() 
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  float duration = pulseIn(ECHO_PIN, HIGH);
  float distance = (duration / 2) / 29.1; 

  return distance;
}

void sendGPSData() 
{
  if (readGPSData()) 
  {
    struct GPSData 
    {
      float latitude;
      float longitude;
      float distance; 
    } gpsData;

    gpsData.latitude = gps.location.lat();
    gpsData.longitude = gps.location.lng();
    gpsData.distance = readDistance(); 

    radio.stopListening();
    delay(250);
    bool send_status = radio.write(&gpsData, sizeof(gpsData));
    radio.startListening();

    if (send_status) 
    {
      Serial.println("GPS Sended :");
      Serial.print("Latitude  : ");
      Serial.println(gpsData.latitude, 6);
      Serial.print("Longitude : ");
      Serial.println(gpsData.longitude, 6);
      Serial.print("Distance  : ");
      Serial.println(gpsData.distance, 2);
    } 
    else 
    {
      Serial.println("GPS data could not be sent.");
    }
  } 
  else 
  {
    Serial.println("GPS data could not be read.");
  }
}