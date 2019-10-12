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

byte send_buf[32];

// How many changes we need to send
int num_changed_keys;

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
    memset(send_buf, 0x00, 32);
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

void sendCodes() {
  // send the keycode to the remote end.
  
  for (int idx=0; idx < 16; idx++) {
    if (send_buf[2*idx] == 0x00) {
      continue;
    }
    Serial.print(send_buf[2*idx]);
    Serial.print(" ");
    if (send_buf[2*idx+1] == 1) {
        Serial.println("KeyDown ");
    } else {
        Serial.println("KeyUp ");
    }   
  }
  irq();
}

int findMatrixChanges() {
  unsigned int num_changed_keys = 0;
  memset(send_buf, 0x00, 32);

  // scan the matrixes for differences and appends the results to the send buffer
  for (int col_idx=0; col_idx < num_cols; col_idx++) {
    for (int row_idx=0; row_idx < num_rows; row_idx++) {
      if (prev_keys[row_idx][col_idx] != cur_keys[row_idx][col_idx]) {
        send_buf[2*num_changed_keys] = SCAN_CODES[row_idx][col_idx];
        send_buf[2*num_changed_keys+1] = (cur_keys[row_idx][col_idx] == LOW ? 1 : 0);
        num_changed_keys++;
        if (num_changed_keys == 16) goto exit;
      }
    }   
  }
exit:
  return num_changed_keys;
}

void irq() {
  Serial.println("irq");
  digitalWrite(IRQ_PIN, HIGH);
  digitalWrite(IRQ_PIN, LOW);    
}

void loop() {
  readMatrix();
  if (findMatrixChanges()) {
    sendCodes();
  }
  copyMatrix();
}

void requestEvent() {
  // send 32 bytes, always.
  for (unsigned int i = 0; i< 32; i++) { Serial.print(send_buf[i]); Serial.print(' ');}
  Serial.println(' ');
  Wire.write(send_buf, 32);
}

void receiveData(int byteCount){
  int counter = 0;
  while(Wire.available()) {
    receiveBuffer[counter] = Wire.read();
    counter++;
  }
}
