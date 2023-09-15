#include <USBHost_t36.h>
#include "HID.h"
#include "toojpeg.h"
#include <NativeEthernet.h>
#include <Adafruit_GFX.h>

#define NET_STATUS_NONE 0
#define NET_STATUS_BEGUN 1
#define NET_STATUS_CONNECTED 2
#define NET_STATUS_RECEIVING 3
#define NET_STATUS_FAIL_CONNECT 4
#define NET_STATUS_LINK_OFF 5

#define USB_STATUS_NONE 0
#define USB_STATUS_CONNECTED 2

#define CPU_RESTART (*(uint32_t *)0xE000ED0C = 0x5FA0004);

USBHost myusb;
USBHIDParser hid1(myusb);
// USBHIDParser hid2(myusb);
HIDDumpController hdc1(myusb);

int remainingBytes;

byte iteration = 0;
boolean state = false;

long lastHeartbeat = 0;
String product;
byte jpeg2[11000];
boolean buttonStates[32];
// unsigned char image[72 * 72 * 3];
unsigned char image[96 * 96 * 3];

EthernetClient companionServer;
IPAddress IP_Static(192, 168, 1, 130);
IPAddress IP_Subnet{ 255, 255, 255, 0 };
IPAddress IP_Gateway(192, 168, 0, 1);
IPAddress IP_Destination(192, 168, 1, 120);

uint8_t mac[6];
boolean linkOn = false;
byte pressCount = 0;
byte opMode = 0;
byte drawnMode = 255;
byte netStatus = NET_STATUS_NONE;
byte usbStatus = USB_STATUS_NONE;
String companionApiVersion = "0.0.0";
byte serverIP[4];
String msg = "";
byte menuOctet = 0;
boolean dhcpEnabled = false;

long lastNoButtonTime;
long lastPushTime;

int netConnectAttempts = 0;
int randInt;


void setup() {
  myusb.begin();
  Serial.begin(74880);
  teensyMAC();
  netStatus = NET_STATUS_NONE;
  
  Ethernet.begin(mac, IP_Static);

  if (Ethernet.linkStatus() == LinkON) {
    netStatus = NET_STATUS_BEGUN;
  }else{
    netStatus = NET_STATUS_LINK_OFF;
  }

  lastNoButtonTime = millis();
}


