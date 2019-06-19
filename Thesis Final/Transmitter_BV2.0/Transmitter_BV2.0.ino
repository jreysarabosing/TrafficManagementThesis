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
#define TRUCK_LENGTH 0.0139999974   //in km
#define SUV_LENGTH 0.0048514        //in km
#define SEDAN_LENGTH 0.0047244      //in km
#define MAX_DISTANCE 105.354
#define MIN_DISTANCE 7.874

//Instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

//Instance of stopwatch timer
Chrono StopWatch;

//Global variables for Ultrasonic
const int pwPin1 = 3;
float sensor, mm1, mm, pw1, distance1, vehicle_length;
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

float read_sensors(){
  /*
  Scale factor is (Vcc/512) per inch. A 5V supply yields ~9.8mV/in
  Arduino analog pin goes from 0 to 1024, so the value has to be divided by 2 to get the actual inches
  */
  sensor = pulseIn(pwPin1, HIGH);
  mm1 = sensor;
  pw1=25.4/147;
  mm=mm1*pw1;
  return mm/25.4;
}

void loop(){
  String packet,spd_raw;
  float spd, t;
  distance1=read_sensors();
  delay(10);
  Serial.println(distance1,5);
  Serial.println(flag);
  char radiopacket[20];

  if(distance1<=MAX_DISTANCE && distance1>=MIN_DISTANCE){
    Serial.println("Something Detected");
    if(StopWatch.isRunning()==false){
      Serial.println("Timer was Running");
      StopWatch.restart();
      Serial.println("Timer Reset");
      flag=true;
      if(distance1>=MIN_DISTANCE && distance1<95.054){
        vehicle_length=TRUCK_LENGTH;  //551.181 inches in km
      }else if(distance1>=95.054 && distance1<99.354){
        vehicle_length=SUV_LENGTH;     //191 inches in km
      }else{
        vehicle_length=SEDAN_LENGTH;     //186 inches in km
      }
    }
  }
  else if((distance1>MAX_DISTANCE || distance1<MIN_DISTANCE) && flag==true){
      Serial.println("Stop Detecting");
      t=StopWatch.elapsed();
      Serial.println(t);
      spd=(vehicle_length)/(t/3600000.0);
      if(spd>60){
        //False Data
        t=0;
        flag=false;
        return;
      }
      spd_raw=spd;
      Serial.println(spd,5);
      packet='B'+spd_raw;
      Serial.println(packet);
      packet.toCharArray(radiopacket,20);
      Serial.println(radiopacket);
      rf95.send((uint8_t*)radiopacket, 20);
      rf95.waitPacketSent();
      Serial.println("Sent");
      digitalWrite(LED, HIGH);
      digitalWrite(LED, LOW);
      t=0;
      StopWatch.stop();
      flag=false;
  }
  delay(50);
}
