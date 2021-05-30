#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <Wire.h> //I2C
#include <RF_Data_Processor.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>          /* Process JSON documents, serialization, packs and other*/

RF_Data_Processor data_processor;
WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

int receivedCount = 0;

void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
	const uint8_t* src = (const uint8_t*) mem;
	Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
	for(uint32_t i = 0; i < len; i++) {
		if(i % cols == 0) {
			Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
		}
		Serial.printf("%02X ", *src);
		src++;
	}
	Serial.printf("\n");
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
	switch(type) {
		case WStype_DISCONNECTED:
			Serial.printf("[WSc] Disconnected!\n");
			break;
		case WStype_CONNECTED:
			Serial.printf("[WSc] Connected to url: %s\n", payload);
			webSocket.sendTXT("{\"event\": \"connecting\"}"); // send message to server when Connected
			break;
      
		case WStype_TEXT:
      {
        // Serial.printf("Response: %s\n", payload);
        receivedCount++;
        char buff[static_cast<int>(length)+1];
        for(int x = 0; x < sizeof(buff); x++) {
          buff[x] = (char)(payload[x]);
        }
        // Serial.print("Size of buffer: ");
        // Serial.println(sizeof(buff));
        // Serial.print("Buffer: ");
        // Serial.println(buff);
        sendJsonViaI2C(buff, sizeof(buff));
        // webSocket.sendTXT("message here"); // send message to server
      }
			break;
		case WStype_BIN:
			Serial.printf("[WSc] get binary length: %u\n", length);
			hexdump(payload, length);
			// webSocket.sendBIN(payload, length); // send data to server
			break;

		case WStype_ERROR:			
		case WStype_FRAGMENT_TEXT_START:
		case WStype_FRAGMENT_BIN_START:
		case WStype_FRAGMENT:
		case WStype_FRAGMENT_FIN:
    default:
      // Serial.printf("Event occured, but unknown type or no handler; Payload: %s", payload);
      Serial.println("Event occured, but unknown type or no handler;");
			break;
	}

}

void setup() {
	Serial.begin(115200);
  Serial.printf("Begin!\n"); 

  Wire.begin(21, 22); /* join i2c bus with SDA=D1 and SCL=D2 of NodeMCU */

	for(uint8_t t = 4; t > 0; t--) {
		Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
		Serial.flush(); // Waits for the transmission of outgoing serial data to complete.
		delay(1000);
	}

  // WiFiMulti.addAP("Nataliya", "08061961");  
	// WiFiMulti.addAP("U3H", "12345678");
  WiFiMulti.addAP("Redmi", "randompass"); 

	while(WiFiMulti.run() != WL_CONNECTED) {
		delay(500);
    Serial.print(".");
	}
  Serial.println();  

	// server address, port and URL
	webSocket.begin("3785fcc32d2a.ngrok.io", 80, "/socket1", "ws");
  // webSocket.beginSocketIO("92.112.198.38", 5000, "/");
	
	webSocket.onEvent(webSocketEvent); // event handler
	webSocket.setReconnectInterval(5000); //If failed try again after this time passes
  Serial.println("Ready!");  
}

const int sendInterval = 5000;
unsigned int prevTime = 0;
unsigned int prevTime2 = 0;
int lastPackId = 0;

void loop() {
	webSocket.loop();

  // Wire.requestFrom(8, 32); /* request & read data of size 13 from slave */
  // String jsonPack = "";
  // while(Wire.available()){
  //   jsonPack += (char) Wire.read();
  // }
  // jsonPack.trim();

  // if(jsonPack != "") {
  //   data_processor.pushJsonPack(jsonPack);
  //   // Serial.println((String) "'" + jsonPack + "'");
  // }
  // if(data_processor.available()) {
  //   String json = data_processor.getLastJson();
  //   sendMessage(json);
  //   // Serial.println((String) "Json: " + json);
  // }


  // if((millis()-prevTime) >= sendInterval) {
  //   Serial.print("Received: ");
  //   Serial.println(receivedCount);
  //   receivedCount = 0;
  //   sendMessage();
  //   prevTime = millis();
  // }

  if((millis()-prevTime2) >= 100) {
    sendMessage("\"Manual\"");
    prevTime2 = millis();
  }
}