void loop() {

  // myusb.Task();
  
  if(netStatus==NET_STATUS_CONNECTED || netStatus==NET_STATUS_RECEIVING){
    while(companionServer.available()){
      byte c = companionServer.read();
      // Serial.print((char)c);
      if(c==0x0a){
        processServerMessage();
        msg = "";
      }else if(c!=0x0d){
        msg+=(char)c;
      }
    }
  }


  if (millis() - lastHeartbeat > 500) {
    lastHeartbeat = millis();
    

    if (usbStatus == USB_STATUS_NONE) {
      product = String((char *)hid1.product());

      Serial.print("product: ");
      Serial.println(product);
      
      if (product == "Stream Deck MK.2") {
        usbStatus = USB_STATUS_CONNECTED;

      } else if (product == "Stream Deck XL") {
        usbStatus = USB_STATUS_CONNECTED;
      }

    }else if(usbStatus == USB_STATUS_CONNECTED){

      // Serial.println(opMode);

      if(opMode==0){
        menu0();
      }else if(opMode==2){
        textButton(1, "\n\nDevice ID\nError");
      }else if(opMode==10){
        menu10();
      }else if(opMode==11){
        menu11();
      }else if(opMode==12){
        menu12();
      }else if(opMode==13){
        menu13();
      }else if(opMode==14){
        menu14();
      }else if(opMode==15){
        menu15();
      }
      
      if(opMode==0 || opMode==10){
        if(netStatus == NET_STATUS_BEGUN || netStatus == NET_STATUS_FAIL_CONNECT){
          Serial.println("attempting to connect");
          companionServer.setConnectionTimeout(1000);
          boolean netConnected = companionServer.connect(IP_Destination, 16622);

          netConnectAttempts++;
          if(netConnected){
            netStatus = NET_STATUS_CONNECTED;
          }

        }

      }else if(opMode==1){
        companionServer.println("PING");
      }
    }

    if (millis() - lastPushTime > 60000 && opMode < 10){
      opMode = 0;
    }

    

  }

  if (millis() - lastPushTime > 4000 && pressCount == 1 && opMode < 10) { // long press menu
    for(byte i=0; i<32; i++){
      if(buttonStates[i]){
        char key[2];
        String(i).toCharArray(key, 3);
        companionServer.print("KEY-PRESS DEVICEID=");
        companionServer.print(mac[4]);
        companionServer.print(mac[5]);
        companionServer.print(randInt);
        companionServer.print(" KEY=");
        companionServer.printf(key);
        companionServer.print(" PRESSED=");
        companionServer.print("false");
        companionServer.print("\n");
        buttonStates[i] = false;
      }
    }
    pressCount = 0;
    menu10();
  }

  int delayTime = (millis()-lastPushTime<1000)*200+50;
  if(opMode == 13){
    if(buttonStates[5]){
      IP_Static[menuOctet]--;
      drawIPButton("IP Addr\n\n", IP_Static);
      delay(delayTime);
    }else if(buttonStates[7]){
      IP_Static[menuOctet]++;
      drawIPButton("IP Addr\n\n", IP_Static);
      delay(delayTime);
    }
  }else if(opMode == 14){
    if(buttonStates[5]){
      IP_Subnet[menuOctet]--;
      drawIPButton("Subnet\n\n", IP_Subnet);
      delay(delayTime);
    }else if(buttonStates[7]){
      IP_Subnet[menuOctet]++;
      drawIPButton("Subnet\n\n", IP_Subnet);
      delay(delayTime);
    }
  }else if(opMode == 15){
    if(buttonStates[5]){
      IP_Gateway[menuOctet]--;
      drawIPButton("Gateway\n\n", IP_Gateway);
      delay(delayTime);
    }else if(buttonStates[7]){
      IP_Gateway[menuOctet]++;
      drawIPButton("Gateway\n\n", IP_Gateway);
      delay(delayTime);
    }
  }
  
}


void keyPressedEvent(const void *ptr, uint32_t len) {

  pressCount = 0;

  const uint8_t *p = (const uint8_t *)ptr;
  for (byte i = 0; i < 32; i++) {

    boolean newState = *(p + 4 + i);
    if (newState) {
      pressCount++;
    }

    char key[2];
    String(i).toCharArray(key, 3);

    if (newState != buttonStates[i]) {
      
      if (opMode == 1) {
        companionServer.print("KEY-PRESS DEVICEID=");
        companionServer.print(mac[4]);
        companionServer.print(mac[5]);
        companionServer.print(randInt);
        companionServer.print(" KEY=");
        companionServer.printf(key);
        companionServer.print(" PRESSED=");
        companionServer.print(newState ? "true" : "false");
        companionServer.print("\n");

      }else if (opMode == 10 && !newState) {
      
        if (i == 2) {
          netStatus = NET_STATUS_BEGUN;
          companionServer.close();
          lastHeartbeat-=3000;

        }else if (i==5) {
          companionServer.close();
          opMode = 0;
          delay(500);
          CPU_RESTART;

        }else if (i == 6) {
          menu11();
        }
        
      }else if (opMode == 11 && !newState) { // Network Menu
        if(i==0){
          menu10();
        }else if(i==1){
          menu12();
        }else if(i==5){
          menu13();
        }else if(i==6){
          menu14();
        }else if(i==7){
          menu15();
        }

      }else if (opMode == 12 && !newState) { // DHCP Menu
        if(i==0){
          menu11();
        }else if(i==1){
          dhcpEnabled = true;
          menu11();
        }else if(i==2){
          dhcpEnabled = false;
          menu11();
        }

      }else if (opMode == 13 && !newState) { // IP Address Menu
        if(i==0){
          menu11();
        }else if(i==1 || i==6){
          menuOctet++;
          if(menuOctet>3){
            menuOctet = 0;
          }
          menu13();
        }

      }else if (opMode == 14 && !newState) { // Subnet Menu
        if(i==0){
          menu11();
        }else if(i==1 || i==6){
          menuOctet++;
          if(menuOctet>3){
            menuOctet = 0;
          }
          menu14();
        }

      }else if (opMode == 15 && !newState) { // Gateway Menu
        if(i==0){
          menu11();
        }else if(i==1 || i==6){
          menuOctet++;
          if(menuOctet>3){
            menuOctet = 0;
          }
          menu15();
        }
      }

    }
    buttonStates[i] = newState;
  }

  if (pressCount == 0) {
    lastNoButtonTime = millis();
  } else {
    lastPushTime = millis();
  }

  if(opMode==0 && pressCount>0){
    menu10();
  }

}


