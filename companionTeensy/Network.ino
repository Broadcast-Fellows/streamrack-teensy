#include <Arduino.h>
#include <QNEthernet.h>
//#include "base64.hpp"
using namespace qindesign::network;

int netMode = 0;


void netSetup(){
  Ethernet.macAddress(macAddress);

  Ethernet.onLinkState([](bool state) {
    Serial.print("Ethernet Link state changed to ");
    Serial.println(state ? "ON":"OFF");
    randomSeed(millis()); 
    linkOn = state;
    
  });

  Ethernet.onAddressChanged([]() {
    Serial.println("Ethernet Address changed");
  });

  Serial.println("Beginning Ethernet");

  if(Ethernet.begin()){
    if(netMode == 2){
      if (!Ethernet.waitForLocalIP(5000)) {
        Serial.println("Unable to connect with DHCP");
        Serial.print("Changing to Static IP ");
        Serial.println(IP_Static);
        Ethernet.begin(IP_Static, IP_Subnet, IP_Gateway);
        
      } else{
        Serial.println("Connected via DHCP");
        Serial.println(IP_Static);
  
      }
    }else{
      Serial.print("Setting Static IP ");
      Serial.println(IP_Static);
      Ethernet.begin(IP_Static, IP_Subnet, IP_Gateway);
    }
    
  }
  companionServer.setNoDelay(true);
}


String msg;
void netLoop(){
  while(companionServer.available()){
    byte c = companionServer.read();
    if(c==0x0a){
      processServerMessage();
      msg = "";
    }else if(c!=0x0d){
      msg+=(char)c;
    }
  }
}


void processServerMessage(){
  if(!initialized){
    return;
  }
  if(msg.startsWith("BEGIN")){
    uniqueID[0] = (char)random(48, 58);
    uniqueID[1] = (char)random(48, 58);
    uniqueID[2] = (char)random(48, 58);
    
    companionServer.writeFully("ADD-DEVICE DEVICEID=");
    companionServer.writeFully(uniqueID[0]);
    companionServer.writeFully(uniqueID[1]);
    companionServer.writeFully(uniqueID[2]);

    if(product == "Stream Deck MK.2"){
      companionServer.writeFully(" PRODUCT_NAME=\"Companion Satellite (Teensy)\" KEYS_TOTAL=15 KEYS_PER_ROW=5\n");
    }else if(product == "Stream Deck XL"){
      companionServer.writeFully(" PRODUCT_NAME=\"Companion Satellite (Teensy)\" KEYS_TOTAL=32 KEYS_PER_ROW=8\n");
    }
    
  }else if(msg.startsWith("BRIGHTNESS")){
    int val = msg.substring(msg.indexOf("VALUE=")+6).toInt();
    setBrightness((byte)(val*2));

  }else if(msg.startsWith("KEY-STATE") && msg.indexOf("BITMAP")>0){
    int key = msg.substring(msg.indexOf("KEY=")+4, msg.indexOf(" ", msg.indexOf("KEY="))).toInt();
    
    int start = msg.indexOf("BITMAP=")+7;
    int end = msg.indexOf(" ", start);
   
    String str = msg.substring(start, end);
    char char_array[str.length()]; 
    str.toCharArray(char_array, str.length()+1);
    
    decode_base64(char_array, str.length(), image);
    
    rotate180();
    
    drawButton(key);
    
    
  }else{
    //Serial.println(msg);
  }
  //Serial.println(msg);
  
}


void rotate180(){
  int length = 72*72*3;
  for(int i=0; i<length/2; i+=3){
    byte tempR = image[i];
    byte tempG = image[i+1];
    byte tempB = image[i+2];
    
    image[i] = image[length-i];
    image[i+1] = image[length-i+1];
    image[i+2] = image[length-i+2];
    
    image[length-i+2] = tempB;
    image[length-i+1] = tempG;
    image[length-i] = tempR;
    
  }
}
