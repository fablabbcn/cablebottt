#define DRVR_PIN1 D9
#define DRVR_PIN2 D10

#define BUZZER_PIN D6

#define LDR1_PIN A1
#define LDR2_PIN A0

#define N_READINGS 5
#define SAMPLE_DELAY 10

const int ACTIVATION_THRESHOLD = 200;
const int DIFFERENCE_DEADBAND = 80;

int leftBuffer[N_READINGS] = { 0 };
int rightBuffer[N_READINGS] = { 0 };

int leftBaseline = 0;
int rightBaseline = 0;

int left = 0;
int right = 0;

unsigned long readingCount = 0;
unsigned long lastReadingTime = 0;

void setup() {
  pinMode(DRVR_PIN1, OUTPUT);
  pinMode(DRVR_PIN2, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(DRVR_PIN1, LOW);
  digitalWrite(DRVR_PIN2, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.begin(115200);

  stopMotor();

  // Allow the power supply, ADC and sensors to stabilise
  delay(1000);

  readBaseline();
  initialiseReadingBuffer();

}

void loop() {
  updateReadings();

  if (readingCount < N_READINGS) {
    stopMotor();
    return;
  }

  actOnMotor();
}

void actOnMotor() {
  int leftInput = leftBaseline - left;
  int rightInput = rightBaseline - right;

  leftInput = max(leftInput, 0);
  rightInput = max(rightInput, 0);

  int strongestInput = max(leftInput, rightInput);
  int difference = leftInput - rightInput;

  if (strongestInput < ACTIVATION_THRESHOLD ||
      abs(difference) < DIFFERENCE_DEADBAND) {
    stopMotor();
    return;
  }

  int strength = constrain(abs(difference) / 8, 0, 255);

  if (difference > 0) {
    analogWrite(DRVR_PIN2, 0);
    digitalWrite(DRVR_PIN2, LOW);
    analogWrite(DRVR_PIN1, strength);
  } else {
    analogWrite(DRVR_PIN1, 0);
    digitalWrite(DRVR_PIN1, LOW);
    analogWrite(DRVR_PIN2, strength);
  }
}

void stopMotor() {
  analogWrite(DRVR_PIN1, 0);
  analogWrite(DRVR_PIN2, 0);

  digitalWrite(DRVR_PIN1, LOW);
  digitalWrite(DRVR_PIN2, LOW);
}

void readBaseline() {
  long leftTotal = 0;
  long rightTotal = 0;

  stopMotor();

  for (int i = 0; i < N_READINGS; i++) {
    int leftReading = analogRead(LDR1_PIN);
    int rightReading = analogRead(LDR2_PIN);

    leftTotal += leftReading;
    rightTotal += rightReading;

    Serial.print(leftReading);
    Serial.print(",");
    Serial.println(rightReading);

    delay(20);
  }

  leftBaseline = leftTotal / N_READINGS;
  rightBaseline = rightTotal / N_READINGS;

  left = leftBaseline;
  right = rightBaseline;

  Serial.print("Baseline L: ");
  Serial.print(leftBaseline);
  Serial.print(" R: ");
  Serial.println(rightBaseline);
}

void initialiseReadingBuffer() {
  for (int i = 0; i < N_READINGS; i++) {
    leftBuffer[i] = leftBaseline;
    rightBuffer[i] = rightBaseline;
  }

  left = leftBaseline;
  right = rightBaseline;

  readingCount = N_READINGS;
  lastReadingTime = millis();
}

void updateReadings() {
  unsigned long now = millis();

  if (now - lastReadingTime < SAMPLE_DELAY) {
    return;
  }

  lastReadingTime = now;

  int index = readingCount % N_READINGS;

  leftBuffer[index] = analogRead(LDR1_PIN);
  rightBuffer[index] = analogRead(LDR2_PIN);

  readingCount++;

  long leftTotal = 0;
  long rightTotal = 0;

  for (int i = 0; i < N_READINGS; i++) {
    leftTotal += leftBuffer[i];
    rightTotal += rightBuffer[i];
  }

  left = leftTotal / N_READINGS;
  right = rightTotal / N_READINGS;

  Serial.print("L:");
  Serial.print(left);
  Serial.print(" R:");
  Serial.print(right);
  Serial.print(" DL:");
  Serial.print(leftBaseline - left);
  Serial.print(" DR:");
  Serial.println(rightBaseline - right);
}