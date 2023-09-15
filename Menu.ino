void menu0(){
  for (byte i = 1; i < 15; i++) {
    colorButton(i, 0, 0, 0);
  }
  drawMenuButton1();
}

void menu10() {
  opMode = 10;
  drawMenuButton1();
  drawMenuButton2();
  textButton(2, "\n\n\n<Connect>");
  textButton(5, "\n\n\n <Reboot>");
  textButton(6, "\n\n\n <Config>");
  colorButton(7, 0, 0, 0);
  setBrightness(100);

}

void menu11(){
// for(byte i=0; i<15; i++){
//   colorButton(i, 255, 0, 255);
// }  
  opMode = 11;
  menuOctet = 0;
  textButton(0, "\n\n\n \x11 Back");
  if(dhcpEnabled){
    textButton(1, "\n\n  <DHCP>\n\n\n Enabled");
  }else{
    textButton(1, "\n\n  <DHCP>\n\n\n Disabled");
  }
  colorButton(2, 0, 0, 0);

  String buttonText = "\n\n<IP Addr>\n\n\n";
  buttonText.concat(IP_Static[0]);
  buttonText.concat(".");
  buttonText.concat(IP_Static[1]);
  buttonText.concat(".\n");
  buttonText.concat(IP_Static[2]);
  buttonText.concat(".");
  buttonText.concat(IP_Static[3]);
  textButton(5, buttonText);

  buttonText = "\n\n <Subnet>\n\n\n";
  buttonText.concat(IP_Subnet[0]);
  buttonText.concat(".");
  buttonText.concat(IP_Subnet[1]);
  buttonText.concat(".\n");
  buttonText.concat(IP_Subnet[2]);
  buttonText.concat(".");
  buttonText.concat(IP_Subnet[3]);
  textButton(6, buttonText);

  buttonText = "\n\n<Gateway>\n\n\n";
  buttonText.concat(IP_Gateway[0]);
  buttonText.concat(".");
  buttonText.concat(IP_Gateway[1]);
  buttonText.concat(".\n");
  buttonText.concat(IP_Gateway[2]);
  buttonText.concat(".");
  buttonText.concat(IP_Gateway[3]);
  textButton(7, buttonText);

}

void menu12(){
  opMode = 12;
  textButton(0, "\n\n\n \x11 Back");
  textButton(1, "\n   DHCP\n\n <Enable>");
  textButton(2, "\n   DHCP\n\n<Disable>");
  colorButton(5, 0, 0, 0);
  colorButton(6, 0, 0, 0);
  colorButton(7, 0, 0, 0);

}

void menu13(){
  opMode = 13;
  textButton(0, "\n\n\n \x11 Back");
  drawIPButton("IP Addr\n\n", IP_Static);

  textButton(2, "");
  textButton(5, "\n\n\n    -");
  textButton(7, "\n\n\n    +");
}

void menu14(){
  opMode = 14;
  textButton(0, "\n\n\n \x11 Back");
  drawIPButton("Subnet\n\n", IP_Subnet);

  textButton(2, "");
  textButton(5, "\n\n\n    -");
  textButton(7, "\n\n\n    +");
}

void menu15(){
  opMode = 15;
  textButton(0, "\n\n\n \x11 Back");
  drawIPButton("Gateway\n\n", IP_Gateway);

  textButton(2, "");
  textButton(5, "\n\n\n    -");
  textButton(7, "\n\n\n    +");
}

void drawMenuButton2(){
  String statusText = "server\n";
  if(netStatus == NET_STATUS_RECEIVING){
    statusText.concat(companionApiVersion);
  }else{
    statusText.concat("?.?.?");
  }
  statusText.concat("\n");
  statusText.concat("\n");
  statusText.concat(IP_Destination[0]);
  statusText.concat(".");
  statusText.concat(IP_Destination[1]);
  statusText.concat(".\n");
  statusText.concat(IP_Destination[2]);
  statusText.concat(".");
  statusText.concat(IP_Destination[3]);
  textButton(1, statusText);
}

void drawMenuButton1(){
  String statusText = "satellite\n";
  statusText.concat(mac[4]);
  statusText.concat(mac[5]);
  statusText.concat("\n\n");

  if(netStatus == NET_STATUS_BEGUN || netStatus == NET_STATUS_FAIL_CONNECT){
    for(byte i=0; i<netConnectAttempts%4; i++){
      statusText.concat(".");
    }
  
  }else if(netStatus == NET_STATUS_BEGUN || netStatus == NET_STATUS_CONNECTED || netStatus == NET_STATUS_RECEIVING){
    char ipStr[17];
    sprintf(ipStr, "%i.%i.\n%i.%i",
      Ethernet.localIP()[0],
      Ethernet.localIP()[1],
      Ethernet.localIP()[2],
      Ethernet.localIP()[3]);

    statusText.concat(ipStr);

  }
  
  textButton(0, statusText);
}

void drawIPButton(String label, IPAddress ip){
  String ipStr = label;

  menuOctet==0 ? ipStr.concat("> ") : ipStr.concat("  ");
  ipStr.concat(ip[0]);
  ipStr.concat("\n");

  menuOctet==1 ? ipStr.concat("> ") : ipStr.concat("  ");
  ipStr.concat(ip[1]);
  ipStr.concat("\n");

  menuOctet==2 ? ipStr.concat("> ") : ipStr.concat("  ");
  ipStr.concat(ip[2]);
  ipStr.concat("\n");

  menuOctet==3 ? ipStr.concat("> ") : ipStr.concat("  ");
  ipStr.concat(ip[3]);
  textButton(1, ipStr);

  ipStr = "\n\n\n   ";
  ipStr.concat(ip[menuOctet]);
  textButton(6, ipStr);
}

