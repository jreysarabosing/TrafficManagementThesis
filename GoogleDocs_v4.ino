/*  HTTPS on ESP8266 with follow redirects, chunked encoding support
 *  Version 3.0
 *  Author: Sujay Phadke
 *  Github: @electronicsguy
 *  Copyright (C) 2018 Sujay Phadke <electronicsguy123@gmail.com>
 *  All rights reserved.
 *
 *  Example Arduino program
 */

#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"
#include "DebugMacros.h"

// Fill ssid and password with your network credentials
const char* ssid = "Door to Narnia";
const char* password = "jonathan";

const char* host = "script.google.com";
// Replace with your own script id to make server side changes
const char *GScriptId = "AKfycbxMqhKd60KPOe21ZBRlwLmk_oKpbR2RIDlhV_6v_v2DzDfqH8c";

const int httpsPort = 443;

//GET to Google Spreadsheet
String url_base = String("/macros/s/") + GScriptId + "/exec";
String url = "";

HTTPSRedirect* client = nullptr;

void setup() {
  Serial.begin(115200);
  Serial.flush();
  
  Serial.println();
  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);
  // flush() is needed to print the above (connecting...) message reliably, 
  // in case the wireless connection doesn't go through
  Serial.flush();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  
  Serial.print("Connecting to ");
  Serial.println(host);

  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i=0; i<5; i++){
    int retval = client->connect(host, httpsPort);
    if (retval == 1) {
       flag = true;
       break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }

  if (!flag){
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    Serial.println("Exiting...");
    return;
  }
  
  // Note: setup() must finish within approx. 1s, or the the watchdog timer
  // will reset the chip. Hence don't put too many requests in setup()
  // ref: https://github.com/esp8266/Arduino/issues/34

  // delete HTTPSRedirect object
  delete client;
  client = nullptr;
}

void loop() {
  static int error_count = 0;
  static int connect_count = 0;
  const unsigned int MAX_CONNECT = 20;
  static bool flag = false;

  float speedData = 69;
  
  if (!flag){
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }

  if (client != nullptr){
    if (!client->connected()){
      client->connect(host, httpsPort);
    }
  }
  else{
    DPRINTLN("Error creating client object!");
    error_count = 5;
  }
  
  if (connect_count > MAX_CONNECT){
    connect_count = 0;
    flag = false;
    delete client;
    return;
  }

  if (client->GET(url_base, host, false)){
    ++connect_count;
  }
  else{
    ++error_count;
    DPRINT("Error-count while connecting: ");
    DPRINTLN(error_count);
  }

  Serial.println("GET append memory data to spreadsheet:");
  url = url_base + "?value=" + (String)speedData;
  if(client->GET(url, host, false)){
    Serial.print(" " + (String)speedData + " kmph\n");
  }
  else{
    ++error_count;
    DPRINT("Error-count while connecting: ");
    DPRINTLN(error_count);
  }
  
  if (error_count > 3){
    Serial.println("Halting processor..."); 
    delete client;
    client = nullptr;
    Serial.flush();
    ESP.deepSleep(0);
  }
  
  // In my testing on a ESP-01, a delay of less than 1500 resulted 
  // in a crash and reboot after about 50 loop runs.
  delay(1500);      
}
