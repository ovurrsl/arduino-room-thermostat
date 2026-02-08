#include <ArduinoBLE.h>
#include <math.h>

// =====================================================
// [RELAY SHIELD PIN MAP]  Arduino 4 Relays Shield
// Relay 1 -> D4
// Relay 2 -> D7
// Relay 3 -> D8
// Relay 4 -> D12
// =====================================================
static const uint8_t RELAY_CH = 1;   // <<< KULLANDIĞIN RÖLE: 1/2/3/4

static int relayPinFromChannel(uint8_t ch) {
  switch (ch) {
    case 1: return 4;
    case 2: return 7;
    case 3: return 8;
    case 4: return 12;
    default: return 4;
  }
}

// =====================================================
// [RELAY LOGIC]
// Shield’lerde çoğunlukla LOW=ON çalışır.
// =====================================================
static const bool RELAY_ACTIVE_LOW = true;   // LOW=ON
static const float ON_DELTA  = 0.5f;         // T <= SP-0.5 => ON
static const float OFF_DELTA = 0.2f;         // T >= SP+0.2 => OFF

// =====================================================
// [BLE UUID] (GIGA ile AYNI olmalı)
// =====================================================
static const char* UUID_SERVICE   = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
static const char* UUID_TEMP_C    = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
static const char* UUID_SETPOINT  = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";
static const char* UUID_RELAY_STA = "6E400004-B5A3-F393-E0A9-E50E24DCCA9E";

// =====================================================
// [BLE SERVICE / CHARS]
// GIGA (Central) -> UNO (Peripheral) yazıyor (4 byte float)
// =====================================================
BLEService svc(UUID_SERVICE);
BLECharacteristic chTemp(UUID_TEMP_C, BLEWrite | BLEWriteWithoutResponse, 4);
BLECharacteristic chSp  (UUID_SETPOINT, BLEWrite | BLEWriteWithoutResponse, 4);
BLECharacteristic chRelay(UUID_RELAY_STA, BLENotify, 1);

// =====================================================
// [STATE]
// =====================================================
static float temp_C = NAN;
static float setpoint_C = NAN;
static bool relay_on = false;

static int RELAY_PIN = 4;

// -----------------------------------------------------
// helper: read 4-byte float from BLECharacteristic
// -----------------------------------------------------
static bool readFloat4(BLECharacteristic &ch, float &out) {
  uint8_t b[4];
  int n = ch.readValue(b, 4);
  if (n != 4) return false;
  memcpy(&out, b, 4);
  return isfinite(out);
}

// -----------------------------------------------------
// relay write (with safe mapping + notify)
// -----------------------------------------------------
static void relay_write(bool on) {
  relay_on = on;

  int level;
  if (RELAY_ACTIVE_LOW) level = on ? LOW : HIGH;
  else                 level = on ? HIGH : LOW;

  digitalWrite(RELAY_PIN, level);

  uint8_t v = on ? 1 : 0;
  chRelay.writeValue(&v, 1);

  Serial.print("RELAY_CH="); Serial.print(RELAY_CH);
  Serial.print(" PIN=D"); Serial.print(RELAY_PIN);
  Serial.print(" => "); Serial.print(on ? "ON" : "OFF");
  Serial.print("  pinLevel="); Serial.println(level == LOW ? "LOW" : "HIGH");
}

// =====================================================
// [SETUP]
// =====================================================
void setup() {
  Serial.begin(115200);
  delay(200);

  RELAY_PIN = relayPinFromChannel(RELAY_CH);

  // Güvenli başlangıç (active LOW ise OFF = HIGH)
  digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
  pinMode(RELAY_PIN, OUTPUT);

  // Başlangıç OFF
  relay_write(false);

  if (!BLE.begin()) {
    Serial.println("BLE.begin FAILED");
    while (1) delay(1000);
  }

  svc.addCharacteristic(chTemp);
  svc.addCharacteristic(chSp);
  svc.addCharacteristic(chRelay);

  BLE.addService(svc);
  BLE.setLocalName("UNO_R4_4RELAYS");
  BLE.setAdvertisedService(svc);
  BLE.advertise();

  Serial.println("UNO R4 + 4Relays Shield BLE Peripheral ready.");
  Serial.print("Using RELAY_CH="); Serial.print(RELAY_CH);
  Serial.print("  PIN=D"); Serial.println(RELAY_PIN);
}

// =====================================================
// [LOOP]
// =====================================================
void loop() {
  BLEDevice central = BLE.central();
  if (!central) return;

  Serial.print("Central connected: ");
  Serial.println(central.address());

  while (central.connected()) {

    // GIGA temp yazdı mı?
    if (chTemp.written()) {
      float v;
      if (readFloat4(chTemp, v)) temp_C = v;
    }

    // GIGA setpoint yazdı mı?
    if (chSp.written()) {
      float v;
      if (readFloat4(chSp, v)) setpoint_C = v;
    }

    // değerler hazırsa kontrol
    if (isfinite(temp_C) && isfinite(setpoint_C)) {
      bool shouldOn  = (temp_C <= (setpoint_C - ON_DELTA));
      bool shouldOff = (temp_C >= (setpoint_C + OFF_DELTA));

      if (!relay_on && shouldOn)  relay_write(true);
      if ( relay_on && shouldOff) relay_write(false);

      Serial.print("T=");  Serial.print(temp_C, 2);
      Serial.print("  SP="); Serial.print(setpoint_C, 2);
      Serial.print("  REL="); Serial.println(relay_on ? "ON" : "OFF");
    }

    delay(50);
  }

  Serial.println("Central disconnected.");
}
