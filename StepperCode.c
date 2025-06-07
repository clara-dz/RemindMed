#include <Stepper.h>

// Steps per revolution (for 28BYJ-48 with gear reduction)
const int stepsPerRevolution = 2048;
const int stepsPer45Degrees = stepsPerRevolution / 8; // 256 steps

// Initialize the stepper (IN1-IN4 on pins 8-11)
Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);

void setup() {
  myStepper.setSpeed(10); // Adjust RPM as needed
}

void loop() {
  myStepper.step(stepsPer45Degrees); // Rotate 45 degrees
  delay(5000);                      // Wait 30 seconds
}
