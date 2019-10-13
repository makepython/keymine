#include <Wire.h>

byte IRQ_PIN = 23;

volatile unsigned int counter;

void setup() {
    Serial.begin(115200);
 
    pinMode(IRQ_PIN, OUTPUT);
    digitalWrite(IRQ_PIN, LOW);
    Wire.begin(0x42);
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveData);

}

void signalMaster() {
  noInterrupts();
  irq();
  interrupts();
}

void irq() {
  digitalWrite(IRQ_PIN, HIGH);
  digitalWrite(IRQ_PIN, LOW);    
}

void requestEvent() {
  // this is an ISR so interrupts are already disabled.
  byte buf[32];
  counter++;
  // Wire.write wants a const uint_t, so we copy the volatile data in the local stack
  memset(buf, counter, 32*sizeof(byte));
  // send 32 bytes, always. The master is waiting for them.  
  Wire.write(buf, 32);
}

void receiveData(int byteCount){
  while(Wire.available()) {
    Wire.read();
  }
}

void loop() {
  signalMaster();
  delay(1);
}
