// RUN ON ARDUINO IDE
#include <SoftwareSerial.h>
SoftwareSerial mySerial(7, 8);
//                     RX  TX
const int HALL_PIN = 2;
const int LED_PIN = 13;
const unsigned long DEBOUNCE = 500000; // 500 ms
const unsigned long REPORT_INTERVAL_MS = 1000; // 1000 ms

volatile unsigned long pulseCount = 0;
volatile unsigned long lastPulse = 0;
volatile bool magnetDetected = false;
volatile bool magnetPresent = false;

void setup() {
  Serial.begin(9600);
  mySerial.begin(2400);

  pinMode(HALL_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(HALL_PIN), hallISR, CHANGE); // only count LOW state
}

void hallISR(){
  bool currentState = (digitalRead(HALL_PIN) == LOW);
  unsigned long now = micros();

  if(currentState && !magnetPresent){ // magnet present (OUT -> IN)
    if(now - lastPulse > DEBOUNCE){
      pulseCount++;
      lastPulse = now;
      magnetDetected = true;
      magnetPresent = true;
    }
  }
  else if(!currentState){
    magnetPresent = false;
  }
}

void loop() {
  if(magnetDetected){
    digitalWrite(LED_PIN, HIGH);
    magnetDetected = false;
    delay(100); // 100 ms
    digitalWrite(LED_PIN, LOW);
  }
  unsigned long nowMS = millis();
  static unsigned long lastCount = 0;
  static unsigned long lastMS = 0;
  const float circumference = 0.3 * 3.14159; // diameter wheel = 30cm

  if(nowMS - lastMS >= REPORT_INTERVAL_MS){
    noInterrupts();
    unsigned long currentCount = pulseCount;
    interrupts();

    unsigned long delta = currentCount - lastCount;
    if(delta > 0){  // only print when detect magnet
      float rpm = (delta * 60000.0) / (nowMS - lastMS); // hamster cycle per min
      float distance = currentCount * circumference; // meter

      Serial.print("RPM: ");
      Serial.print(rpm);
      Serial.print(", count= ");
      Serial.print(currentCount);
      Serial.print(". Distance: ");
      Serial.print(distance);
      Serial.print("\n============\n");

      mySerial.println(rpm); // output in waveform
    }
    lastMS = nowMS;
    lastCount = currentCount;
  }
}
