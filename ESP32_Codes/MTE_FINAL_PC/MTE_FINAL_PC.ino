#include <RF24.h>
#include <Arduino.h>

#define CE_PIN 4
#define CSN_PIN 5
#define LED_PIN 2  // LED'in bağlandığı pin

RF24 radio(CE_PIN, CSN_PIN);
const byte address[] = "00002";  // PC'nin adresi, araç bu adrese veri gönderecek

// GPS verileri için bir yapı tanımlıyoruz
struct GPSData {
  float latitude;
  float longitude;
  float distance;
};

void setup() {
  Serial.begin(115200);
  radio.begin();
  radio.openReadingPipe(1, address);  // Araçtan gelen veriyi dinlemek için
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();  // Dinlemeye başla

  pinMode(LED_PIN, OUTPUT);  // LED pinini çıkış olarak ayarla
  digitalWrite(LED_PIN, LOW);  // Başlangıçta LED kapalı

  Serial.println("PC Ready to Check Connection with Vehicle.");
}

void loop() {
  // NRF modülünden veri alınabilir durumda mı? Eğer evet, haberleşme kurulmuş demektir.
  if (radio.available()) {
    digitalWrite(LED_PIN, HIGH);  // Haberleşme kuruldu, LED'i yak
    Serial.println("Connection with vehicle established!");

    // Veri okuma
    GPSData gpsData;
    radio.read(&gpsData, sizeof(gpsData));  // GPS verisini al

    // Seri monitöre veriyi yazdır
    Serial.print("Latitude: ");
    Serial.println(gpsData.latitude, 6);  // 6 basamaklı hassasiyet
    Serial.print("Longitude: ");
    Serial.println(gpsData.longitude, 6);  // 6 basamaklı hassasiyet
    Serial.print("Distance: ");
    Serial.println(gpsData.distance, 2);  // 2 basamaklı hassasiyet (metre hassasiyeti)

    delay(4000);  // Haberleşme kontrol aralığı
  } else {
    digitalWrite(LED_PIN, LOW);  // Haberleşme yoksa LED'i söndür
  }

  delay(500);  // Küçük bir gecikme
}