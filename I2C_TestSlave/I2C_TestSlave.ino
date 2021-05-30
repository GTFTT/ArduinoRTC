#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h> //I2C
#include <RF_Data_Processor.h>

RF_Data_Processor data_processor;


const char *ssid = "U3H";  //ENTER YOUR WIFI SETTINGS
const char *password = "12345678"; //WIFI PASSWORD
WiFiClient client;

//Web/Server address to read/write from 
// String host = "http://14b0cc88fff7.ngrok.io";
String host = "http://rtc-api-server.herokuapp.com";
String arduinoRoute = "/arduino";

//Create connection------------------------------
HTTPClient http;    //Declare object of class HTTPClient
String Link = host + arduinoRoute;

//--------------------------------------------

void setup() {
  Serial.begin(115200); /* begin serial for debug */
  Wire.begin(4, 5); /* join i2c bus with SDA=D1 and SCL=D2 of NodeMCU */
  // WiFi.mode(WIFI_OFF);
  // delay(500);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // client.setNoDelay(1);

  //Create connection to use it without reconnecting each time
  http.begin(client, Link);     //Specify request destination
  http.addHeader("Content-Type", "application/json");  //Specify content-type header
}

void loop() {
  // Wire.beginTransmission(8); /* begin with device address 8 */
  // Wire.write(messageCallback, sizeof(messageCallback));  /* sends hello string */
  // Wire.endTransmission();    /* stop transmitting */
  
  Wire.requestFrom(8, 32); /* request & read data of size 13 from slave */
  String jsonPack = "";
  while(Wire.available()){
    jsonPack += (char) Wire.read();
  }
  jsonPack.trim();

  if(jsonPack != "") {
    data_processor.pushJsonPack(jsonPack);
    // Serial.println((String) "'" + jsonPack + "'");
  }
  if(data_processor.available()) {
    String json = data_processor.getLastJson();
    sendJsonToTheServer(json);
    Serial.println((String) "Json: " + json);
  }
  delay(5);
}

void sendJsonToTheServer(String jsonToSend) {
  http.POST(jsonToSend);
  // http.getString();    //Get the response payload
  // http.end();  //Close connection
}