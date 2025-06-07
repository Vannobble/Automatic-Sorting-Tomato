#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <Servo.h>

#define TCA_ADDR 0x70

Servo myservo;
const int SERVO_PIN = 9;
unsigned long servoTriggerTime = 0;
bool servoAktif = false;
const unsigned long durasiServoAktif = 5000; 

// Sensor warna channel 0, 1, 3
const uint8_t channels[3] = {0, 1, 3};
Adafruit_TCS34725 tcsArray[3] = {
  Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X),
  Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X),
  Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X)
};

void tcaSelect(uint8_t channel) {
  if (channel > 7) return;
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

String klasifikasiWarna(uint16_t r, uint16_t g, uint16_t b, uint16_t c) {
  if (c > 280) return "Tidak ada objek";

  if (r > g + 20 && r > b + 30) return "Merah";
  else if (abs(r - g) < 10 && abs(r - b) < 15) return "Normal";
  else if (g > r + 10 && g > b + 10) return "Hijau";
  else return "Tidak diketahui";
}

String klasifikasiMayoritas() {
  int countMerah = 0, countHijau = 0, countNormal = 0, countObjek = 0;

  for (uint8_t i = 0; i < 3; i++) {
    uint8_t ch = channels[i];
    tcaSelect(ch);
    delay(30);

    if (!tcsArray[i].begin()) {
      Serial.print("Sensor tidak terdeteksi di channel "); Serial.println(ch);
      continue;
    }

    uint16_t r, g, b, c;
    tcsArray[i].getRawData(&r, &g, &b, &c);

    Serial.print("Channel "); Serial.print(ch);
    Serial.print(" => R: "); Serial.print(r);
    Serial.print(" G: "); Serial.print(g);
    Serial.print(" B: "); Serial.print(b);
    Serial.print(" C: "); Serial.println(c);

    String hasil = klasifikasiWarna(r, g, b, c);
    Serial.print("Klasifikasi: "); Serial.println(hasil);

    if (hasil != "Tidak ada objek") countObjek++;

    if (hasil == "Merah") countMerah++;
    else if (hasil == "Hijau") countHijau++;
    else if (hasil == "Normal") countNormal++;
  }

  
  if (countObjek < 2) return "Tidak ada objek";

  if (countMerah >= 2) return "Merah";
  else if (countHijau >= 2) return "Hijau";
  else if (countNormal >= 2) return "Normal";
  else return "Tidak diketahui";
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  myservo.attach(SERVO_PIN);
  myservo.write(0);
  Serial.println("Sistem deteksi warna aktif.");
}

void loop() {
  String hasilMayoritas = klasifikasiMayoritas();

  if (!servoAktif && hasilMayoritas == "Merah") {
    Serial.println("Mayoritas MERAH - Servo bergerak 60 derajat.");
    myservo.write(60);
    servoAktif = true;
    servoTriggerTime = millis();
  }

  if (servoAktif && millis() - servoTriggerTime >= durasiServoAktif) {
    myservo.write(0);
    servoAktif = false;
    Serial.println("Servo kembali ke posisi 0.");
  }

  delay(150);
}
