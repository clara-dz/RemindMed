#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int stepsPerRevolution = 2048;
Stepper stepper(stepsPerRevolution, 8, 10, 9, 11); // 28BYJ-48 + ULN2003

void setup() {
  Serial.begin(9600);
  Wire.begin();

  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");

  if (!rtc.begin()) {
    lcd.clear();
    lcd.print("RTC not found!");
    while (1);
  }

  // Optional time setting
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  stepper.setSpeed(15); // RPM
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

  // Example: Rotate 45° at specific time
  if (now.second() == 0 || now.second()%10 == 0) { // a cada 10s
    stepper.step(stepsPerRevolution / 8); // 45°
    delay(1000); // prevent re-triggering in the same second
  }

  delay(1000);
}

