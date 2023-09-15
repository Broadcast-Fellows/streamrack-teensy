void colorButton(byte index, byte r, byte g, byte b) {
  // for (int i = 0; i < 72 * 72 * 3; i += 3) {
  for (int i = 0; i < 96 * 96 * 3; i += 3) {
    image[i] = r;
    image[i + 1] = g;
    image[i + 2] = b;
  }
  drawButtonFromBuffer(index);
}

void textButton(byte index, String text) {

  GFXcanvas1 testCanvas(56, 56);
  testCanvas.println(text);

  int i = 96*8*3;
  // int i = 72*8*3;
  for(int y = 0; y<56; y++){
    // i+=8*3;
    i+=20*3;
    for(int x=0; x<56; x++){
      uint8_t pixel = testCanvas.getPixel(x, y)*255;
      image[i] = pixel;
      image[i + 1] = pixel;
      image[i + 2] = pixel;
      i+=3;
    }
    i+=20*3;
    // i+=8*3;
  }

  rotateBuffer180();
  drawButtonFromBuffer(index);
}