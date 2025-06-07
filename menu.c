#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int stepsPerRevolution = 2048;
Stepper stepper(stepsPerRevolution, 8, 10, 9, 11); // 28BYJ-48 + ULN2003

const int buzzerPin = 7;   // Buzzer on pin 7
const int blueLed = 4;     // Blue LED
const int greenLed = 5;    // Green LED (blinks with buzzer)
const int redLed = 6;      // Red LED

const int yellowButton = 2; // Yellow button (increase)
const int greenButton = 3;  // Green button (decrease)
const int blueButton = 13;  // Blue button (confirm)

int setHour = 0;
int setMinute = 0;

enum SettingState { SELECT_HOUR, SELECT_MINUTE, DONE };
SettingState currentState = SELECT_HOUR;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  pinMode(buzzerPin, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
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

  stepper.setSpeed(15); // RPM
  lcd.clear();
}

void buzzAndBlink(int frequency, int duration) {
  digitalWrite(blueLed, HIGH);     // Turn on blue LED
  tone(buzzerPin, frequency);       // Play tone
  delay(duration);                  // Wait
  noTone(buzzerPin);                // Stop tone
  digitalWrite(blueLed, LOW);      // Turn off blue LED
  delay(100);                       // Short pause between tones
}

void displaySetTime() {
  char buffer[17];
  snprintf(buffer, sizeof(buffer), "Set: %02d:%02d", setHour, setMinute);
  lcd.setCursor(0, 1);
  lcd.print(buffer);
}

void displayMenu() {
  lcd.clear();
  if (currentState == SELECT_HOUR) {
    lcd.print("Escolher a Hora");
    lcd.setCursor(0, 1);
    lcd.print("Hour: ");
    lcd.print(setHour);
  } else if (currentState == SELECT_MINUTE) {
    lcd.print("Escolher o Minuto");
    lcd.setCursor(0, 1);
    lcd.print("Minute: ");
    lcd.print(setMinute);
  }
}

void loop() {
  DateTime now = rtc.now();

  // Display menu based on current state
  displayMenu();

  // Check if the blue button is pressed (confirm set time)
  if (digitalRead(blueButton) == LOW) {
    if (currentState == SELECT_HOUR) {
      currentState = SELECT_MINUTE;  // Move to minute selection
    } else if (currentState == SELECT_MINUTE) {
      lcd.clear();
      lcd.print("Time Set: ");
      displaySetTime();
      delay(1000);
      currentState = DONE;  // Finish setting time
    }
    delay(300); // Debounce delay
  }

  // Handle yellow button press (increase)
  if (digitalRead(yellowButton) == LOW) {
    if (currentState == SELECT_HOUR) {
      setHour = (setHour + 1) % 24;  // Increase hour
    } else if (currentState == SELECT_MINUTE) {
      setMinute = (setMinute + 1) % 60;  // Increase minute
    }
    delay(300);  // Debounce delay
  }

  // Handle green button press (decrease)
  if (digitalRead(greenButton) == LOW) {
    if (currentState == SELECT_HOUR) {
      setHour = (setHour - 1 + 24) % 24;  // Decrease hour (wrap around 24)
    } else if (currentState == SELECT_MINUTE) {
      setMinute = (setMinute - 1 + 60) % 60;  // Decrease minute (wrap around 60)
    }
    delay(300);  // Debounce delay
  }

  // Display current time from RTC
  char buffer[17];
  snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  lcd.setCursor(0, 0);
  lcd.print("                "); // clear line
  lcd.setCursor(0, 0);
  lcd.print("Time:");
  lcd.setCursor(6, 0);
  lcd.print(buffer);

  // Display set time
  displaySetTime();

  // Check if the current time matches the set time to trigger the stepper
  if (now.hour() == setHour && now.minute() == setMinute && now.second() == 0) {
    buzzAndBlink(300, 1000);
    buzzAndBlink(400, 1000);
    buzzAndBlink(500, 1000);

    stepper.step(stepsPerRevolution / 8); // Rotate 45°
    delay(1000); // Prevent double-trigger in the same second
  }

  delay(1000); // Wait before the next loop
}

