#include <Arduino.h>

namespace {

constexpr uint8_t kRelay1 = 32;
constexpr uint8_t kRelay2 = 33;
constexpr unsigned long kMaximumOnMs = 5000UL;

#if GRIDEX_RELAY_ACTIVE_HIGH
constexpr uint8_t kRelayOn = HIGH;
constexpr uint8_t kRelayOff = LOW;
#else
constexpr uint8_t kRelayOn = LOW;
constexpr uint8_t kRelayOff = HIGH;
#endif

unsigned long relay1StartedAt = 0;
unsigned long relay2StartedAt = 0;

void setRelay(uint8_t pin, bool on) {
  digitalWrite(pin, on ? kRelayOn : kRelayOff);
  Serial.printf("Relay %u: %s\n", pin == kRelay1 ? 1 : 2, on ? "ON" : "OFF");
}

void allOff() {
  setRelay(kRelay1, false);
  setRelay(kRelay2, false);
  relay1StartedAt = 0;
  relay2StartedAt = 0;
}

void startupSelfTest() {
  Serial.println("Starting relay self-test: Relay 1 then Relay 2.");
  setRelay(kRelay1, true);
  delay(1000);
  setRelay(kRelay1, false);
  delay(500);
  setRelay(kRelay2, true);
  delay(1000);
  setRelay(kRelay2, false);
  Serial.println("Relay self-test finished: both relays OFF.");
}

void handleCommand(String command) {
  command.trim();
  command.toLowerCase();

  if (command == "r1 on") { setRelay(kRelay1, true); relay1StartedAt = millis(); return; }
  if (command == "r2 on") { setRelay(kRelay2, true); relay2StartedAt = millis(); return; }
  if (command == "r1 off") { setRelay(kRelay1, false); relay1StartedAt = 0; return; }
  if (command == "r2 off") { setRelay(kRelay2, false); relay2StartedAt = 0; return; }
  if (command == "all off") { allOff(); return; }
  if (command == "status") {
    Serial.println("Relay test ready. Commands: r1 on, r1 off, r2 on, r2 off, all off, status");
    return;
  }
  Serial.println("Unknown command. Use: r1 on, r1 off, r2 on, r2 off, all off, status");
}

}  // namespace

void setup() {
  pinMode(kRelay1, OUTPUT);
  pinMode(kRelay2, OUTPUT);
  allOff();
  Serial.begin(115200);
  delay(100);
  Serial.println("GrideX OLIMEX ESP32-EVB relay test: both relays OFF");
  Serial.println("Safety timeout: any ON command returns to OFF after 5 seconds.");
  handleCommand("status");
  delay(500);
  startupSelfTest();
}

void loop() {
  if (Serial.available()) handleCommand(Serial.readStringUntil('\n'));

  const unsigned long now = millis();
  if (relay1StartedAt && now - relay1StartedAt >= kMaximumOnMs) {
    setRelay(kRelay1, false);
    relay1StartedAt = 0;
    Serial.println("Relay 1 safety timeout reached.");
  }
  if (relay2StartedAt && now - relay2StartedAt >= kMaximumOnMs) {
    setRelay(kRelay2, false);
    relay2StartedAt = 0;
    Serial.println("Relay 2 safety timeout reached.");
  }
}
