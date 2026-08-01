#define MOTOR_PIN_1 D9
#define MOTOR_PIN_2 D10

#define BUZZER_PIN D6

#define LEFT_LDR_PIN A1
#define RIGHT_LDR_PIN A0

#define FILTER_SAMPLES 5
#define CALIBRATION_SAMPLES 25

const unsigned long SAMPLE_INTERVAL_MS = 10;
const unsigned long PRINT_INTERVAL_MS = 100;

const int ACTIVATION_THRESHOLD = 200;
const int DIFFERENCE_DEADBAND = 80;
const int MIN_MOTOR_SPEED = 70;

int leftBuffer[FILTER_SAMPLES] = {0};
int rightBuffer[FILTER_SAMPLES] = {0};

int leftBaseline = 0;
int rightBaseline = 0;

int leftReading = 0;
int rightReading = 0;

unsigned long sampleCount = 0;
unsigned long lastSampleTime = 0;
unsigned long lastPrintTime = 0;

void setup() {
  pinMode(MOTOR_PIN_1, OUTPUT);
  pinMode(MOTOR_PIN_2, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(MOTOR_PIN_1, LOW);
  digitalWrite(MOTOR_PIN_2, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.begin(115200);

  analogReadResolution(12);

  stopMotor();

  // Let the power supply and sensors stabilise.
  delay(1000);

  calibrateSensors();
  initialiseFilter();
}

void loop() {
  updateSensorReadings();

  if (sampleCount < FILTER_SAMPLES) {
    stopMotor();
    return;
  }

  controlMotor();
}

void controlMotor() {
  // Compare each live reading with its ambient-light baseline.
  // With this LDR circuit, more light produces a lower reading.
  int leftLightIncrease = leftBaseline - leftReading;
  int rightLightIncrease = rightBaseline - rightReading;

  // Ignore changes caused by the environment becoming darker.
  leftLightIncrease = max(leftLightIncrease, 0);
  rightLightIncrease = max(rightLightIncrease, 0);

  // Find the strongest light change detected by either sensor.
  int strongestLightIncrease =
    max(leftLightIncrease, rightLightIncrease);

  // A positive difference means the left sensor sees more light.
  // A negative difference means the right sensor sees more light.
  int lightDifference =
    leftLightIncrease - rightLightIncrease;

  // Stop if neither sensor sees a sufficiently strong light change.
  if (strongestLightIncrease < ACTIVATION_THRESHOLD) {
    stopMotor();
    return;
  }

  // Stop if both sensors see approximately the same amount of light.
  if (abs(lightDifference) < DIFFERENCE_DEADBAND) {
    stopMotor();
    return;
  }

  // A larger difference between the sensors produces a higher speed.
  int motorSpeed = constrain(
    abs(lightDifference) / 8,
    MIN_MOTOR_SPEED,
    255
  );

  // Move towards the sensor receiving the stronger light.
  if (lightDifference > 0) {
    driveLeft(motorSpeed);
  } else {
    driveRight(motorSpeed);
  }
}

void driveLeft(int motorSpeed) {
  analogWrite(MOTOR_PIN_2, 0);
  digitalWrite(MOTOR_PIN_2, LOW);

  analogWrite(MOTOR_PIN_1, motorSpeed);
}

void driveRight(int motorSpeed) {
  analogWrite(MOTOR_PIN_1, 0);
  digitalWrite(MOTOR_PIN_1, LOW);

  analogWrite(MOTOR_PIN_2, motorSpeed);
}

void stopMotor() {
  analogWrite(MOTOR_PIN_1, 0);
  analogWrite(MOTOR_PIN_2, 0);

  digitalWrite(MOTOR_PIN_1, LOW);
  digitalWrite(MOTOR_PIN_2, LOW);
}

void calibrateSensors() {
  long leftTotal = 0;
  long rightTotal = 0;

  stopMotor();

  Serial.println("Calibrating LDR sensors...");

  for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
    leftTotal += analogRead(LEFT_LDR_PIN);
    rightTotal += analogRead(RIGHT_LDR_PIN);

    delay(20);
  }

  leftBaseline = leftTotal / CALIBRATION_SAMPLES;
  rightBaseline = rightTotal / CALIBRATION_SAMPLES;

  leftReading = leftBaseline;
  rightReading = rightBaseline;

  Serial.print("Left baseline: ");
  Serial.println(leftBaseline);

  Serial.print("Right baseline: ");
  Serial.println(rightBaseline);
}

void initialiseFilter() {
  for (int i = 0; i < FILTER_SAMPLES; i++) {
    leftBuffer[i] = leftBaseline;
    rightBuffer[i] = rightBaseline;
  }

  leftReading = leftBaseline;
  rightReading = rightBaseline;

  sampleCount = FILTER_SAMPLES;
  lastSampleTime = millis();
}

void updateSensorReadings() {
  unsigned long now = millis();

  if (now - lastSampleTime < SAMPLE_INTERVAL_MS) {
    return;
  }

  lastSampleTime = now;

  int bufferIndex = sampleCount % FILTER_SAMPLES;

  leftBuffer[bufferIndex] = analogRead(LEFT_LDR_PIN);
  rightBuffer[bufferIndex] = analogRead(RIGHT_LDR_PIN);

  sampleCount++;

  long leftTotal = 0;
  long rightTotal = 0;

  for (int i = 0; i < FILTER_SAMPLES; i++) {
    leftTotal += leftBuffer[i];
    rightTotal += rightBuffer[i];
  }

  leftReading = leftTotal / FILTER_SAMPLES;
  rightReading = rightTotal / FILTER_SAMPLES;

  printSensorReadings();
}

void printSensorReadings() {
  unsigned long now = millis();

  if (now - lastPrintTime < PRINT_INTERVAL_MS) {
    return;
  }

  lastPrintTime = now;

  Serial.print("Left: ");
  Serial.print(leftReading);

  Serial.print("  Right: ");
  Serial.print(rightReading);

  Serial.print("  Left change: ");
  Serial.print(leftBaseline - leftReading);

  Serial.print("  Right change: ");
  Serial.println(rightBaseline - rightReading);
}
