// === Blynk Credentials ===
#define BLYNK_TEMPLATE_ID "TMPL6p2RLvLFB"
#define BLYNK_TEMPLATE_NAME "fire detection02"
#define BLYNK_AUTH_TOKEN "XD16dbnSUrJN6aSsdETYGCpGPPbxCGxa"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>  // Untuk Telegram

// === WiFi & Blynk Auth ===
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "POCO M3";
char pass[] = "12345678";

BlynkTimer timer;

// === Pin Sensor dan Aktuator ===
#define FIRE_SENSOR 23
#define MQ2_SENSOR  34  // AOUT dari MQ-2 (analog)
#define GREEN_LED   26
#define YELLOW_LED  18
#define RED_LED     12
#define BUZZER      13

// === Virtual Pin untuk Widget Tambahan ===
#define VPIN_SMOKE V2
#define VPIN_GAS   V3

// === Telegram Bot Credentials ===
String BOT_TOKEN = "7635617592:AAGJ0gSF06netLRcFBwcjDRCTIKHk-wnG9M";
String CHAT_ID = "2131924306";

// === Widget Blynk ===
WidgetLED led(V1);

// === Fungsi Kirim Telegram ===
void sendTelegramMessage(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://api.telegram.org/bot" + BOT_TOKEN + "/sendMessage?chat_id=" + CHAT_ID + "&text=" + message;
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.println("✅ Pesan Telegram terkirim.");
    } else {
      Serial.println("❌ Gagal kirim pesan Telegram.");
    }
    http.end();
  }
}

void setup() {
  Serial.begin(115200);

  // === Inisialisasi Pin ===
  pinMode(FIRE_SENSOR, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
   pinMode(BUZZER, OUTPUT);

  // === Koneksi ke WiFi + Blynk ===
  Blynk.begin(auth, ssid, pass);

  // Jadwalkan pengecekan sensor setiap 1 detik
  timer.setInterval(1000L, mySensor);
}

void loop() {
  Blynk.run();
  timer.run();
}

// === Fungsi Cek Sensor dan Kontrol Output ===
void mySensor() {
  int fireVal = digitalRead(FIRE_SENSOR);
  int mq2Val = analogRead(MQ2_SENSOR);

  // === Tampilkan di Serial Monitor ===
  Serial.println("========== SENSOR STATUS ==========");
  Serial.print("🔥 Sensor Api (digital): ");
  Serial.println(fireVal);
  Serial.print("🌫️ Sensor MQ2 (analog): ");
  Serial.println(mq2Val);
  Serial.println("===================================");

  // === Threshold Penyesuaian ===
  int asapThreshold = 100;
  int gasThreshold  = 1000;

  // === Logika Deteksi ===
  bool apiDetected   = (fireVal == LOW);
  bool gasDetected   = (mq2Val > gasThreshold);
  bool smokeDetected = (mq2Val > asapThreshold && mq2Val <= gasThreshold);

  if (apiDetected) {
    Serial.println("🚨 Deteksi: API TERDETEKSI");
    Blynk.logEvent("fire_alert", "🔥 Api terdeteksi!");
    digitalWrite(RED_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BUZZER, HIGH);
    led.on();
    Blynk.virtualWrite(V0, 1);
    Blynk.virtualWrite(VPIN_SMOKE, "Tidak Ada Asap");
    Blynk.virtualWrite(VPIN_GAS, "Tidak Ada Gas");
    sendTelegramMessage("🔥 *Peringatan!* Api terdeteksi di lokasi!");
  } 
  else if (gasDetected) {
    Serial.println("🚨 Deteksi: GAS TERDETEKSI");
    Blynk.logEvent("gas_alert", "🧪 Gas terdeteksi!");
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BUZZER, HIGH);
    led.on();
    Blynk.virtualWrite(V0, 1);
    Blynk.virtualWrite(VPIN_SMOKE, "Tidak Ada Asap");
    Blynk.virtualWrite(VPIN_GAS, "Gas Terdeteksi!");
    sendTelegramMessage("🧪 *Bahaya!* Gas terdeteksi di lokasi!");
  } 
  else if (smokeDetected) {
    Serial.println("🚨 Deteksi: ASAP TERDETEKSI");
    Blynk.logEvent("smoke_alert", "🌫️ Asap terdeteksi!");
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BUZZER, HIGH);
    led.on();
    Blynk.virtualWrite(V0, 1);
    Blynk.virtualWrite(VPIN_SMOKE, "Asap Terdeteksi!");
    Blynk.virtualWrite(VPIN_GAS, "Tidak Ada Gas");
    sendTelegramMessage("🌫️ *Peringatan!* Asap terdeteksi di lokasi!");
  } 
  else {
    Serial.println("✅ Tidak ada deteksi. Aman.");
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(BUZZER, LOW);
    led.off();
    Blynk.virtualWrite(V0, 0);
    Blynk.virtualWrite(VPIN_SMOKE, "Tidak Ada Asap");
    Blynk.virtualWrite(VPIN_GAS, "Tidak Ada Gas");
  }

  Serial.println();
  delay(500); // jeda supaya serial tidak terlalu cepat
}
