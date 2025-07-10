#define BLYNK_TEMPLATE_ID "TMPL6rjelSjFo"
#define BLYNK_TEMPLATE_NAME "UAS EMBEDDED"
#define BLYNK_AUTH_TOKEN "iItx1IGdMOcvJVGVK6bQIZnP0Uuk3-KX"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <ESP32Servo.h>

// --- WiFi Credentials ---
char ssid[] = "Cann";
char pass[] = "apahayoo";

// --- Pin Setup ---
#define DHTPIN 4
#define DHTTYPE DHT11
#define LDR_PIN 35
#define RAIN_ANALOG_PIN 33
#define SERVO_PIN 23

// --- Objek Sensor dan Servo ---
DHT dht(DHTPIN, DHTTYPE);
Servo jemuran;

// --- Variabel Mode dan Manual ---
int modeOtomatis = 1;   // 1 = Otomatis, 0 = Manual
int kontrolManual = 0;  // 1 = Buka, 0 = Tutup

BlynkTimer timer;

// Fungsi Update Status Servo
void updateServo() {
  int cahaya = analogRead(LDR_PIN);
  int hujan = analogRead(RAIN_ANALOG_PIN);
  const char* status;

  if (modeOtomatis == 1) {
    if (hujan < 2000) {
      jemuran.write(0);
      status = "Tertutup (Hujan)";
    } else if (cahaya >= 500) {
      jemuran.write(80);
      status = "Terbuka (Cerah)";
    } else {
      jemuran.write(0);
      status = "Tertutup (Gelap)";
    }
  } else {
    if (kontrolManual == 1) {
      jemuran.write(100);
      status = "Terbuka (Manual)";
    } else {
      jemuran.write(0);
      status = "Tertutup (Manual)";
    }
  }

  Blynk.virtualWrite(V6, status);
  Serial.println("Status Servo: " + String(status));
}

// Kirim Data Sensor ke Blynk
void bacaSensor() {
  float suhu = dht.readTemperature();
  float kelembaban = dht.readHumidity();
  int cahaya = analogRead(LDR_PIN);
  int hujan = analogRead(RAIN_ANALOG_PIN);

  // Kirim ke Blynk
  Blynk.virtualWrite(V0, suhu);        // Suhu
  Blynk.virtualWrite(V1, kelembaban);  // Kelembapan
  Blynk.virtualWrite(V2, cahaya);      // LDR
  Blynk.virtualWrite(V3, hujan);       // Sensor hujan
  Blynk.virtualWrite(V4, modeOtomatis);
  Blynk.virtualWrite(V5, kontrolManual);

  // Debug
  Serial.print("Suhu: "); Serial.print(suhu);
  Serial.print(" | Kelembaban: "); Serial.print(kelembaban);
  Serial.print(" | Cahaya: "); Serial.print(cahaya);
  Serial.print(" | Hujan: "); Serial.print(hujan);
  Serial.print(" | Mode: "); Serial.print(modeOtomatis);
  Serial.print(" | Manual: "); Serial.println(kontrolManual);

  updateServo();  // Panggil logika kontrol servo
}

// Saat ESP32 terhubung ke Blynk
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V4); // Sinkronisasi mode otomatis
  Blynk.syncVirtual(V5); // Sinkronisasi kontrol manual
}

// Mode Otomatis dari Blynk (Switch V4)
BLYNK_WRITE(V4) {
  modeOtomatis = param.asInt();
  updateServo();
}

// Kontrol Manual dari Blynk (Switch V5)
BLYNK_WRITE(V5) {
  kontrolManual = param.asInt();
  updateServo();
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  dht.begin();
  jemuran.attach(SERVO_PIN);
  jemuran.write(0); // Posisi awal tertutup
  timer.setInterval(2000L, bacaSensor); // Baca sensor tiap 2 detik
}

void loop() {
  Blynk.run();
  timer.run();
}