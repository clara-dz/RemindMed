#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#define MAX_ALARMS 5  // adjust as needed
String alarms[MAX_ALARMS];
int alarmCount = 0;


RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Stepper motor setup
const int stepsPerRevolution = 2048;
Stepper stepper(stepsPerRevolution, 13, 12, 11, 10); // 28BYJ-48 + ULN2003

// Pins
const int buzzerPin = 6;
const int blueLed = 7;
const int yellowButton = 4;
const int greenButton = 3;
const int blueButton = 2;

int setHour = 0;
int setMinute = 0;

String bluetoothInput = "";
unsigned long lastDisplayUpdate = 0;
bool displayingBluetooth = false;

void setup() {
  Serial.begin(9600); // HC-06 default baud rate
  Wire.begin();

  pinMode(buzzerPin, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(yellowButton, INPUT_PULLUP);
  pinMode(greenButton, INPUT_PULLUP);
  pinMode(blueButton, INPUT_PULLUP);

  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");

  if (!rtc.begin()) {
    lcd.clear();
    lcd.print("RTC not found!");
    while (1);
  }

  setHour = EEPROM.read(0);
  setMinute = EEPROM.read(1);

  stepper.setSpeed(15);
  lcd.clear();
}

void buzzAndBlink(int frequency, int duration) {
  digitalWrite(blueLed, HIGH);
  tone(buzzerPin, frequency);
  delay(duration);
  noTone(buzzerPin);
  digitalWrite(blueLed, LOW);
  delay(100);
}

void displaySetTime() {
  char buffer[17];
  snprintf(buffer, sizeof(buffer), "Liberacao %02d:%02d", setHour, setMinute);
  lcd.setCursor(0, 1);
  lcd.print(buffer);
}

void checkBluetoothInput() {
  while (Serial.available()) {
    char c = Serial.read();
    
if (c == '\n') {
  Serial.println("Received raw:");
  Serial.println(bluetoothInput);

  // 🔧 Strip everything before '{'
  int jsonStart = bluetoothInput.indexOf('{');
  if (jsonStart >= 0) {
    bluetoothInput = bluetoothInput.substring(jsonStart);
  } else {
    Serial.println("No JSON object found, skipping.");
    bluetoothInput = "";
    return;
  }

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, bluetoothInput);

  if (error) {
    Serial.print("JSON parse error: ");
    Serial.println(error.f_str());
    bluetoothInput = "";
    return;
  }

  // proceed to extract 'dispense' and 'alarms'...


      // ✔ DISPENSE
      const char* dispenseTime = doc["dispense"];
      if (dispenseTime && strlen(dispenseTime) == 5 && dispenseTime[2] == ':') {
        int newHour = atoi(String(dispenseTime).substring(0, 2).c_str());
        int newMinute = atoi(String(dispenseTime).substring(3, 5).c_str());

        Serial.print("Parsed dispense: ");
        Serial.println(dispenseTime);
        Serial.print("Hour: "); Serial.println(newHour);
        Serial.print("Minute: "); Serial.println(newMinute);

        setHour = newHour;
        setMinute = newMinute;
        EEPROM.write(0, setHour);
        EEPROM.write(1, setMinute);
      } else {
        Serial.println("Invalid or missing 'dispense' field.");
      }

      // ✔ ALARMS
      JsonArray alarmArray = doc["alarms"];
      alarmCount = 0;
      for (JsonVariant v : alarmArray) {
        if (alarmCount < MAX_ALARMS && v.is<const char*>()) {
          alarms[alarmCount++] = v.as<String>();
          Serial.print("Alarm ");
          Serial.print(alarmCount);
          Serial.print(": ");
          Serial.println(alarms[alarmCount - 1]);
        }
      }

      // ✔ DISPLAY FEEDBACK
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("BT Updated!");
      lcd.setCursor(0, 1);
      lcd.print("Time ");
      lcd.print(setHour < 10 ? "0" : "");
      lcd.print(setHour);
      lcd.print(":");
      lcd.print(setMinute < 10 ? "0" : "");
      lcd.print(setMinute);

      displayingBluetooth = true;
      lastDisplayUpdate = millis();
      bluetoothInput = "";
    } else {
      bluetoothInput += c;
    }
  }
}

void loop() {
  checkBluetoothInput();

  if (displayingBluetooth && millis() - lastDisplayUpdate < 5000) {
    // Show the message for 5 seconds, then resume normal display
    return;
  } else {
    displayingBluetooth = false;
  }

  DateTime now = rtc.now();

  if (digitalRead(blueButton) == LOW) {
    EEPROM.write(0, setHour);
    EEPROM.write(1, setMinute);
    lcd.clear();
    lcd.print("Horario para");
    lcd.setCursor(0, 1);
    lcd.print("liberacao OK");
    delay(2000);
    return;
  }

  if (digitalRead(yellowButton) == LOW) {
    setHour = (setHour + 1) % 24;
  }

  if (digitalRead(greenButton) == LOW) {
    setMinute = (setMinute + 1) % 60;
  }

  displaySetTime();

  char buffer[17];
  snprintf(buffer, sizeof(buffer), "%02d:%02d", now.hour(), now.minute());
  lcd.setCursor(0, 0);
  lcd.print("Hora atual ");
  lcd.setCursor(11, 0);
  lcd.print(buffer);

  if (now.hour() == setHour && now.minute() == setMinute && now.second() == 0) {
    buzzAndBlink(300, 1000);
    buzzAndBlink(400, 1000);
    buzzAndBlink(500, 1000);
    stepper.step(stepsPerRevolution / 8);
    delay(1000);
  }

  delay(1000);
}

