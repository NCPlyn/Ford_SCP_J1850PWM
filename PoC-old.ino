#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX connected to ELM327

void setup() {
  Serial.begin(115200);
  mySerial.begin(115200);
  mySerial.println("AT WS"); //restart
  delay(200);
  mySerial.println("AT AL"); //long msgs
  delay(20);
  mySerial.println("AT E0"); //echo off
  delay(20);
  mySerial.println("AT R0"); //recieve off
  delay(20);
  mySerial.println("AT SH 41 88 10"); //turn off gear
  delay(20);
  mySerial.println("02");
  delay(20);
  mySerial.println("AT SH 41 73 10"); //turn alternator off
  delay(20);
  mySerial.println("1420");
  delay(20);
}

float speedCorrDial = 0.97; //data to cluster is ok but dial is off
int kph = (int)80*speedCorrDial;
int rpm = 1255;
int temp = 90;

unsigned long t1000,t50;
char buff[8];
int x = 0;
void loop() {
  if(millis() > t50 + 50) { //50ms
    mySerial.println("AT SH 81 1B 10"); //rpm header
    delay(2);
    sprintf(buff, "25%06lX0000", (long unsigned int)1024*rpm); //rpm msg
    mySerial.println(buff);
    delay(5);
    mySerial.println("AT SH A1 7B 10"); //odo header
    delay(2);
    x+=kph/10; //odo calc
    if(x > 255) { x=0; }
    sprintf(buff, "02%02X", x); //odo msg
    mySerial.println(buff);
    delay(5);
    mySerial.println("AT SH A1 29 10"); //speed header
    delay(2);
    sprintf(buff, "02%04lX", (long unsigned int)128*kph); //speed msg
    mySerial.println(buff);
    t50 = millis();
    delay(5);
  }
  if(millis() > t1000 + 1000) { //1000ms
    mySerial.println("AT SH 81 88 10"); //CEL header
    delay(2);
    mySerial.println("3001"); //turn off CEL
    delay(5);
    mySerial.println("AT SH 81 49 10"); //temp header
    delay(2);
    sprintf(buff, "10%02X", 40+temp); //temp msg
    mySerial.println(buff);
    delay(5);
    t1000 = millis();
  }
}
