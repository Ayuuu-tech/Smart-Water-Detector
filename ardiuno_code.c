#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define PH_PIN A0
#define TDS_PIN A1
#define TURBIDITY_PIN A2

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Use your I2C address

// Function to average analog input
float readAveragedSensor(int pin, int samples = 10) {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delay(5);
  }
  return sum / (float)samples;
}

// Track previous drinkability status
bool isDrinkable = true;
bool lastStatus = true;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(1500);
  lcd.clear();
}

void loop() {
  // Read sensor values (averaged)
  float phRaw = readAveragedSensor(PH_PIN);
  float tdsRaw = readAveragedSensor(TDS_PIN);
  float turbRaw = readAveragedSensor(TURBIDITY_PIN);

  float phVoltage = phRaw * 5.0 / 1023.0;
  float tdsVoltage = tdsRaw * 5.0 / 1023.0;
  float turbVoltage = turbRaw * 5.0 / 1023.0;

  float pH = 7 + ((2.5 - phVoltage) / 0.18);
  float tds = tdsVoltage * 133.42;
  float turbidity = (1.0 - turbVoltage / 5.0) * 100.0;

  // Apply improved hysteresis
  if (pH > 6.7 && pH < 8.3 && tds < 480) {
    isDrinkable = true;
  } else if (pH < 6.3 || pH > 8.7 || tds > 520) {
    isDrinkable = false;
  }

  // Serial debug info
  Serial.print("pH: "); Serial.print(pH, 2);
  Serial.print(" | TDS: "); Serial.print(tds, 1);
  Serial.print(" | Turb: "); Serial.print(turbidity, 1);
  Serial.print(" | Status: ");
  Serial.println(isDrinkable ? "SAFE" : "UNSAFE");

  // Only update LCD if status or readings significantly change
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();
  if (isDrinkable != lastStatus || now - lastUpdate >= 5000) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("pH:");
    lcd.print(pH, 1);
    lcd.print(" T:");
    lcd.print((int)tds);

    lcd.setCursor(0, 1);
    lcd.print("Tu:");
    lcd.print((int)turbidity);
    lcd.print("% ");
    lcd.print(isDrinkable ? "SAFE" : "UNSAFE");

    lastStatus = isDrinkable;
    lastUpdate = now;
  }

  delay(1000);  // Delay before next reading
}
