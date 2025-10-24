// AccelStepper object: DRIVER mode uses STEP + DIR
#include <Arduino.h>
static AccelStepper stepper(AccelStepper::DRIVER, MOTOR_STEP_PIN, MOTOR_DIR_PIN);

#define STEPPER_PWM_CHANNEL 0
#define STEPPER_PWM_TIMER 0
#define STEPPER_PWM_RESOLUTION 10 // 10 bits, 0-1023
#define STEPPER_PWM_MAX_FREQ 2500

// Internal state
static bool motorActive = false;
static bool motorStopping = false;
static unsigned long motorStartMillis = 0;
static unsigned long motorDuration = 0; // ms, 0 = infinite
static int motorDirSign = 1;
static float motorRequestedSpeed = 0.0f;

void motorInit() {
  pinMode(MOTOR_ENABLE_PIN, OUTPUT);
  pinMode(MOTOR_DIR_PIN, OUTPUT);
  digitalWrite(MOTOR_ENABLE_PIN, HIGH); // disable outputs (active LOW typical)

  // The new ledcAttach function combines setup and pin attachment.
  // It configures the channel, frequency, and resolution, then attaches the pin.
  ledcAttach(MOTOR_STEP_PIN, STEPPER_PWM_MAX_FREQ, STEPPER_PWM_RESOLUTION);
}

void spinMotorStop() {
  if (!motorActive && !motorStopping)
    return;

  // Stop the PWM signal by setting duty cycle to 0
  ledcWrite(STEPPER_PWM_CHANNEL, 0);
  digitalWrite(MOTOR_ENABLE_PIN, HIGH); // Disable driver

  motorActive = false;
  motorStopping = false;
}

void spinMotor(short speed, bool reverse, unsigned long durationMs) {
  if (speed <= 0) {
    spinMotorStop();
    return;
  }

  motorRequestedSpeed = (float)constrain(speed, 0, 255) * (STEPPER_PWM_MAX_FREQ / 255.0f);
  motorDirSign = reverse ? -1 : 1;
  motorDuration = durationMs;
  motorStartMillis = millis();
  motorActive = true;
  motorStopping = false;

  // Set direction
  digitalWrite(MOTOR_DIR_PIN, reverse ? HIGH : LOW);
  // Enable driver
  digitalWrite(MOTOR_ENABLE_PIN, LOW);

  // Calculate frequency based on requested speed (steps/sec)
  int freq = (int)motorRequestedSpeed;
  if (freq > STEPPER_PWM_MAX_FREQ) {
    freq = STEPPER_PWM_MAX_FREQ;
  }

  // This single call configures the frequency and resolution for the channel.
  ledcAttach(MOTOR_STEP_PIN, freq, STEPPER_PWM_RESOLUTION);

  // Set PWM duty cycle to 50% to create a square wave.
  ledcWrite(MOTOR_STEP_PIN, (1 << STEPPER_PWM_RESOLUTION) / 2);
}

void motorTask() {
  // If a duration was specified and elapsed -> request stop
  if (motorActive && motorDuration > 0) {
    if ((unsigned long)(millis() - motorStartMillis) >= motorDuration) {
      spinMotorStop();
    }
  }
}