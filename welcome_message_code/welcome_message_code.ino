#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // LCD address 0x27, 16 chars, 2 lines

void setup() {
  Serial.begin(9600);               // Start serial communication at 9600 baud
  lcd.init();                       // Initialize the LCD
  lcd.backlight();                  // Turn on backlight
Serial.println("E UNIFORM SOLDIERS");
  lcd.setCursor(0, 0);
  lcd.print("SMART E-UNIFORM");
  lcd.setCursor(0, 1);
  lcd.print("SOLDIERS");
}

void loop() {
  // Optional: Keep screen stable or refresh
  delay(5000);
}
