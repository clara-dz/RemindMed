#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>
#include <EEPROM.h> // <--- added

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Stepper motor setup
const int stepsPerRevolution = 2048;
Stepper stepper(stepsPerRevolution, 13, 12, 11, 10); // 28BYJ-48 + ULN2003

// Pins
const int buzzerPin = 6;     // Buzzer on pin 6
const int blueLed = 7;       // Blue LED now on pin 7
const int yellowButton = 4;  // Yellow button (hour)
const int greenButton = 3;   // Green button (minute)
const int blueButton = 2;   // Blue button (set)

int setHour = 0;
int setMinute = 0;

void setup() {
  Serial.begin(9600);
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

  // Load saved values from EEPROM
  setHour = EEPROM.read(0);
  setMinute = EEPROM.read(1);

  stepper.setSpeed(15); // RPM
  lcd.clear();
}

void buzzAndBlink(int frequency, int duration) {
  digitalWrite(blueLed, HIGH);     // Turn on blue LED
  tone(buzzerPin, frequency);      // Play tone
  delay(duration);                 // Wait
  noTone(buzzerPin);               // Stop tone
  digitalWrite(blueLed, LOW);      // Turn off blue LED
  delay(100);                      // Short pause between tones
}

void displaySetTime() {
  char buffer[17];
  snprintf(buffer, sizeof(buffer), "Liberacao %02d:%02d", setHour, setMinute);
  lcd.setCursor(0, 1);
  lcd.print(buffer);
}

void loop() {
  DateTime now = rtc.now();

  // Confirm set time and save to EEPROM
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

  // Increase hour
  if (digitalRead(yellowButton) == LOW) {
    setHour = (setHour + 1) % 24;
  }

  // Increase minute
  if (digitalRead(greenButton) == LOW) {
    setMinute = (setMinute + 1) % 60;
  }

  displaySetTime();

  // Display current time
  char buffer[17];
  snprintf(buffer, sizeof(buffer), "%02d:%02d", now.hour(), now.minute());
  lcd.setCursor(0, 0);
  lcd.print("Hora atual ");
  lcd.setCursor(11, 0);
  lcd.print(buffer);

  // Trigger action at the scheduled time
  if (now.hour() == setHour && now.minute() == setMinute && now.second() == 0) {
    buzzAndBlink(300, 1000);
    buzzAndBlink(400, 1000);
    buzzAndBlink(500, 1000);

    stepper.step(stepsPerRevolution / 8); // Rotate 45°
    delay(1000); // Prevent retriggering
  }

  delay(1000);
}
