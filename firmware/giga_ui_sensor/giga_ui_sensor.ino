#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <ArduinoBLE.h>
#include <WiFi.h>

String wifi_ssid = "";
String wifi_pass = "";
unsigned long last_wifi_check_time = 0;

// Mbed OS KVStore (Kalıcı bellek)
#include "KVStore.h"
#include "kvstore_global_api.h"

// GIGA Display & LVGL Kütüphaneleri
#include <lvgl.h>  // DIKKAT: lvgl.h KESINLIKLE Arduino_H7_Video'dan ONCE cagirilmali!
#include <Arduino_H7_Video.h>
#include <Arduino_GigaDisplayTouch.h>
#include <Arduino_GigaDisplay.h> // Backlight kontrolü için
#include "lvgl_ui.h"

// Ekran ve Dokunmatik tanımları (Dikey/Portrait için genişlik ve yüksekliği ayarladık)
Arduino_H7_Video Display(480, 800, GigaDisplayShield);
Arduino_GigaDisplayTouch TouchDetector;
GigaDisplayBacklight backlight;


#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme;  // I2C

// Default Setpoint
float setpoint_C = 25.0f;

// BLE UUIDs (Must match UNO R4)
static const char* UUID_SERVICE = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
static const char* UUID_TEMP_C = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
static const char* UUID_SETPOINT = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";
static const char* UUID_RELAY_STA = "6E400004-B5A3-F393-E0A9-E50E24DCCA9E";
static const char* UUID_SYS_MODE = "6E400005-B5A3-F393-E0A9-E50E24DCCA9E";
static const char* UUID_HYSTERESIS = "6E400006-B5A3-F393-E0A9-E50E24DCCA9E";

BLEDevice peripheral;
BLECharacteristic chTemp;
BLECharacteristic chSp;
BLECharacteristic chRelay;
BLECharacteristic chMode;
BLECharacteristic chHysteresis;

unsigned long lastUpdate = 0;
unsigned long lastChartUpdate = 0;
bool relayState = false;
bool is_screen_dimmed = false;

// Ayarları kalıcı bellekten yükleme
void loadSettings() {
  size_t actual_size;
  float stored_temp;
  if (kv_get("target_temp", &stored_temp, sizeof(stored_temp), &actual_size) == MBED_SUCCESS) {
    if (actual_size == sizeof(stored_temp)) {
      target_temp_val = stored_temp;
      Serial.print("Loaded target_temp: "); Serial.println(target_temp_val);
    }
  }
  
  uint8_t stored_mode;
  if (kv_get("sys_mode", &stored_mode, sizeof(stored_mode), &actual_size) == MBED_SUCCESS) {
    if (actual_size == sizeof(stored_mode)) {
      current_system_mode = stored_mode;
      Serial.print("Loaded sys_mode: "); Serial.println(current_system_mode);
    }
  }

  float stored_hysteresis;
  if (kv_get("hysteresis", &stored_hysteresis, sizeof(stored_hysteresis), &actual_size) == MBED_SUCCESS) {
    if (actual_size == sizeof(stored_hysteresis)) {
      hysteresis_val = stored_hysteresis;
      Serial.print("Loaded hysteresis: "); Serial.println(hysteresis_val);
    }
  }

  char buf[64];
  if (kv_get("wifi_ssid", buf, sizeof(buf), &actual_size) == MBED_SUCCESS) {
    wifi_ssid = String(buf);
  }
  if (kv_get("wifi_pass", buf, sizeof(buf), &actual_size) == MBED_SUCCESS) {
    wifi_pass = String(buf);
  }
}

// Ayarları kalıcı belleğe kaydetme
void saveSettings() {
  kv_set("target_temp", &target_temp_val, sizeof(target_temp_val), 0);
  kv_set("sys_mode", &current_system_mode, sizeof(current_system_mode), 0);
  kv_set("hysteresis", &hysteresis_val, sizeof(hysteresis_val), 0);
  Serial.println("Settings saved to KVStore.");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("GIGA R1 - Room Thermostat Central");

  // Ayarları yükle
  loadSettings();

  // I2C Baslatma (GIGA icin garantilemek adina acikca cagiriyoruz)
  Wire.begin();

  // Ekranı ve Dokunmatiği Başlat
  Display.begin();
  TouchDetector.begin();
  backlight.begin();
  backlight.set(100); // %100 Parlaklık

  // LVGL Arayüzümüzü oluştur
  create_thermostat_ui();

  // Ilk cizimi ekrana yansit (BME hatasinda asagida takilirsa ekran siyah kalmasin diye)
  lv_timer_handler();

  // BME680 Baslatma (I2C Tarayicida 0x77 oldugunu teyit ettik)
  if (!bme.begin(0x77)) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    // Ekranda hata goster (Basitce mevcut sicaklik kismina HATA yazdiralim)
    if (label_current_temp) lv_label_set_text(label_current_temp, "BME HATA");
    lv_timer_handler();
  } else {
    // Set up oversampling and filter initialization
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150);  // 320*C for 150 ms
  }

  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1)
      ;
  }

  Serial.println("Starting BLE Scan...");
  BLE.scanForUuid(UUID_SERVICE);
}

