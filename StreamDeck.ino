
int jpegIndex = 0;
void jpegByte(unsigned char oneByte) {
  jpeg2[jpegIndex] = oneByte;
  jpegIndex++;
}

void drawButtonFromBuffer(byte buttonIndex) {

  jpegIndex = 0;
  //boolean ok = TooJpeg::writeJpeg(jpegByte, image, 72, 72, true, 100, true);
  boolean ok = TooJpeg::writeJpeg(jpegByte, image, 96, 96, true, 100, true);

  if (!ok) {
    return;
  }

  remainingBytes = jpegIndex;
  iteration = 0;
  int packetMax = 1024 - 8;

  // https://den.dev/blog/reverse-engineering-stream-deck/
  while (remainingBytes > 0) {

    int sliceLength = min(remainingBytes, packetMax);

    byte finalizer = sliceLength == remainingBytes ? (byte)1 : (byte)0;
    byte bitmaskedLength = (byte)(sliceLength & 0xFF);
    byte shiftedLength = (byte)(sliceLength >> 8);
    byte bitmaskedIteration = (byte)(iteration & 0xFF);
    byte shiftedIteration = (byte)(iteration >> 8);

    byte buffer[packetMax + 8];

    buffer[0] = 0x02;
    buffer[1] = 0x07;
    buffer[2] = buttonIndex;
    buffer[3] = finalizer;
    buffer[4] = bitmaskedLength;
    buffer[5] = shiftedLength;
    buffer[6] = bitmaskedIteration;
    buffer[7] = shiftedIteration;

    for (int i = 0; i < packetMax; i++) {
      unsigned int x = i + (iteration * packetMax);
      if (x > sizeof(jpeg2)) {
        buffer[i + 8] = 0;
      } else {
        buffer[i + 8] = jpeg2[i + (iteration * packetMax)];
      }
    }

    hid1.sendPacket(buffer, packetMax + 8);
    delayMicroseconds(500);
    
    remainingBytes -= sliceLength;
    iteration++;
  }
}


uint8_t brightnessControlPacket[32];
void setBrightness(byte brightness) {
  brightnessControlPacket[0] = 0x03;
  brightnessControlPacket[1] = 0x08;
  brightnessControlPacket[2] = brightness;
  hid1.sendControlPacket(0x21, 0x09, 0x0303, 0x00, 32, brightnessControlPacket);
}

void rotateBuffer180(){
  // long length = 72*72*3-3;
  long length = 96*96*3-3;
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


void scaleBuffer(){

  unsigned char scaled[96 * 96 * 3];
  int k = 0;
  int i = 0;
  for(int y=0; y<96; y++){
    for(int x=0; x<72; x++){
      scaled[k] = image[i];
      scaled[k+1] = image[i+1];
      scaled[k+2] = image[i+2];
      k+=3;
      if(x%3==0){
        scaled[k] = image[i];
        scaled[k+1] = image[i+1];
        scaled[k+2] = image[i+2];
        k+=3;
      }
      i+=3;
    }
    if(y%3==0){
      i-=72*3;
    }
  }

  for(int i=0; i<96 * 96 * 3; i++){
    image[i] = scaled[i];
  }
}