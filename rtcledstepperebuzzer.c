#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int stepsPerRevolution = 2048;
Stepper stepper(stepsPerRevolution, 8, 10, 9, 11); // 28BYJ-48 + ULN2003

const int buzzerPin = 7;  // Buzzer connected to digital pin 7

void setup() {
  Serial.begin(9600);
  Wire.begin();

  pinMode(buzzerPin, OUTPUT);  // Set buzzer pin as output

  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");

  if (!rtc.begin()) {
    lcd.clear();
    lcd.print("RTC not found!");
    while (1);
  }

  stepper.setSpeed(15); // RPM
  lcd.clear();
}

void loop() {
  DateTime now = rtc.now();

  char buffer[17];
  snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

  lcd.setCursor(0, 0);
  lcd.print("                "); // clear line
  lcd.setCursor(0, 0);
  lcd.print("Time:");
  lcd.setCursor(6, 0);
  lcd.print(buffer);

  // Rotate and buzz every 10 seconds
  if (now.second() % 10 == 0) {
    // 🔊 Buzz using tone (required for passive buzzer)
    tone(buzzerPin, 300);   // 1kHz tone
    delay(1000);              // play for 2000 ms
   // noTone(buzzerPin);

    tone(buzzerPin, 400);   // 1kHz tone
    delay(1000);              // play for 2000 ms
  //  noTone(buzzerPin);

    tone(buzzerPin, 500);   // 1kHz tone
    delay(1000);              // play for 2000 ms
    noTone(buzzerPin);


    stepper.step(stepsPerRevolution / 8); // Rotate 45°
    delay(1000); // Prevent double-trigger in the same second
  }

  delay(1000);

}