void loop() {
  static bool last_ble_connected = false;
  bool current_ble_connected = (peripheral && peripheral.connected());
  
  if (current_ble_connected != last_ble_connected) {
    last_ble_connected = current_ble_connected;
    update_ble_status_ui(current_ble_connected);
  }

  if (wifi_scan_requested) {
    wifi_scan_requested = false;
    Serial.println("WiFi taramasi baslatiliyor...");
    int numNetworks = WiFi.scanNetworks();
    if (numNetworks > 0) {
      String* ssids = new String[numNetworks];
      for(int i = 0; i < numNetworks; i++) ssids[i] = WiFi.SSID(i);
      populate_wifi_list(numNetworks, ssids);
      delete[] ssids;
    } else {
      populate_wifi_list(0, NULL);
    }
  }

  if (wifi_credentials_ready) {
    wifi_credentials_ready = false;
    wifi_ssid = selected_ssid;
    wifi_pass = entered_password;
    kv_set("wifi_ssid", wifi_ssid.c_str(), wifi_ssid.length() + 1, 0);
    kv_set("wifi_pass", wifi_pass.c_str(), wifi_pass.length() + 1, 0);
    WiFi.disconnect();
    last_wifi_check_time = millis() - 10000; // Hemen bağlanmayı dene
  }

  // Wi-Fi Bağlantı Kontrolü (Her 10 saniyede bir)
  if (millis() - last_wifi_check_time > 10000) {
    last_wifi_check_time = millis();
    if (WiFi.status() != WL_CONNECTED && wifi_ssid.length() > 0) {
      Serial.println("WiFi baglaniliyor: " + wifi_ssid);
      WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
    }
    update_wifi_status_ui(WiFi.status() == WL_CONNECTED);
  }

  // Sensöru her 2 saniyede bir OKU ve EKRANI GUNCELLE (BLE baglantisindan bagimsiz)
  if (millis() - lastUpdate > 2000) {
    lastUpdate = millis();

    if (bme.performReading()) {
      float temp = bme.temperature;
      float gas_res = bme.gas_resistance;
      float hum = bme.humidity;
      float press = bme.pressure / 100.0f;  // hPa

      Serial.print("T: ");
      Serial.print(temp);
      Serial.print(" *C  |  SP: ");
      Serial.print(target_temp_val);
      Serial.print(" *C  |  RELAY: ");
      Serial.println(relayState ? "ON" : "OFF");

      if (gas_res < 50000.0) {
        Serial.println(">>> UYARI: Hava kalitesi dustu! Odayi Havalandirin. <<<");
      }

      // Ekrana yansıt (Her durumda)
      update_bme680_ui(temp, hum, press, gas_res);

      // Grafiği her 60 saniyede bir güncelle (LVGL Timer da kullanılabilir)
      if (millis() - lastChartUpdate > 60000 || lastChartUpdate == 0) {
        lastChartUpdate = millis();
        lv_chart_set_next_value(chart, ser_temp, temp);
      }

      // Eger BLE bagliysa karsi tarafa gonder
      if (peripheral && peripheral.connected()) {
        chTemp.writeValue((uint8_t*)&temp, 4);
        chSp.writeValue((uint8_t*)&target_temp_val, 4);
        
        if (hysteresis_changed) {
          if (chHysteresis) chHysteresis.writeValue((uint8_t*)&hysteresis_val, 4);
          hysteresis_changed = false;
        }
      }
    }
  }

  // BLE Baglanti Kontrolleri
  if (!peripheral || !peripheral.connected()) {
    peripheral = BLE.available();

    if (peripheral) {
      Serial.print("Found UNO R4: ");
      Serial.println(peripheral.address());

      BLE.stopScan();
      connectToPeripheral(peripheral);
    }
  }

  if (peripheral && peripheral.connected()) {
    // Read relay state if changed
    if (chRelay.valueUpdated()) {
      uint8_t val = 0;
      chRelay.readValue(val);
      relayState = (val == 1);
    }
    
    // Write system mode if changed by UI
    if (system_mode_changed) {
      if(chMode) chMode.writeValue((uint8_t*)&current_system_mode, 1);
      system_mode_changed = false;
    }
  } else if (peripheral && !peripheral.connected()) {
    Serial.println("Lost connection! Restarting scan...");
    BLE.scanForUuid(UUID_SERVICE);
    peripheral = BLEDevice();  // Reset peripheral object
  }

  // EEPROM Kaydetme tetikleyicisi
  if (settings_need_save) {
    saveSettings();
    settings_need_save = false;
  }

  // Ekran Uyku Modu (30 Saniye Hareketsizlik)
  uint32_t inactive_time = lv_disp_get_inactive_time(NULL);
  if (inactive_time > 30000) {
    if (!is_screen_dimmed) {
      backlight.set(10); // Parlaklığı kıs
      is_screen_dimmed = true;
    }
  } else {
    if (is_screen_dimmed) {
      backlight.set(100); // Parlaklığı eski haline getir
      is_screen_dimmed = false;
    }
  }

  // LVGL görev yöneticisi (animasyonlar ve dokunmatik için)
  lv_timer_handler();
}

void connectToPeripheral(BLEDevice periph) {
  Serial.println("Connecting ...");

  if (periph.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  Serial.println("Discovering attributes ...");
  if (periph.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    periph.disconnect();
    return;
  }

  chTemp = periph.characteristic(UUID_TEMP_C);
  chSp = periph.characteristic(UUID_SETPOINT);
  chRelay = periph.characteristic(UUID_RELAY_STA);
  chMode = periph.characteristic(UUID_SYS_MODE);
  chHysteresis = periph.characteristic(UUID_HYSTERESIS);

  if (!chTemp || !chSp || !chRelay || !chMode || !chHysteresis) {
    Serial.println("Peripheral does not have required characteristics!");
    periph.disconnect();
    return;
  }

  // Subscribe to relay state notifications
  if (chRelay.canSubscribe()) {
    chRelay.subscribe();
  }
  
  // Send initial mode and hysteresis
  if (chMode) {
    chMode.writeValue((uint8_t*)&current_system_mode, 1);
    system_mode_changed = false;
  }
  if (chHysteresis) {
    chHysteresis.writeValue((uint8_t*)&hysteresis_val, 4);
    hysteresis_changed = false;
  }
}
