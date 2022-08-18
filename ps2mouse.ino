
const int clk = 5;
const int data = 6;
void writePS2( byte Data);
unsigned long int readPS2();
volatile unsigned long int MouseRDPacket[3]; //3 11-bit packets:   buttons, x movement, y movement

void setup() {
  
  static unsigned long int rd[4];
  pinMode(clk, INPUT);
  pinMode(data, INPUT);

   writePS2(0xFF);    // reset mouse
   rd[0] = readPS2(); // acknowledge: should be 0xFA
   rd[1] = readPS2(); // self test:   should be 0xAA
   rd[2] = readPS2(); // mouse id:    Should be 0x00
   
   writePS2(0xF4);    // Enable mouse
   rd[3] = readPS2(); // acknowledge: should be 0xFA
   

   Serial.begin(9600);
    while (!Serial) {
    ; // wait for serial port
  }

  for(int i = 0; i < 4; i ++) {
    rd[i] = (rd[i] & 0b00111111110) >> 1; // take off pairity, start & stop bits
  }
 
 // print startup sequence
  Serial.println("FF reset command");
  Serial.println(rd[0]  , HEX);
  Serial.println(rd[1]  , HEX);
  Serial.println(rd[2]  , HEX);
  Serial.println("F4 Enable");
  Serial.println(rd[3]  , HEX);
}


void loop() {
  static unsigned long int x;

    
  while (digitalRead(data)){}
  x = readPS2();
  while (digitalRead(data)){}
  int xmvmt = readPS2();
  while (digitalRead(data)){}
  int ymvmt = readPS2();
  
  x = (x & 0b00111111110) >> 1; //take off pairity, start & stop bits

 if((x & 0b00001000) && (x & 0b00000111)&& !(x & 0b11000000) ) {
    Serial.print(x, BIN);
    Serial.println(" mouse pressed");
 }
  Serial.print("X:");
  xmvmt = (xmvmt & 0b00111111110) >> 1;  // take off pairity, start & stop bits
  if((x & 0b00010000) >> 4) {            // if negative, process 9-bit 2s complement to regular signed int type
    xmvmt = -((~xmvmt & 0b11111111) + 1);
  }
  Serial.print(xmvmt);
  Serial.print("  Y:");
   
   ymvmt = (ymvmt & 0b00111111110) >> 1; // take off pairity, start & stop bits
   if((x & 0b00100000) >> 5) {           // if negative, process 9-bit 2s complement to regular signed int type
    ymvmt = -((~ymvmt &0b11111111) + 1);
  }
  Serial.println(ymvmt);
  Serial.println(" ");
}

// write a byte to the mouse
void writePS2( byte Data) {

  // bring the clock low to stop mouse from communicating
  pinMode(clk, OUTPUT); 
  digitalWrite(clk, LOW);
  delayMicroseconds(200); // wait for mouse

  // bring data low to tell mouse that the host wants to communicate
  pinMode(data, OUTPUT);
  digitalWrite(data, LOW);
  delayMicroseconds(50);
  
  // release control of clock by putting it back to a input
  pinMode(clk, INPUT_PULLUP);

  // now clk is high because of pullup
  
  while(digitalRead(clk)){} // wait for clock to go low again
  
  // when the mouse brings clock low again, it is ready to communicate

  // shift out byte from LSB to MSB
  int pairck = 0;                               // track # of 1s in byte for pairity
  for(int i = 0; i < 8; i++) {
    digitalWrite(data, (Data & (1 <<i))  >> i); // shift out a bit when clock is low
    pairck += ((Data & (1 <<i))  >> i);         // count if 1 for pairity check later
    while(digitalRead(clk) == LOW){}            // wait for clock to go high
    while(digitalRead(clk) == HIGH){}           // wait for clock to go low
  }

  // add pairity bit if needed so that the number of 1s in the byte shifted out plus the pairity bit is always odd
  if(pairck %2 == 0) {
    digitalWrite(data, HIGH);
  }
  else {
    digitalWrite(data, LOW);
  }
  while(digitalRead(clk) == LOW){} // wait for clock to go high
  while(digitalRead(clk) == HIGH){} // wait for clock to go low
  

  pinMode(data, INPUT_PULLUP);       // release control of data
  while(digitalRead(data) == HIGH){} // wait for data to go low
  while(digitalRead(clk)  == HIGH){} // wait for clk to go low
  
  while(digitalRead(data) == LOW){} // wait for data to go high again
  while(digitalRead(clk) == LOW){}  // wait for clk to go high again
  
}





// read 11 byte packet from the mouse
unsigned long int readPS2 () {
  unsigned long int b;

  // shift in 11 bits from MSB to LSB
  for(int i = 0; i < 11; i++) {
    while(digitalRead(clk) == HIGH){} // wait for the clock to go LOW
    b += (digitalRead(data) << i);    // shift in a bit 
    while(digitalRead(clk) == LOW){}  // wait here while the clock is still low
   }
   return b;
}
