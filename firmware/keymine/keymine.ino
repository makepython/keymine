#include <Wire.h>

// Keymine firmware for mega2560
// Stefano Borini - 2019
// from baldengineer.com
// CC BY-SA 4.0
#include "usb_hid_keycodes.h"

byte ROW_PINS[] = {22, 24, 26, 28, 30, 32, 34, 36};
byte COL_PINS[] = {38, 40, 42, 44, 46, 48, 50, 52};
byte IRQ_PIN = 23;
int receiveBuffer[9];

const int num_rows = sizeof(ROW_PINS)/sizeof(ROW_PINS[0]);
const int num_cols = sizeof(COL_PINS)/sizeof(COL_PINS[0]); 

byte SCAN_CODES[][num_cols] = {
  {KEY_Q, KEY_W, KEY_E},
  {KEY_A, KEY_S, KEY_D},
  {KEY_Z, KEY_X, KEY_C}
};

byte prev_keys[num_rows][num_cols];
byte cur_keys[num_rows][num_cols];

// Send buffer and number of changed keys to send to the remote end.
// While making volatile an array will make volatile only the pointer to the array probably,
// it's better safe than sorry. I am unsure how this works at the lower level.
// Note that some time may pass between the interrupt is issued and the master come and read
// the content, so what we want to do is to only push to this buffer when num_changed_keys
// is zero. If it's not zero, it means that there are pending data to be sent, and we will
// simply throw away the changeset. At least for now. In other words, we must ensure atomic
// operations on num_changed_keys and use it as a flag.
volatile byte send_buf[32];
volatile unsigned int num_changed_keys;
volatile unsigned int watchdog;

void setup() {
    Serial.begin(115200);
 
    for(int x=0; x<num_rows; x++) {
        pinMode(ROW_PINS[x], INPUT);
    }
    for (int x=0; x<num_cols; x++) {
        pinMode(COL_PINS[x], INPUT_PULLUP);
    }
    pinMode(IRQ_PIN, OUTPUT);
    digitalWrite(IRQ_PIN, LOW);
    for (int col_idx=0; col_idx < num_cols; col_idx++) {
      for (int row_idx=0; row_idx < num_rows; row_idx++) {
        prev_keys[row_idx][col_idx] = HIGH;
        cur_keys[row_idx][col_idx] = HIGH;
      }
    }
    watchdog = 0;
    memset((byte *)send_buf, 0x00, 32*sizeof(byte));
    Wire.begin(0x42);
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveData);

}

void copyMatrix() {
    // copy the current matrix to the storage for change detection
    for (int col_idx=0; col_idx < num_cols; col_idx++) {
      for (int row_idx=0; row_idx < num_rows; row_idx++) {
            prev_keys[row_idx][col_idx] = cur_keys[row_idx][col_idx];
      }
    }
  
}

void readMatrix() {
    // read the current matrix by scanning the keys one by one
    for (int col_idx=0; col_idx < num_cols; col_idx++) {
        // col: set to output to low
        byte cur_col = COL_PINS[col_idx];
        pinMode(cur_col, OUTPUT);
        digitalWrite(cur_col, LOW);
 
        // row: interate through the ROW_PINS
        for (int row_idx=0; row_idx < num_rows; row_idx++) {
            byte row_col = ROW_PINS[row_idx];
            pinMode(row_col, INPUT_PULLUP);
            cur_keys[row_idx][col_idx] = digitalRead(row_col);
            pinMode(row_col, INPUT);
        }
        // disable the column
        pinMode(cur_col, INPUT);
    }
}

void signalMaster() {
  // Report the master that we have something to read, but only if we have space for it.
  noInterrupts();
  if (num_changed_keys) {
    irq();
  }
  interrupts();
}

int packChanges() {
  unsigned int nchanged = 0;
  char buf[32];
  memset(buf, 0x00, 32*sizeof(byte));
  
  // scan the matrixes for ditfferences and appends the results to the send buffer
  for (int col_idx=0; col_idx < num_cols; col_idx++) {
    for (int row_idx=0; row_idx < num_rows; row_idx++) {
      if (prev_keys[row_idx][col_idx] != cur_keys[row_idx][col_idx]) {
        buf[2*nchanged] = SCAN_CODES[row_idx][col_idx];
        buf[2*nchanged+1] = (cur_keys[row_idx][col_idx] == LOW ? 1 : 0);
        nchanged++;
        if (nchanged == 16) goto exit;
      }
    }   
  }
  
exit:
//  // if we don't have space in the buffer because the master hasn't retrieved it yet
//  // we have no other choice but to throw the events away.
//  if (num_changed_keys) {
//    Serial.println("Throwing away buffer");
//    watchdog++;
//    if ((watchdog % 3) == 0) {
//      Serial.println("Resetting");
//      noInterrupts();
//      // copy the local buffer into the send buffer as an atomic operation
//      memcpy((byte *)send_buf, buf, 32*sizeof(byte));
//      num_changed_keys = nchanged;
//      interrupts();  
//    }
//    return 0;
//  }
//
  if (nchanged) {
//    noInterrupts();
    // copy the local buffer into the send buffer as an atomic operation
    memcpy((byte *)send_buf, buf, 32*sizeof(byte));
    num_changed_keys = nchanged;
//    interrupts();
//    
  }
  return nchanged;
}

void irq() {
  digitalWrite(IRQ_PIN, HIGH);
  digitalWrite(IRQ_PIN, LOW);    
}

void requestEvent() {
  // this is an ISR so interrupts are already disabled.
  byte buf[32];
  // Wire.write wants a const uint_t, so we copy the volatile data in the local stack
  memcpy(buf, (byte *)send_buf, 32*sizeof(byte));
  
  // send 32 bytes, always. The master is waiting for them.  
  Wire.write(buf, 32);
  // mark the buffer as sent.
  memset((byte *)send_buf, 0x00, 32*sizeof(byte));
  num_changed_keys = 0;
}

void receiveData(int byteCount){
  int counter = 0;
  while(Wire.available()) {
    receiveBuffer[counter] = Wire.read();
    counter++;
  }
}

void loop() {
  readMatrix();
  if (packChanges()) {
    signalMaster();
    copyMatrix();
  }
}
