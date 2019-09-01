// Keymine firmware for mega2560
// Stefano Borini - 2019
// from baldengineer.com
// CC BY-SA 4.0
#include "usb_hid_keycodes.h"

byte rowPins[] = {22, 24};
byte colPins[] = {38, 40};

const int rowCount = sizeof(rowPins)/sizeof(rowPins[0]);
const int colCount = sizeof(colPins)/sizeof(colPins[0]); 
byte keys[colCount][rowCount];

byte scanCodes[][rowCount] = {
  {KEY_Q, KEY_W},
  {KEY_A, KEY_S}
};


void setup() {
    Serial.begin(115200);
 
    for(int x=0; x<rowCount; x++) {
        Serial.print(rowPins[x]); Serial.println(" as input");
        pinMode(rowPins[x], INPUT);
    }
 
    for (int x=0; x<colCount; x++) {
        Serial.print(colPins[x]); Serial.println(" as input-pullup");
        pinMode(colPins[x], INPUT_PULLUP);
    }
         
}
 
void readMatrix() {
    // iterate the columns
    for (int colIndex=0; colIndex < colCount; colIndex++) {
        // col: set to output to low
        byte curCol = colPins[colIndex];
        pinMode(curCol, OUTPUT);
        digitalWrite(curCol, LOW);
 
        // row: interate through the rowPins
        for (int rowIndex=0; rowIndex < rowCount; rowIndex++) {
            byte rowCol = rowPins[rowIndex];
            pinMode(rowCol, INPUT_PULLUP);
            keys[colIndex][rowIndex] = digitalRead(rowCol);
            pinMode(rowCol, INPUT);
        }
        // disable the column
        pinMode(curCol, INPUT);
    }
}
 
void printMatrix() {
    for (int colIndex=0; colIndex < colCount; colIndex++) {
      for (int rowIndex=0; rowIndex < rowCount; rowIndex++) {
            if (!keys[colIndex][rowIndex]) {
              Serial.println(scanCodes[colIndex][rowIndex]);
            }
        }   
    }
}
 
void loop() {
    readMatrix();
    printMatrix();
}
