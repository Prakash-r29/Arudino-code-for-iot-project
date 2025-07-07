#include <LiquidCrystal_I2C.h> 
#include <DHT.h>

#define DHTPIN D7
#define DHTTYPE DHT11
#define TEMP_THRESHOLD 35  // Set threshold to 35°C

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C address and LCD size

void setup() {
  Serial.begin(9600);
  dht.begin();

  lcd.init();          
  lcd.backlight();     

  lcd.setCursor(0, 0);
  lcd.print("Temp Sensor Init");
  delay(2000);
  lcd.clear();
} 

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Print values to Serial Monitor
  Serial.println("TEMPERATURE = " + String(t) + " C");
  Serial.println("HUMIDITY = " + String(h) + " %");

  lcd.clear();
  lcd.setCursor(0, 0);

  if (t >= TEMP_THRESHOLD) {
    lcd.print("HIGH TEMP:");
    Serial.println("HIGH TEMPERATURE ALERT");
  } else {
    lcd.print("LOW TEMP:");
    Serial.println("LOW TEMPERATURE ALERT");
  }

  lcd.print(t);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Humidity:");
  lcd.print(h);
  lcd.print("%");

  delay(5000);
}
