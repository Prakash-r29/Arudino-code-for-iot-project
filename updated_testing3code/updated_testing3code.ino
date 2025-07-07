#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <EMailSender.h>
#include <base64.h>
#include <WiFiClientSecure.h>

// LCD Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin Definitions
#define TRIG_PIN D3
#define ECHO_PIN D4
#define GAS_SENSOR A0
#define RELAY D5
#define RELAY1 D6
#define DHTPIN D7
#define DHTTYPE DHT11

// Email Sender
EMailSender emailSend("pp4399348raj@gmail.com", "yaoh maap cwob ldwo");
// Twilio credentials
const char* accountSID = "AC3196a13378a5b0de7af86bf5de03538d";
const char* authToken = "f37edf78846e37f7d5b57e410e2cffec";
const char* fromNumber = "+12298505618"; // Twilio
const char* toNumber   = "+919361208320"; // Your number
const char* twilioHost = "api.twilio.com";
const int httpsPort = 443;

// Network + MQTT
const char* ssid = "vivo Y15s";
const char* password = "prakash29";
const char* mqtt_server = "broker.hivemq.com";

// ThingSpeak
const char* TS_SERVER = "api.thingspeak.com";
String TS_API_KEY = "KV6LEKXTDZGLWJLX";

// Objects
DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

// MQTT
void callback(char* topic, byte* payload, unsigned int length) {}
void reconnect() {
  while (!client.connected()) {
    String clientId = "ESP8266-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      client.subscribe("soldier");
    } else {
      delay(5000);
    }
  }
}
void mqtt() {
  client.setCallback(callback);
  if (!client.connected()) reconnect();
  client.loop();
}

// WiFi
void connectwifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println("WiFi connected");
}
int entryCount=0;

