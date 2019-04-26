//Libraries being used
#include <SPI.h>
#include <Chrono.h>
#include <RH_RF95.h>

//Definitions
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2
#define RF95_FREQ 915.0 //Must match RX's freq!
#define LED 13

//Instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

//Instance of stopwatch timer
Chrono StopWatch;

//Global variables for Ultrasonic
const int pwPin1 = 3;
float sensor, mm1, mm, pw1, distance1;
bool flag = false;

void setup(){
  StopWatch.stop();
  
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  pinMode(pwPin1, INPUT);
  delay(100);

  while (!Serial);
  Serial.begin(9600);
  delay(100);
  
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  while (!rf95.init()){
    digitalWrite(LED, LOW);
    delay(100);
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);
    delay(100);
    digitalWrite(LED, HIGH);
    delay(500);
  }

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  while (!rf95.setFrequency(RF95_FREQ)){
    digitalWrite(LED, LOW);
    delay(100);
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);
    delay(100);
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);
    delay(100);
    digitalWrite(LED, HIGH);
    delay(500);
  }
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}

int16_t packetnum = 0;  // packet counter, we increment per xmission

void read_sensors(){
  /*
  Scale factor is (Vcc/512) per inch. A 5V supply yields ~9.8mV/in
  Arduino analog pin goes from 0 to 1024, so the value has to be divided by 2 to get the actual inches
  */
  sensor = pulseIn(pwPin1, HIGH);
  mm1 = sensor;
  pw1=25.4/147;
  mm=mm1*pw1;
  distance1 = mm/25.4;
}

void loop(){
  char radiopacket[20];
  float spd,t;
  read_sensors();

  if(distance1>7.87 && distance1<86.61){
    if(StopWatch.isRunning()==false){
      StopWatch.restart();
      flag=1;
    }
  }
  else if(distance1<7.87 && flag==1 || distance1>86.61 && flag==1){
      radiopacket[0]="A";
      t=StopWatch.elapsed();
      Serial.println(t);
      spd=12/(t/1000);
      Serial.println(spd);
      dtostrf(spd,4,6,radiopacket);
      Serial.println(radiopacket);
      rf95.send((uint8_t*)radiopacket, 20);
      rf95.waitPacketSent();
      Serial.println("Sent");
      digitalWrite(LED, HIGH);
      digitalWrite(LED, LOW);
      t=0;
      StopWatch.stop();
      flag=0;
    }
    delay(50);

    
  /*if(distance1 <200 && distance1 > 6){
    StopWatch.restart();
    t = StopWatch.elapsed();
    spd = 12/t; //inches per sec
    itoa(spd, radiopacket, 10);
    radiopacket[19] = 0;
    Serial.println(radiopacket);
    rf95.send((uint8_t*)radiopacket, 20);
    rf95.waitPacketSent();
    Serial.println("Sent");
    digitalWrite(LED, HIGH);
    digitalWrite(LED, LOW);
    t=0;
    StopWatch.restart();
  }else{
    t = StopWatch.elapsed();
    spd = 0;
    itoa(spd, radiopacket, 10);
    radiopacket[19] = 0;
    Serial.println(radiopacket);
    rf95.send((uint8_t*)radiopacket, 20);
    rf95.waitPacketSent();
    Serial.println("Sent");
    digitalWrite(LED, HIGH);
    digitalWrite(LED, LOW);
    t=0;
    StopWatch.restart();
  }*/
}
