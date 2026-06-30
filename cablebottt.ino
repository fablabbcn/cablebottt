#define DRVR_PIN1 D9
#define DRVR_PIN2 D10

#define LDR1_PIN A0
#define LDR2_PIN A1

#define N_READINGS 10    // Number of readings to smooth
#define SAMPLE_DELAY 20  // milliseconds before taking another reading

int left_buffer[N_READINGS] = { 0 };
int right_buffer[N_READINGS] = { 0 };

int left_baseline = 0;
int right_baseline = 0;
int left = 0;
int right = 0;

long reading = 0;
long lastReadingTime = 0;

void setup() {
  Serial.begin(115200);

  pinMode(DRVR_PIN1, OUTPUT);
  pinMode(DRVR_PIN2, OUTPUT);

  pinMode(DRVR_PIN1, INPUT);
  pinMode(DRVR_PIN2, INPUT);

  readBaseline();
}

void loop() {

  updateReadings();
}


void readBaseline() {
  for (int i = 0; i < N_READINGS; i++) {
    left_baseline += analogRead(LDR1_PIN);
    right_baseline += analogRead(LDR2_PIN);

    Serial.print(analogRead(LDR1_PIN));
    Serial.print(",");
    Serial.println(analogRead(LDR2_PIN));

    delay(20);
  }

  left_baseline = left_baseline / N_READINGS;
  right_baseline = right_baseline / N_READINGS;

  Serial.println("baseline");

  Serial.print(left_baseline);
  Serial.print(",");
  Serial.println(right_baseline);
}

void updateReadings() {
  long now = millis();

  long elapsed = now - lastReadingTime;

  if (elapsed > SAMPLE_DELAY) {
      left_buffer[reading % N_READINGS] = analogRead(LDR1_PIN);
      right_buffer[reading % N_READINGS] = analogRead(LDR2_PIN);
      reading += 1;
      lastReadingTime = now;

      if (reading >= N_READINGS) {
        left = 0;
        right = 0;

        for (int i = 0; i < N_READINGS; i++) {
          left += left_buffer[i];
          right += right_buffer[i];
        }

        left /= N_READINGS;
        right /= N_READINGS;

        Serial.print("smoothL:");
        Serial.print(left);
        Serial.print(" smoothR:");
        Serial.println(right);
      }
    }
}
