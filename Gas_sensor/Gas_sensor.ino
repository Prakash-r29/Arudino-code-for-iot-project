#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C LCD address and dimensions
const int gasSensorPin = A0;         // Analog pin for gas sensor
const int threshold = 500;           // Threshold level for gas detection

void setup() {
  pinMode(gasSensorPin, INPUT);
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Gas Sensor Ready");
  delay(200);
}

void loop() {
  int gasLevel = analogRead(gasSensorPin);  // Read analog value from sensor

  Serial.print("Gas Level: ");
  Serial.println(gasLevel);

  if (gasLevel > threshold) {
    Serial.println("Toxic Gas Detected!");
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Toxic Detected!");

    lcd.setCursor(0, 1);
    lcd.print("Level: ");
    lcd.print(gasLevel);
    lcd.print("%");
    delay(5000);  // Show alert message for 5 seconds
    lcd.clear();  // Clear LCD after alert
  }

  delay(5000);  // Wait before next reading
}
