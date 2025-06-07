
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>
#include <SoftwareSerial.h>

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// HC-06 on pins 10 (RX), 11 (TX)
SoftwareSerial bluetooth(0, 1);  // RX, TX

const int stepsPerRevolution = 2048;
Stepper stepper(stepsPerRevolution, 8, 10, 9, 11); // 28BYJ-48 + ULN2003

const int buzzerPin = 7;   // Buzzer on pin 7
const int blueLed = 4;     // Blue LED
const int greenLed = 5;    // Green LED (blinks with buzzer)
const int redLed = 6;      // Red LED

const int yellowButton = 2; // Yellow button (hour)
const int greenButton = 3;  // Green button (minute)
const int blueButton = 13;  // Blue button (set)

int setHour = 0;
int setMinute = 0;

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
  Serial.println("Bluetooth pronto"); // Confirm via Bluetooth
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
  snprintf(buffer, sizeof(buffer), "Liberacao %02d:%02d", setHour, setMinute);
  lcd.setCursor(0, 1);
  lcd.print(buffer);
}

void loop() {

  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();

    Serial.println("Recebido: " + msg); // Echo back via Bluetooth

    if (msg == "ping") {
      Serial.println("pong");           // Optional test
    }
  }
  
  DateTime now = rtc.now();

  // Check if the blue button is pressed (confirm set time)
    if (digitalRead(blueButton) == LOW) {
    lcd.clear();
    lcd.print("Horario para");
    lcd.setCursor(0, 1);
    lcd.print("liberacao OK");
    delay(2000);
    return; // Exit the loop after setting
  }

  //aumentar horas
  if (digitalRead(yellowButton) == LOW) {
    setHour = (setHour + 1) % 24; // Increment hour (wrap around 24)
    // delay(300); // Debounce delay
  }

  // aumentar minutos
  if (digitalRead(greenButton) == LOW) {
    setMinute = (setMinute + 1) % 60; // Increment minute (wrap around 60)
   // delay(300); // Debounce delay
  }

     displaySetTime();

  // Display current time from RTC
  char buffer[17];
  snprintf(buffer, sizeof(buffer), "%02d:%02d", now.hour(), now.minute());
  //lcd.setCursor(0, 0);
  //lcd.print("                "); // clear line
  lcd.setCursor(0, 0);
  lcd.print("Hora atual ");
  lcd.setCursor(11, 0);
  lcd.print(buffer);

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
