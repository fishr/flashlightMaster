#include <SPI.h>
#define NOP __asm__ __volatile__ ("nop\n\t")
//Master comm on Uno
int CE=8;
int IRQ=9;
int CSN=10;

void setup(){
  delay(1000);
  pinMode(IRQ, INPUT);
  pinMode(CSN, OUTPUT);
  digitalWrite(CSN, HIGH);
  pinMode(CE, OUTPUT);
  digitalWrite(CE,LOW);
  Serial.begin(115200);
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.begin();
  nrfMastOn();
  delay(5);
  writeReg(0x00,0b00001111); //powerup
  flushBuffers();
  delay(5);
  digitalWrite(CE,HIGH);
  delay(5);
}

void loop(){
  if(digitalRead(IRQ)==LOW){
    //Serial.println(readReg(0x1D));
    //Serial.println(readReg(0x00));
    //delay(10);
  
  if(0b01000000&readReg(0x07)){
    byte indata = readrf();
    Serial.println(indata);
  }else{
    Serial.println("oops!");
  }
  }
}

void nrfMastOn(){
  writeReg(0x00,0b00001101); //make rx mode
  writeReg(0x01,0b00111111); //autoACK for all pipes
  writeReg(0x02,0b00111111); //enable all pipes
  writeReg(0x03,0b00000001);  //3 byte addresses
  writeReg(0x04,0b00111111);  // retransmit delay, and up to 15 retries
  //writeReg(0x11, 0b00111111);  //payload size max
  //writeReg(0x12, 0b00111111);  //payload size max
  //writeReg(0x13, 0b00111111);  //payload size max
  //writeReg(0x14, 0b00111111);  //payload size max
  //writeReg(0x15, 0b00111111);  //payload size max
  writeReg(0x16, 0b00000001);  //payload size max
  writeAddr(0x0A, 0x01); //first receive addr
  writeAddr(0x0B, 0x02); //first receive addr
  writeReg(0x0C,0x03); //first receive addr
  writeReg(0x0D,0x04); //first receive addr
  writeReg(0x0E,0x05); //first receive addr
  writeReg(0x0F,0x06); //first receive addr
  //writeReg(0x50, 0x73);  //enable specials
  //writeReg(0x1D,0b00000100); //enable dynamic payload and ack payload
  //writeReg(0x1C,0b00111111); //enable dynamic payload for all pipes
  delay(5);
  
}

byte writeAddr(byte addr, byte index){
  digitalWrite(CSN, LOW);
  NOP;
  byte incoming = SPI.transfer(writeRegVal(addr)); //first receive addr
  NOP;
  SPI.transfer(index);
  NOP;
  SPI.transfer(0xE7);
  NOP;
  SPI.transfer(0xE7);
  NOP;
  digitalWrite(CSN, HIGH);
  NOP;
  return incoming;
}

byte writeReg(byte addr, byte data){
  digitalWrite(CSN, LOW);
  NOP;
  byte incoming = SPI.transfer(0b00100000|(0b00011111&addr));
  NOP;
  SPI.transfer(data);
  NOP;
  digitalWrite(CSN, HIGH);
  NOP;
  return incoming;
}

byte writeRegVal(byte addr){
  return (0b00100000|(0b00011111&addr));
}

byte readReg(byte addr){
  digitalWrite(CSN, LOW);
  NOP;
  SPI.transfer(0b00011111&addr);
  NOP;
  byte incoming = SPI.transfer(0xFF);
  NOP;
  digitalWrite(CSN, HIGH);
  NOP;
  return incoming;
}

byte transmit(byte data){
  digitalWrite(CSN, LOW);
  NOP;
  SPI.transfer(0b10100000);
  NOP;
  SPI.transfer(data);
  NOP;
  digitalWrite(CSN, HIGH);
}

byte readrf(){
  digitalWrite(CSN, LOW);
  NOP;
  SPI.transfer(0b01100001);
  NOP;
  byte data = SPI.transfer(0xFF);
  NOP;
  digitalWrite(CSN, HIGH);
  NOP;
  writeReg(0x07, 0b01110000);
  return data;
}

void flushBuffers(){
  digitalWrite(CSN, LOW);
  NOP;
  SPI.transfer(0b11100001);
  NOP;
  digitalWrite(CSN, HIGH);
  NOP;
  digitalWrite(CSN, LOW);
  NOP;
  SPI.transfer(0b11100010);
  NOP;
  digitalWrite(CSN, HIGH);
  NOP;
}
