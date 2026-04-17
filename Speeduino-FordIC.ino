#include <math.h>

#define tempPin A6 //from engine temp sensor
#define wheelPin 2 //from speed hall sensor
#define enginePin 3 //from speeduino

#define DEBUG false //true makes it output set data on line 11, false takes interrupts

volatile unsigned long lastEnginePulse = 0, enginePeriod = 0;
volatile unsigned long lastWheelPulse = 0, wheelPeriod = 0;

float speedCorrDial = 0.97;
int kph = 0, rpm = 0, temp = 90, runningOdo = 0, t1000run = 1;
unsigned long t1000, t100, t50;
char buff[10];

void readWait() { //continuous loop read of serial buffer till ELM responds with '>'
  bool test = true;
  while(test){
    while(Serial.available() > 0){
      if(Serial.read() == '>') {
        test = false;
      }
      Serial.read();
    }
  }
  delayMicroseconds(100);
}

void sendSCP(String header, const char* data) {
  Serial.println("AT SH " + header);
  readWait();
  Serial.println(data);
  readWait();
}

void sendODO() {
  runningOdo += kph / 10;
  if(runningOdo > 255) { runningOdo = 0; }
  sprintf(buff, "02%02X", runningOdo);
  sendSCP("A1 7B 10", buff);
}

void sendSPEED() {
  if(!DEBUG) {
    unsigned long period;
    noInterrupts();
    period = wheelPeriod;
    interrupts();

    if(period > 0) {
      kph = (262000.0 * speedCorrDial) / period;
    } else {
      kph = 0;
    }
  }
  sprintf(buff, "02%04lX", (long unsigned int)128 * kph);
  sendSCP("A1 29 10", buff);
}

void sendRPM() {
  if(!DEBUG) {
    unsigned long period;
    noInterrupts();
    period = enginePeriod;
    interrupts();

    if(period > 0) {
      rpm = 60000000UL / period;
    } else {
      rpm = 0;
    }
  }
  sprintf(buff, "25%06lXF82C", (long unsigned int)1024 * rpm);
  sendSCP("81 1B 10", buff);
}

void wheelRise() {
  unsigned long now = micros();
  if(lastWheelPulse > 0) {
    wheelPeriod = now - lastWheelPulse;
  }
  lastWheelPulse = now;
}

void engineRise() {
  unsigned long now = micros();
  if(lastEnginePulse > 0) {
    enginePeriod = now - lastEnginePulse;
  }
  lastEnginePulse = now;
}

void setup() {
  pinMode(wheelPin, INPUT_PULLUP);
  pinMode(enginePin, INPUT_PULLUP);
  pinMode(tempPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(wheelPin), wheelRise, RISING);
  attachInterrupt(digitalPinToInterrupt(enginePin), engineRise, RISING);
  Serial.begin(115200);
  delay(3000);
  for(int y = 0; y < 64; y++) { Serial.read(); }
  Serial.println("AT Z"); //restart
  readWait();
  Serial.println("AT SP 1"); //long msgs
  readWait();
  Serial.println("AT E0"); //echo off
  readWait();
  Serial.println("AT R0"); //recieve off
  readWait();
  delay(1000);
  sendSCP("41 88 10", "02"); // Transmission off
  sendSCP("41 73 10", "1420"); // Alternator off
}

void loop() {
  // Zero out RPM and speed if no pulse received for >500ms (engine/wheel stopped)
  if(millis() > t100 + 500) {
    noInterrupts();
    unsigned long now = micros();
    if(lastEnginePulse > 0 && (now - lastEnginePulse) > 500000UL) {
      enginePeriod = 0;
      rpm = 0;
    }
    if(lastWheelPulse > 0 && (now - lastWheelPulse) > 500000UL) {
      wheelPeriod = 0;
      kph = 0;
    }
    interrupts();
    t100 = millis();
  }

  // Send all cluster messages every 50ms
  if(millis() > t50 + 50) {
    if(millis() > t1000 + 1000 && t1000run == 1) {
      if(!DEBUG) { temp = -31.031 * log(0.0188039 * (2.49 / (((float)1023 / analogRead(tempPin)) - 1))); }
      sprintf(buff, "10%02X", 40 + temp);
      sendSCP("81 49 10", buff); // Temp
      //-----------
      delay(5);
      sendODO();
      //-----------
      delay(5);
      sendSPEED();
      //-----------
      delay(5);
      sendRPM();
      
      t1000run = 2;
    } else if(millis() > t1000 + 1000 && t1000run == 2) {
      sendSCP("81 88 10", "3001"); // CEL Off
      //-----------
      delay(5);
      sendODO();
      //-----------
      delay(5);
      sendSPEED();
      //-----------
      delay(5);
      sendRPM();

      t1000run = 1;
      t1000 = millis();
    } else {
      sendODO();
      //-----------
      delay(5);
      sendSPEED();
      //-----------
      delay(5);
      sendRPM();
    }
    t50 = millis();
  }
}
