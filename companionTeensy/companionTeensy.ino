#include <USBHost_t36.h>
#include "HID.h"
#include "toojpeg.h"
#include <QNEthernet.h>
using namespace qindesign::network;

USBHost myusb;
USBHIDParser hid1(myusb);
USBHIDParser hid2(myusb);
HIDDumpController hdc1(myusb);

int remainingBytes;
int packetMax = 8192-8;
byte iteration = 0;
boolean state = false;
byte buffer[8192];
long lastHeartbeat = 0;
String product;
boolean initialized = false;
byte jpeg2[9000];
boolean buttonStates[32];
auto image = new unsigned char[72 * 72 * 3];

EthernetClient companionServer;
uint8_t macAddress[6];
IPAddress IP_Static{192, 168, 1, 130};
IPAddress IP_Subnet{255, 255, 255, 0};
IPAddress IP_Gateway{192, 168, 0, 1};
IPAddress IP_Destination{192, 168, 1, 120};
boolean netConnected = false;
boolean linkOn = false;
char uniqueID[3];



void setup(){
  myusb.begin();
  Serial.begin(9600);

  netSetup();

}


void loop(){
  netLoop();

  if(millis()-lastHeartbeat>1000){
    lastHeartbeat = millis();
    
    if(!netConnected && Ethernet.linkState()){
      delay(100);
      netConnected = companionServer.connect(IP_Destination, 16622);
      delay(100);
    }
    
    if(!initialized){
      product = String((char *)hid1.product());
      Serial.println(product);
     
      if(product == "Stream Deck MK.2"){
        initialized = true;
        for(byte i=0; i<15; i++){
          colorButton(i, 0, 0, 50);
        }
        
      }else if(product == "Stream Deck XL"){
        initialized = true;
        for(byte i=0; i<32; i++){
          colorButton(i, 255, 0, 255);
        }
      }
    }
  }
  
}


void buttonPressed(const void *ptr, uint32_t len){
  const uint8_t *p = (const uint8_t *)ptr;
  for(byte i=0; i<32; i++){
  
    boolean newState = *(p + 4 + i);
    char key[2];
    String(i).toCharArray(key, 3);
    
    if(newState!=buttonStates[i]){
      companionServer.writeFully("KEY-PRESS DEVICEID=");
      companionServer.writeFully(uniqueID[0]);
      companionServer.writeFully(uniqueID[1]);
      companionServer.writeFully(uniqueID[2]);
      companionServer.writeFully(" KEY=");
      companionServer.writeFully(key, 2);
      companionServer.writeFully(" PRESSED=");
      companionServer.writeFully(newState? "true":"false");
      companionServer.writeFully("\n");
      companionServer.flush();
    
    }
    buttonStates[i] = newState;
  }

}


int jpegIndex = 0;
int col = 0;
void myOutput(unsigned char oneByte){
  jpeg2[jpegIndex] = oneByte;
  jpegIndex++;
  
}


void colorButton(byte index, byte r, byte g, byte b){
  for(int i=0; i<72*72*3; i+=3){
    image[i] = r;
    image[i+1] = g;
    image[i+2] = b;
  }
  drawButton(index);
}


void drawButton(byte buttonIndex){

  jpegIndex = 0;
  
  boolean ok = TooJpeg::writeJpeg(myOutput, image, 72, 72);
  
  if(!ok){
    return;
  }
  
  remainingBytes = jpegIndex;
  iteration = 0;
  
  while (remainingBytes > 0){
    int sliceLength = min(remainingBytes, packetMax);
    byte finalizer = sliceLength == remainingBytes ? (byte)1 : (byte)0;
    int bitmaskedLength = (byte)(sliceLength & 0xFF);
    int shiftedLength = (byte)(sliceLength >> 8);
    int bitmaskedIteration = (byte)(iteration & 0xFF);
    byte shiftedIteration = (byte)(iteration >> 8);
  
    buffer[0] = 0x02;
    buffer[1] = 0x07;
    buffer[2] = buttonIndex;
    buffer[3] = finalizer;
    buffer[4] = bitmaskedLength;
    buffer[5] = shiftedLength;
    buffer[6] = bitmaskedIteration;
    buffer[7] = shiftedIteration;

    for(int i=0; i<packetMax; i++){
      unsigned int x = i+(iteration*packetMax);
      if(x>sizeof(jpeg2)){
        buffer[i+8] = 0;
      }else{
        buffer[i+8] = jpeg2[i+(iteration*packetMax)];
      }
    }

    hid1.sendPacket(buffer, 8192);
  
    remainingBytes -= sliceLength;
    iteration++;  
  }
}


uint8_t brightnessControlPacket[32]; 
void setBrightness(byte brightness){
  brightnessControlPacket[0] = 0x03;
  brightnessControlPacket[1] = 0x08;
  brightnessControlPacket[2] = brightness;
  hid1.sendControlPacket(0x21, 0x09, 0x0303, 0x00, 32, brightnessControlPacket);
}
