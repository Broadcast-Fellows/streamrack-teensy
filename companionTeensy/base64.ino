
unsigned int decode_base64(unsigned char input[], unsigned int input_length, unsigned char output[]) {
  unsigned int output_length = decode_base64_length(input, input_length);
  byte scale = 0;
  byte row = 0;
  // While there are still full sets of 24 bits...
  for(unsigned int i = 2; i < output_length; i += 3) {
    output[0] = base64_to_binary(input[0]) << 2 | base64_to_binary(input[1]) >> 4;
    output[1] = base64_to_binary(input[1]) << 4 | base64_to_binary(input[2]) >> 2;
    output[2] = base64_to_binary(input[2]) << 6 | base64_to_binary(input[3]);
//
//    if(scale%3==0){
//      output[3] = base64_to_binary(input[0]) << 2 | base64_to_binary(input[1]) >> 4;
//      output[4] = base64_to_binary(input[1]) << 4 | base64_to_binary(input[2]) >> 2;
//      output[5] = base64_to_binary(input[2]) << 6 | base64_to_binary(input[3]);
//      output+=6;
//    }else{
//      output += 3;
//    }

    output += 3;
    input += 4;

//    scale++;
//    if(scale>74){
//      scale = 0;
//      row++;
//      if(row%3==0){
//        i-=74*3;
//        output-=74*3
//        input-=74*4;
//      }
//    }

  }
  
  switch(output_length % 3) {
    case 1:
      output[0] = base64_to_binary(input[0]) << 2 | base64_to_binary(input[1]) >> 4;
      break;
    case 2:
      output[0] = base64_to_binary(input[0]) << 2 | base64_to_binary(input[1]) >> 4;
      output[1] = base64_to_binary(input[1]) << 4 | base64_to_binary(input[2]) >> 2;
      break;
  }
  
  return output_length;
}

unsigned int decode_base64_length(unsigned char input[], unsigned int input_length) {
  unsigned char *start = input;
  
  while(base64_to_binary(input[0]) < 64 && (unsigned int) (input - start) < input_length) {
    ++input;
  }
  
  input_length = (unsigned int) (input - start);
  return input_length/4*3 + (input_length % 4 ? input_length % 4 - 1 : 0);
}

unsigned char base64_to_binary(unsigned char c) {
  // Capital letters - 'A' is ascii 65 and base64 0
  if('A' <= c && c <= 'Z') return c - 'A';
  
  // Lowercase letters - 'a' is ascii 97 and base64 26
  if('a' <= c && c <= 'z') return c - 71;
  
  // Digits - '0' is ascii 48 and base64 52
  if('0' <= c && c <= '9') return c + 4;
  
  #ifdef BASE64_URL
  // '-' is ascii 45 and base64 62
  if(c == '-') return 62;
  #else
  // '+' is ascii 43 and base64 62
  if(c == '+') return 62;
  #endif
  
  #ifdef BASE64_URL
  // '_' is ascii 95 and base64 62
  if(c == '_') return 63;
  #else
  // '/' is ascii 47 and base64 63
  if(c == '/') return 63;
  #endif
  
  return 255;
}
