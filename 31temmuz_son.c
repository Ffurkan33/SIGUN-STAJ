#include <Arduino.h>
#include <DHT.h>
#include "esp_log.h"
#include <driver/ledc.h>

void processCommand(String cmd);
void resetLedState();

const int ledPin = 2;
const int DHTPin = 5; // DHT11 sensörünün bağlı olduğu pin
#define DHTTYPE DHT11

DHT dht(DHTPin, DHTTYPE);
String command = "";  // Seri porttan gelen komutu tutmak için

static const char *TAG = "example";

// LED Durum Makinesi Değişkenleri
enum LedState { IDLE, BLINKING, PWM } ledState = IDLE;
unsigned long previousMillis = 0;
int blinkCount = 0;
int blinkTimes = 0;
int pwmValue = 0;
int pwmDirection = 1;
int pwmCount = 0;

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  dht.begin();
  ESP_LOGI(TAG, "DHT11 Sıcaklık ve Nem Sensörü Başlatıldı");
}

void loop() {
  if (Serial.available() > 0) {
    char c = Serial.read(); // Seri porttan bir karakter oku
    if (c == '\n') { // Eğer yeni satır karakteri geldiyse komutu işle
      processCommand(command);
      command = ""; // Komutu işledikten sonra sıfırla
    } else {
      command += c; // Gelen karakteri komuta ekle
    }
  }

  unsigned long currentMillis = millis();

  if (ledState == BLINKING && currentMillis - previousMillis >= 500) {
    previousMillis = currentMillis;
    digitalWrite(ledPin, !digitalRead(ledPin));
    blinkCount++;
    if (blinkCount >= blinkTimes * 2) { // LED her blink için 2 kez değişir (aç/kapa)
      resetLedState();
    }
  }

  if (ledState == PWM && currentMillis - previousMillis >= 10) {
    previousMillis = currentMillis;
    pwmValue += pwmDirection;
    if (pwmValue <= 0 || pwmValue >= 255) {
      pwmDirection = -pwmDirection;
      if (pwmDirection == 1) { // Parlama ve sönme tamamlandı
        pwmCount++;
        if (pwmCount >= blinkTimes) {
          resetLedState();
        }
      }
    }
    analogWrite(ledPin, pwmValue); // PWM yaz
  }
}

void processCommand(String cmd) {
  cmd.trim(); // Komutun başındaki ve sonundaki boşlukları kaldır
  ESP_LOGI(TAG, "Komut alındı: %s", cmd.c_str());

  if (cmd.equals("L0")) {
    resetLedState();
    digitalWrite(ledPin, HIGH);
    ESP_LOGI(TAG, "LED yakıldı.");

  } else if (cmd.startsWith("L") && cmd.length() > 1) {
    resetLedState();
    blinkTimes = cmd.substring(1).toInt();
    blinkCount = 0;
    previousMillis = millis();
    ledState = BLINKING;
    ESP_LOGI(TAG, "LED %d kez yanıp sönmeye başlayacak.", blinkTimes);

  } else if (cmd.startsWith("A") && cmd.length() > 1) {
    resetLedState();
    blinkTimes = cmd.substring(1).toInt();
    pwmValue = 0;
    pwmDirection = 1;
    pwmCount = 0;
    previousMillis = millis();
    ledState = PWM;
    ESP_LOGI(TAG, "LED %d kez PWM ile parlayıp sönmeye başlayacak.", blinkTimes);

  } else if (cmd.equals("S1")) {
    // Sıcaklık ölçme
    float temperature = dht.readTemperature();
    if (isnan(temperature)) {
      ESP_LOGE(TAG, "Sıcaklık okunamadı!");
      Serial.println("Sıcaklık okunamadı!");
    } else {
      ESP_LOGI(TAG, "Sıcaklık: %.2f °C", temperature);
      Serial.print("Sıcaklık: ");
      Serial.print(temperature);
      Serial.println(" °C");
    }

  } else if (cmd.equals("N1")) {
    // Nem ölçme
    float humidity = dht.readHumidity();
    if (isnan(humidity)) {
      ESP_LOGE(TAG, "Nem okunamadı!");
      Serial.println("Nem okunamadı!");
    } else {
      ESP_LOGI(TAG, "Nem: %.2f %%", humidity);
      Serial.print("Nem: ");
      Serial.print(humidity);
      Serial.println(" %");
    }
  
  } else if (cmd.equals("SN")) {
    // Hem sıcaklık hem nem ölçme
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    if (isnan(temperature) || isnan(humidity)) {
      ESP_LOGE(TAG, "Sıcaklık veya nem okunamadı!");
      Serial.println("Sıcaklık veya nem okunamadı!");
    } else {
      ESP_LOGI(TAG, "Sıcaklık: %.2f °C, Nem: %.2f %%", temperature, humidity);
      Serial.print("Sıcaklık: ");
      Serial.print(temperature);
      Serial.println(" °C");
      Serial.print("Nem: ");
      Serial.print(humidity);
      Serial.println(" %");
    }
  } else {
    ESP_LOGW(TAG, "Geçersiz komut: %s", cmd.c_str());
    Serial.println("Geçersiz komut!");
  }
}

void resetLedState() {
  ledState = IDLE;
  digitalWrite(ledPin, LOW); // LED'i kapat
  pwmValue = 0;
  pwmCount = 0;
  pwmDirection = 1;
  blinkCount = 0;
  blinkTimes = 0;
}
