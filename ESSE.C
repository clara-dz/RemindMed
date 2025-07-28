#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>

// LCD and RTC
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;

// Stepper setup
const int stepsPerRevolution = 2048;
Stepper stepper(stepsPerRevolution, 13, 12, 11, 10);

// Pins
const int buzzerPin = 6;
const int blueLed = 7;
const int yellowButton = 4;  // Hour+
const int greenButton = 3;   // Minute+
const int blueButton = 2;    // Confirm

// Time config
int setHour = 7;
int setMinute = 0;
bool alreadyDispensed = false;

void buzzAndBlink(int times, int duration, int frequency = 1000) {
  for (int i = 0; i < times; i++) {
    digitalWrite(blueLed, HIGH);
    tone(buzzerPin, frequency);
    delay(duration);
    digitalWrite(blueLed, LOW);
    noTone(buzzerPin);
    delay(duration);
  }
}

void setup() {
  Serial.begin(9600);
  Wire.begin();

  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Init...");

  if (!rtc.begin()) {
    lcd.setCursor(0, 1);
    lcd.print("RTC fail!");
    while (1);
  }

  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // Only once

  stepper.setSpeed(10);

  pinMode(buzzerPin, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(yellowButton, INPUT_PULLUP);
  pinMode(greenButton, INPUT_PULLUP);
  pinMode(blueButton, INPUT_PULLUP);

  delay(2000);
  lcd.clear();
}

void loop() {
  DateTime now = rtc.now();

  // Show current time
  char timeStr[17];
  snprintf(timeStr, sizeof(timeStr), "Time %02d:%02d:%02d",
           now.hour(), now.minute(), now.second());
  lcd.setCursor(0, 0);
  lcd.print(timeStr);

  // Show configured dispense time
  char configStr[17];
  snprintf(configStr, sizeof(configStr), "Dose at %02d:%02d",
           setHour, setMinute);
  lcd.setCursor(0, 1);
  lcd.print(configStr);

  // Button input: increment hour
  if (digitalRead(yellowButton) == LOW) {
    delay(200);  // debounce
    setHour = (setHour + 1) % 24;
  }

  // Button input: increment minute
  if (digitalRead(greenButton) == LOW) {
    delay(200);
    setMinute = (setMinute + 1) % 60;
  }

  // Button input: confirm and buzz
  if (digitalRead(blueButton) == LOW) {
    delay(200);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Time saved:");
    lcd.setCursor(0, 1);
    lcd.print(setHour < 10 ? "0" : "");
    lcd.print(setHour);
    lcd.print(":");
    lcd.print(setMinute < 10 ? "0" : "");
    lcd.print(setMinute);
    buzzAndBlink(2, 200);
    delay(2000);
    lcd.clear();
  }

  // Dispense if current time matches
  if (now.hour() == setHour && now.minute() == setMinute) {
    if (!alreadyDispensed && now.second() == 0) {
      lcd.setCursor(0, 1);
      lcd.print("Dispensing...   ");
      buzzAndBlink(3, 300);
      stepper.step(256);  // 45 degrees
      lcd.setCursor(0, 1);
      lcd.print("Done rotating   ");
      alreadyDispensed = true;
    }
  } else {
    alreadyDispensed = false;  // reset flag if time doesn't match
  }

  delay(300);
}

