
#include<LiquidCrystal_I2C.h>

//ultrasonic sensor pins

#define TRIG_PIN D3  // GPIO pin connected to the TRIG pin of the sensor
#define ECHO_PIN D4  // GPIO pin connected to the ECHO pin of the sensor
// LCD setup(address 0X27 is common; if needed)

LiquidCrystal_I2C lcd( 0x27,16,2);


// Variable to store the duration and distance
long duration;
float distance;

void setup() {
  // Initialize serial communication at 115200 baud rate
  Serial.begin(115200);
  
  // Set TRIG_PIN as output and ECHO_PIN as input
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  //lcd initialization
  lcd.init();
  lcd.backlight( );
  lcd.setCursor(0,0);
  lcd.print("Distance sensor");
}

void loop() {
  // Ensure the trigger pin is low for a short period to start the measurement cycle
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Send a 10-microsecond HIGH pulse to trigger the sensor
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure the length of time (in microseconds) that the ECHO pin stays HIGH
  duration = pulseIn(ECHO_PIN, HIGH); 
  
  // Calculate the distance in centimeters
  // Speed of sound is 343 m/s or 0.0343 cm/µs. Dividing by 2 because the sound travels to the object and back.
  distance = (duration * 0.0343) / 2;
  
  // Print the distance to the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  //print to LCD
  lcd.clear( );
  lcd.setCursor(0,0);
  lcd.print("Distance:");
  lcd.print(distance);
  lcd.print("cm");
  // Wait for 500 milliseconds before taking the next measurement
  delay(5000);
}
