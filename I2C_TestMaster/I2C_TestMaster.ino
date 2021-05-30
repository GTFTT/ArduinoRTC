
#include <Wire.h>
// #include <Test.h>
#include <RF_Data_Processor.h>

// Test test;
RF_Data_Processor test;

char message[] = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
char message2[] ="012345678901234567890123456789112";

void setup() {
 Wire.begin(8);                /* join i2c bus with address 8 */
 Wire.onReceive(receiveEvent); /* register receive event */
 Wire.onRequest(requestEvent); /* register request event */
 Serial.begin(9600);           /* start serial for debug */
}

void loop() {
 delay(100);
//  test.superTest();
}

// function that executes whenever data is received from master
void receiveEvent(int howMany) {
 while (0 <Wire.available()) {
    char c = Wire.read();      /* receive byte as a character */
    Serial.print(c);           /* print the character */
  }
 Serial.println();             /* to newline */
}

// function that executes whenever data is requested from master
void requestEvent() {
  //Print action type here and payload(json to send)
 Wire.write(message2);  /*send string on request */
 delay(100);
}