void processServerMessage(){
  if(usbStatus == USB_STATUS_NONE){
    return;
  }

  // Serial.print("-");
  // Serial.println(msg);

  if(msg.startsWith("BEGIN")){

    netStatus = NET_STATUS_RECEIVING;
    
    companionServer.print("ADD-DEVICE DEVICEID=");
    companionServer.print(mac[4]);
    companionServer.print(mac[5]);

    randomSeed(millis());
    int randInt = random(0, 10);
    companionServer.print(randInt);


    if(product == "Stream Deck MK.2"){
      companionServer.print(" PRODUCT_NAME=\"Companion Satellite (Teensy)\" KEYS_TOTAL=15 KEYS_PER_ROW=5\n");

    }else if(product == "Stream Deck XL"){
      companionServer.print(" PRODUCT_NAME=\"Companion Satellite (Teensy)\" KEYS_TOTAL=32 KEYS_PER_ROW=8\n");
    }

    short start = msg.indexOf("ApiVersion=")+11;
    companionApiVersion = msg.substring(start);
    menu0();
    
  }else if(msg.startsWith("BRIGHTNESS")){
    netStatus = NET_STATUS_RECEIVING;

    int val = msg.substring(msg.indexOf("VALUE=")+6).toInt();
    setBrightness((byte)(val*2));

  }else if(msg.startsWith("KEY-STATE") && msg.indexOf("BITMAP")>0){
    netStatus = NET_STATUS_RECEIVING;
    
    companionServer.print("");
    // Serial.println(msg);

    if(opMode==1){

      int key = msg.substring(msg.indexOf("KEY=")+4, msg.indexOf(" ", msg.indexOf("KEY="))).toInt();
      
      int start = msg.indexOf("BITMAP=")+7;
      int end = msg.indexOf(" ", start);
    
      String str = msg.substring(start, end);
      char char_array[str.length()]; 
      str.toCharArray(char_array, str.length());

      decode_base64((unsigned char *)char_array, str.length(), image);

      scaleBuffer();
      
      rotateBuffer180();
      
      drawButtonFromBuffer(key);

    }
    
  }else if(msg.startsWith("ADD-DEVICE OK")){
    opMode = 1;

  }else if(msg.startsWith("ADD-DEVICE ERROR")){
    companionServer.close();
    opMode = 2;
  
  }else{
    // Serial.println("unknown message");
    // Serial.println(msg);
  }
  //Serial.println(msg);
  
}









static void teensyMAC() {
  uint32_t m1 = HW_OCOTP_MAC1;
  uint32_t m2 = HW_OCOTP_MAC0;
  mac[0] = m1 >> 8;
  mac[1] = m1 >> 0;
  mac[2] = m2 >> 24;
  mac[3] = m2 >> 16;
  mac[4] = m2 >> 8;
  mac[5] = m2 >> 0;
}