void sendMessage(String message) {

  // String event = "{\"event\": \"test\", \"data\": \"" + message + "\"}";
  String event = "{\"event\": \"test\", \"data\": " + message + "}";
  // String event = "{\"event\": \"test\", \"data\": \"testVal\"}";

  // Serial.print("Sending: ");
  // Serial.println(event);

  webSocket.sendTXT(event); // send message to server when Connected
}

void sendJsonViaI2C(char* message, int messageSize) {
  //CONSTANT VALUES FOR PACKS---------------------------------
  int code = 1;
  int packNo = 1;
  const size_t ARRAY_CAPACITY_FOR_PACK = JSON_ARRAY_SIZE(3);// compute the required size
  //----------------------------------------------------------

  //PREPARE---------------------------------------------------
  bool isInitPack = true;
  bool sending = true;
  int lastIndex = 0;
  char payload[messageSize];
  for(int g = 0; g < messageSize; g++)payload[g] = (message[g] != '\"')? message[g]: '$'; //Copy message to another array and replace all forbidden characters
  String payloadString = String(payload);
  //----------------------------------------------------------

  while(sending) {
    // 1. Generate empty json
    DynamicJsonDocument emptyDocPack(512);
    //generate meta
    StaticJsonDocument< ARRAY_CAPACITY_FOR_PACK > initMetaArrayDoc;// allocate the memory for the document
    JsonArray initMetaArray = initMetaArrayDoc.to<JsonArray>();// create an empty array
    initMetaArray.add(0); //Any number between 0 and 9, needed just to calculate size
    initMetaArray.add(packNo);
    initMetaArray.add(lastPackId);
    emptyDocPack["m"] = initMetaArrayDoc;
    emptyDocPack["d"] = "";

    // 2. Measure its actual size in bytes and calculate required data
    int emptyPackSize = measureJson(emptyDocPack);
    int availablePayloadSize = 32 - emptyPackSize - 2;//Minus two because we will transform it into an array later

    // 3. Generate data snippet(from payload)
    String payloadSnippet = payloadString.substring(lastIndex, lastIndex + availablePayloadSize);
    lastIndex = lastIndex + availablePayloadSize;

    // 4. Create actually object to save data input
    DynamicJsonDocument docPack(512);
    StaticJsonDocument< ARRAY_CAPACITY_FOR_PACK > metaArrayDoc;//allocate the memory for the document
    JsonArray metaArray = metaArrayDoc.to<JsonArray>();// create an empty array
    //Set code( 1-one message is a full pack, 2-beginning of the message, 3-somewhere in the middle, 4-end of the message(last pack))
    if(availablePayloadSize >= payloadString.length()) {
      metaArray.add(1);
    } else if(isInitPack) {
      metaArray.add(2);
      isInitPack = false;
    }  else if(lastIndex >= payloadString.length()) {
      metaArray.add(4);
    } else {
      metaArray.add(3);
    }
    metaArray.add(packNo);
    metaArray.add(lastPackId);
    docPack["m"] = metaArrayDoc;
    docPack["d"] = payloadSnippet; //Insert payload snippet

    // 5. Serialize pack
    String finishedPack;
    serializeJson(docPack, finishedPack);
    char finishedPackArr[finishedPack.length()+1];
    finishedPack.toCharArray(finishedPackArr, sizeof(finishedPackArr));

    // 6. Send finished pack via I2C
    Wire.beginTransmission(8); /* begin with device address 8 */
    // Wire.write(finishedPackArr, sizeof(finishedPackArr));
    // Serial.print("Sending I2C: ");
    // Serial.println(finishedPackArr);
    Wire.write(finishedPackArr);
    Wire.endTransmission();    /* stop transmitting */
    // send(finishedPackArr, sizeof(finishedPackArr));
    
    // 7. If all payload was sent - stop sending
    if(lastIndex >= payloadString.length()) sending = false;
    packNo++;
  }
  
  //Maximum pack id is 99
  getLastPackId();
}

int getLastPackId(void) {
  int res = lastPackId;
  lastPackId++;
  if(lastPackId > 99) lastPackId = 1;
  return res;
}
