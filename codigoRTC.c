#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;  // or use RTC_DS1307 rtc; for DS1307

void setup () {
  Serial.begin(9600);
  if (!rtc.begin()) {
    Serial.println("❌ Couldn't find RTC");
    while (1);
  }

  // Uncomment this to set the RTC to the current time the sketch was compiled
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  Serial.println("✅ Time set!");
}

void loop () {
  DateTime now = rtc.now();

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  delay(1000);
}
