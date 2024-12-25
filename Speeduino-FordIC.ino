#include <math.h>

#define tempPin A6 //from engine temp sensor
#define wheelPin 2 //from speed hall sensor
#define enginePin 3 //from speeduino

#define DEBUG false //true makes it output set data on line 11, false takes interrupts

volatile long wheelCount = 0, engineCount = 0;
float speedCorrDial = 0.97; //data to cluster is ok but dial is off
int kph = 55,rpm = 3500,temp = 90,runningOdo = 0,t1000run = 1;
unsigned long t1000,t50;
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
  runningOdo+=kph/10; //odo calc
  if(runningOdo > 255) { runningOdo=0; }
  sprintf(buff, "02%02X", runningOdo); //odo msg
  sendSCP("A1 7B 10",buff);
}

void sendSPEED() {
  //kph = (((((wheelCount*20)/28)*1872)*0.0036)+kph)/2;  nebo 24 místo 28?
  //191 rpm at 373cm circ (59.4cm dia 195/55/R15) = 42.7kmh
  //78hz / 24 pulses per rev * 373cm circ
  //nemá to být = (((wheelCount*20)/24)*373)*0.036 = 43.6kmh
  //((11.19*wheelCount)+kph)/2
  if(!DEBUG) {kph = ((11.19*wheelCount)+kph)/2;}
  wheelCount = 0;
  sprintf(buff, "02%04lX", (long unsigned int)128*kph); //speed msg
  sendSCP("A1 29 10",buff);
}

void sendRPM() {
  if(!DEBUG) {rpm = ((engineCount*1200)+rpm)/2;}
  engineCount = 0;
  sprintf(buff, "25%06lXF82C", (long unsigned int)1024*rpm); //rpm msg
  sendSCP("81 1B 10",buff);
}

void wheelRise() {
  wheelCount++;
}
void engineRise() {
  engineCount++;
}

void setup() {
  pinMode(wheelPin, INPUT_PULLUP);
  pinMode(enginePin, INPUT_PULLUP);
  pinMode(tempPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(wheelPin), wheelRise, RISING);
  attachInterrupt(digitalPinToInterrupt(enginePin), engineRise, RISING);
  Serial.begin(115200);
  delay(3000);
  for(int y = 0;y<64;y++) { Serial.read(); }
  Serial.println("AT Z"); //restart
  readWait();
  Serial.println("AT SP 1"); //long msgs
  readWait();
  Serial.println("AT E0"); //echo off
  readWait();
  Serial.println("AT R0"); //recieve off
  readWait();
  delay(1000);
  sendSCP("41 88 10","02"); //Transmission off
  sendSCP("41 73 10","1420"); //Alternator off
}

void loop() {
  if(millis()> t50+50) {
    if(millis()> t1000+1000 && t1000run == 1) {
      if(!DEBUG) {temp = -31.031*log(0.0188039*(2.49/(((float)1023/analogRead(tempPin))-1)));}
      sprintf(buff, "10%02X", 40+temp);
      sendSCP("81 49 10",buff); // Temp data
      //-----------
      delay(5);
      sendODO();
      //-----------
      delay(5);
      sendSPEED();
      //-----------
      delay(5);
      sendRPM();

      t1000run=2;
    } else if(millis()> t1000+1000 && t1000run == 2) {
      sendSCP("81 88 10","3001"); //CEL Off
      //-----------
      delay(5);
      sendODO();
      //-----------
      delay(5);
      sendSPEED();
      //-----------
      delay(5);
      sendRPM();

      t1000run=1;
      t1000=millis();
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
