// Keymine firmware for mega2560
// Stefano Borini - 2019
// from baldengineer.com
// CC BY-SA 4.0
#include "usb_hid_keycodes.h"

byte ROW_PINS[] = {22, 24, 26, 28, 30, 32, 34, 36};
byte COL_PINS[] = {38, 40, 42, 44, 46, 48, 50, 52};

const int num_rows = sizeof(ROW_PINS)/sizeof(ROW_PINS[0]);
const int num_cols = sizeof(COL_PINS)/sizeof(COL_PINS[0]); 

byte SCAN_CODES[][num_cols] = {
  {KEY_Q, KEY_W},
  {KEY_A, KEY_S}
};

byte prevKeys[num_rows][num_cols];
byte curKeys[num_rows][num_cols];

void setup() {
    Serial.begin(115200);
 
    for(int x=0; x<num_rows; x++) {
        pinMode(ROW_PINS[x], INPUT);
    }
    for (int x=0; x<num_cols; x++) {
        pinMode(COL_PINS[x], INPUT_PULLUP);
    }
    for (int col_idx=0; col_idx < num_cols; col_idx++) {
      for (int row_idx=0; row_idx < num_rows; row_idx++) {
        prevKeys[row_idx][col_idx] = HIGH;
        curKeys[row_idx][col_idx] = HIGH; 
      }
    }
    Serial.println(matrixChanged());     
}

void copyMatrix() {
    for (int col_idx=0; col_idx < num_cols; col_idx++) {
      for (int row_idx=0; row_idx < num_rows; row_idx++) {
            prevKeys[row_idx][col_idx] = curKeys[row_idx][col_idx];
      }
    }
  
}
void readMatrix() {
    // iterate the columns
    for (int col_idx=0; col_idx < num_cols; col_idx++) {
        // col: set to output to low
        byte curCol = COL_PINS[col_idx];
        pinMode(curCol, OUTPUT);
        digitalWrite(curCol, LOW);
 
        // row: interate through the ROW_PINS
        for (int row_idx=0; row_idx < num_rows; row_idx++) {
            byte rowCol = ROW_PINS[row_idx];
            pinMode(rowCol, INPUT_PULLUP);
            curKeys[row_idx][col_idx] = digitalRead(rowCol);
            pinMode(rowCol, INPUT);
        }
        // disable the column
        pinMode(curCol, INPUT);
    }
}
 
void sendCode() {
  for (int col_idx=0; col_idx < num_cols; col_idx++) {
    for (int row_idx=0; row_idx < num_rows; row_idx++) {
      if (prevKeys[row_idx][col_idx] == curKeys[row_idx][col_idx]) {
        continue;
      }
      if (curKeys[row_idx][col_idx] == LOW) {
        Serial.print("KeyDown ");
      } else {
        Serial.print("KeyUp ");
      }
      Serial.println(SCAN_CODES[row_idx][col_idx]);
    }   
  }
}

bool matrixChanged() {
  for (int col_idx=0; col_idx < num_cols; col_idx++) {
    for (int row_idx=0; row_idx < num_rows; row_idx++) {
      if (prevKeys[row_idx][col_idx] != curKeys[row_idx][col_idx]) {
        return true;
      }
    }   
  }
  return false;  
}

void loop() {
  readMatrix();
  if (matrixChanged()) {
    sendCode();
    copyMatrix();
  }
}
