#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address may be 0x3F or 0x27

const int stepsPerRevolution = 2048;
const int stepsPer45Degrees = stepsPerRevolution / 8; // 256 steps

void setup() {
  Serial.begin(9600);
  Wire.begin();

  lcd.begin(16, 2);  // Init LCD with 16 columns and 2 rows
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");

  if (!rtc.begin()) {
    lcd.clear();
    lcd.print("RTC not found!");
    while (1); // halt
  }

  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Optional: only use once

  lcd.clear();
}

void loop() {
  DateTime now = rtc.now();

  char buffer[17];
  snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

  lcd.setCursor(0, 0);
  lcd.print("Time:");
  lcd.setCursor(6, 0);
  lcd.print(buffer);

  delay(1000);
}