//Ultrasonic Distance Funtion
long readUltrasonicDistance() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;
}
// Send data to ThingSpeak
void sendToThingSpeak(float temperature, float humidity, int gasValue, long distance) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(TS_SERVER, httpPort)) {
      Serial.println("Connection to ThingSpeak failed");
      return;
    }

    String url = "/update?api_key=" + TS_API_KEY;
    url += "&field1=" + String(temperature);
    url += "&field2=" + String(humidity);
    url += "&field3=" + String(gasValue);
    url += "&field4=" + String(distance);

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + TS_SERVER + "\r\n" +
                 "Connection: close\r\n\r\n");

    Serial.println("Sent data to ThingSpeak:");
    Serial.println(url);
  } else {
    Serial.println("WiFi not connected");
  }
}
// URL Encoding helper
String urlencode(String str) {
  String encodedString = "";
  char c, code0, code1;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0'; if ((c & 0xf) > 9) code1 = (c & 0xf) - 10 + 'A';
      c = (c >> 4) & 0xf;      code0 = c + '0'; if (c > 9) code0 = c - 10 + 'A';
      encodedString += '%'; encodedString += code0; encodedString += code1;
    }
  }
  return encodedString;
}
// Twilio SMS Function
void sendSMS(String messageBody) {
  WiFiClientSecure client;
  client.setInsecure();

  if (!client.connect(twilioHost, httpsPort)) {
    Serial.println("❌ Twilio connection failed");
    return;
  }

  // Use raw phone number directly without variables
  String postData = "To=%2B919361208320"  // "+91..." properly encoded
                    "&From=%2B12298505618"  // Twilio number
                    "&Body=" + urlencode(messageBody);
  String credentials = String(accountSID) + ":" + String(authToken);
  String encodedCreds = base64::encode(credentials);

  String url = "/2010-04-01/Accounts/" + String(accountSID) + "/Messages.json";

  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + twilioHost + "\r\n" +
               "Authorization: Basic " + encodedCreds + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded\r\n" +
               "Content-Length: " + postData.length() + "\r\n\r\n" +
               postData);

  Serial.println("📡 SMS Request Sent. Awaiting Response:");
  
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break;
    Serial.println(line);
  }

  String responseBody = client.readString();
  Serial.println("📨 SMS Response:");
  Serial.println(responseBody);
}
void setup() {
  Serial.begin(9600);

  //pin setup
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(GAS_SENSOR, INPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(RELAY1, OUTPUT);
  digitalWrite(RELAY, LOW);  // Peltier OFF
  digitalWrite(RELAY1, LOW); // Fan OFF

  dht.begin();
  delay(1000); // Let DHT settle
  dht.readTemperature(); // dummy read to stabilize

  lcd.init(); 
  lcd.backlight();
  
//connect wifi&MQTT
  connectwifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  mqtt();

  // Read sensors
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Validate DHT sensor data
 if (isnan(temperature) || isnan(humidity)){
    Serial.println("⚠️ Invalid DHT11 reading. Skipping.");
    return;
  }
  int gasValue = analogRead(GAS_SENSOR);
  long distance = readUltrasonicDistance();

  //output to serial monitor
  entryCount++;
  Serial.println("\n--- Entry #" + String(entryCount) + " ---");
  Serial.println("Temp: " + String(temperature) + "°C");
  Serial.println("Humidity: " + String(humidity) + " %");
  Serial.println("Gas Level: " + String(gasValue)+"%");
  Serial.println("Distance: " + String(distance) + " cm");

  // Display Smart E-uniform
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("SMART E-UNIFORM");
  lcd.setCursor(0, 1); lcd.print("For SOLDIERS");
  delay(2000);
  
//LCD : Temperature and Humidity
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Temp:" + String(temperature) +" C ");
  lcd.setCursor(0, 1); lcd.print("Humidity:" + String(humidity) + "%");
  delay(3000);
// LCD :Gas and Distance
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Gas level:" + String(gasValue)+"%");
  lcd.setCursor(0, 1); lcd.print("Distance:" + String(distance) + "cm");
  delay(3000);

// 🔁 Hybrid Alert Flags
static bool highTempAlertSent = false;
static bool lowTempAlertSent = false;
static int highTempAlertCount = 0;
static int lowTempAlertCount = 0;
static int lastHighAlertTemp = 0;
static int lastLowAlertTemp = 0;

// ⚠️ Check for high temperature
if (temperature >= 34.0) {
  digitalWrite(RELAY1, LOW);  // Fan ON
  digitalWrite(RELAY, LOW);   // Peltier OFF
  client.publish("soldier", "HIGH TEMPERATURE", true);
  Serial.println("HIGH TEMP: Hydrate Mode on");
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("HIGH TEMP");
  lcd.setCursor(0, 1); lcd.print("Hydrate Mode ON");

  if (!highTempAlertSent || 
      (highTempAlertCount < 4 && temperature >= lastHighAlertTemp + 2)) {
    
    String sms = "🔥 High Temp Alert!\nTemp: " + String(temperature) +
                 "°C\nHumidity: " + String(humidity) +
                 "%\nGas: " + String(gasValue) +
                 "%\nDistance: " + String(distance) + " cm";
sendSMS(sms);
    // Send Email
    EMailSender::EMailMessage message;
    message.subject = "Smart Uniform Alert!";
    message.message = "⚠️ High Temperature Detected!\n\n" +
                      String("Temperature: ") + temperature + " °C\n" +
                      String("Humidity: ") + humidity + " %\n" +
                      String("Gas Level: ") + gasValue + "%\n" +
                      String("Distance: ") + distance + " cm\n";
    EMailSender::Response resp = emailSend.send("p.prakash292002@gmail.com", message);
    if (resp.status) {
  Serial.println("Email sent successfully.");
} else {
  Serial.println("❌ Email failed:");
  Serial.println("Error code: " + String(resp.code));
  Serial.println("Desc: " + resp.desc);
}
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(resp.status ? "Email Sent" : "Email Failed");

    // Send SMS
    //sendSMS(sms);

    highTempAlertSent = true;
    lastHighAlertTemp = (int)temperature;
    highTempAlertCount++;
    delay(3000);
  }

  // Reset low side
  lowTempAlertSent = false;
  lowTempAlertCount = 0;
  lastLowAlertTemp = 0;
}
else if (temperature <= 28.0) {
  digitalWrite(RELAY1, LOW);  // Fan OFF
  digitalWrite(RELAY, HIGH);  // Peltier ON
  client.publish("soldier", "LOW TEMPERATURE", true);
  Serial.println("LOW TEMP: Warm Mode");
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("LOW TEMP");
  lcd.setCursor(0, 1); lcd.print("Warm Mode ON");

  if (!lowTempAlertSent || 
      (lowTempAlertCount < 4 && temperature <= lastLowAlertTemp - 2)) {
    
    String sms = "❄️ Low Temp Alert!\nTemp: " + String(temperature) +
                 "°C\nHumidity: " + String(humidity) +
                 "%\nGas: " + String(gasValue) +
                 "%\nDistance: " + String(distance) + " cm";
  sendSMS(sms);

    // Send Email
    EMailSender::EMailMessage message;
    message.subject = "Smart Uniform Alert!";
    message.message = "⚠️ Low Temperature Detected!\n\n" +
                      String("Temperature: ") + temperature + " °C\n" +
                      String("Humidity: ") + humidity + " %\n" +
                      String("Gas Level: ") + gasValue + "%\n" +
                      String("Distance: ") + distance + " cm\n";
    EMailSender::Response resp = emailSend.send("p.prakash292002@gmail.com", message);
    if (resp.status) {
  Serial.println("Email sent successfully.");
} else {
  Serial.println("❌ Email failed:");
  Serial.println("Error code: " + String(resp.code));
  Serial.println("Desc: " + resp.desc);
}
lcd.setCursor(0,0);
 lcd.print(resp.status ? "Email Sent OK" : "Email Failed");

    // Send SMS
    //sendSMS(sms);

    lowTempAlertSent = true;
    lastLowAlertTemp = (int)temperature;
    lowTempAlertCount++;
    delay(3000);
  }

  // Reset high side
  highTempAlertSent = false;
  highTempAlertCount = 0;
  lastHighAlertTemp = 0;
} 
else {
  // Normal temperature — reset everything
  highTempAlertSent = false;
  lowTempAlertSent = false;
  highTempAlertCount = 0;
  lowTempAlertCount = 0;
  lastHighAlertTemp = 0;
  lastLowAlertTemp = 0;

  digitalWrite(RELAY1, HIGH);  // Fan OFF
  digitalWrite(RELAY, LOW);    // Peltier OFF
}
  // Gas alert
  if (gasValue > 500) {
    client.publish("soldier", "HIGH GAS LEVEL", true);
    lcd.clear(); lcd.setCursor(0, 0); lcd.print("GAS LEVEL HIGH");
  }

  // Send to ThingSpeak
  sendToThingSpeak(temperature, humidity, gasValue, distance);
  delay(15000);// Minimum delay for ThingSpeak
}
