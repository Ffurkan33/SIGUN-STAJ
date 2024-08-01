/* tüm komutlar hatasız çalışıyor*/

#include <Arduino.h>
#include <driver/ledc.h>
#include <DHT.h>
#include "esp_log.h"

const int ledPin = 2;
const int DHTPin = 5; // DHT11 sensörünün bağlı olduğu pin
#define DHTTYPE DHT11

DHT dht(DHTPin, DHTTYPE);
String command = "";  // Seri porttan gelen komutu tutmak için

static const char *TAG = "example";

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
}

void processCommand(String cmd) {
  cmd.trim(); // Komutun başındaki ve sonundaki boşlukları kaldır
  ESP_LOGI(TAG, "Komut alındı: %s", cmd.c_str());

  if (cmd.equals("L0")) {
    digitalWrite(ledPin, HIGH);
    ESP_LOGI(TAG, "LED yakıldı.");
  } 
  else if (cmd.startsWith("L") && cmd.length() > 1) {
    int numBlinks = cmd.substring(1).toInt(); 
    // 'L' karakterinden sonraki sayıyı al
    ESP_LOGI(TAG, "LED %d kez yanıp sönecek.", numBlinks);
    for (int i = 0; i < numBlinks; i++) {
      digitalWrite(ledPin, HIGH);
      delay(500); 
      digitalWrite(ledPin, LOW);
      delay(500); 
    }
  } 

else if (cmd.startsWith("A") && cmd.length() > 1) {
    int numBlinks = cmd.substring(1).toInt();
    ESP_LOGI(TAG, "LED %d kez parlayıp sönmeye başlayacak.", numBlinks); 
    for (int i = 0; i < numBlinks; i++) {
      // PWM ile parlama ve sönme işlemi kaç kez istiyorsak (A1,A2,A3...)
      for (int brightness = 0; brightness < 255; brightness++) {
        analogWrite(ledPin, brightness); 
        delay(10);
      }
      for (int brightness = 254; brightness >= 0; brightness--) {
        analogWrite(ledPin, brightness);
        delay(10);
      }
      delay(500); // Parlama ve sönme arasına biraz bekleme ekle
    }
  }

  else if (cmd.equals("S1")) {
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
  } 
  else if (cmd.equals("N1")) {
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
  } 
  else if (cmd.equals("SN")) {
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
  }
  else {
    ESP_LOGW(TAG, "Geçersiz komut: %s", cmd.c_str());
    Serial.println("Geçersiz komut!");
  }
}
