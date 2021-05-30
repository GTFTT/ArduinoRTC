
#include <Wire.h>
#include <RF_Data_Processor.h>

RF24 radio(7, 8); // CE, CSN
RF24* RF_Data_Processor::_radio = &radio;
RF_Data_Processor data_processor;


char message[] = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
char message2[] ="012345678901234567890123456789";

void setup() {
  Wire.begin(8);                /* join i2c bus with address 8 */
  Wire.onReceive(receiveEvent); /* register receive event */
  Wire.onRequest(requestEvent); /* register request event */
  Serial.begin(115200);           /* start serial for debug */

  data_processor.initializeRadio("00009", "00008");
  // data_processor.setupRadioForReading();
  data_processor.setupRadioForWriting();
}

// unsigned int time;
// int prev = 0;
// int cou = 0;
String lastPack;

void loop() {
  // time = millis();
  
  // char buff[JSON_Pack.length()];
  // Wire.write(buff, sizeof(buff)); //Send it 
  // if(JSON_Pack != "") Serial.println((String ) "Received: " + JSON_Pack);
  // String data = data_processor.receive();
  // if(data != "") {
  //   Serial.println((String) "Data: " + data_processor.pushJsonPack(data));
  //   // data_processor.pushJsonPack(data);
  //   Serial.println((String) "Pack: '" + data + "'");
  // }
  // if(data_processor.available()) Serial.println((String) "Json: " + data_processor.getLastJson());
  // if(data_processor.available()) {
  //   data_processor.getLastJson();
  //   cou++;
  // }

  // if(time/1000 > prev) {
  //   prev=time/1000;
  //   Serial.println((String) "Count: " + cou);
  //   cou = 0;
  // }

  // String JSON_Pack = data_processor.receiveJson();
  // if(JSON_Pack!= "") Serial.println(JSON_Pack);
}

// function that executes whenever data is received from master
void receiveEvent(int howMany) {
  String pack = "";
  while (0 <Wire.available()) {
    char c = Wire.read();
    pack += c;
  }
  while(pack.length() < 32) {
    pack += ' ';
  }
  
  char buff[pack.length()+1];
  pack.toCharArray(buff, sizeof(buff));

  data_processor.send(buff, sizeof(buff));
  Serial.println((String)"Sent: " + buff);
}

// function that executes whenever data is requested from master
void requestEvent() {
  // data_processor.setupRadioForReading();
  String JSON_Pack = data_processor.receiveJson(); //Receive 32 byte pack
  if(JSON_Pack != "") {
    String strToSend = JSON_Pack;
    char buff[33]; //The last symbol always has to be terminattor and we dont send it
    
    //Remove empty characters as we can send only known amount of bytes, and fill with spaces till the end to form 32bytes of data
    for(int i = strToSend.length(); i < 32; i++) {
      strToSend += ' ';
    }

    // Serial.println((String) "Send: " + "'" + strToSend + "'; Len: " + strToSend.length());

    strToSend.toCharArray(buff, sizeof(buff));

    // Serial.println((String) "Sen2: " + "'" + buff + "'; Len: " + sizeof(buff));
    
    Wire.write(buff, 32); //Send 32 bytes, maximum of the message size
  } else {
    char emptyMessage[] = "                                ";
    Wire.write(emptyMessage, 32); //Send 32 bytesof spaces, empty message
  }
  // delay(2);
}