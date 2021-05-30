#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

/* Set these to your desired credentials. */
const char *ssid = "U3H";  //ENTER YOUR WIFI SETTINGS
const char *password = "12345678"; //WIFI PASSWORD
WiFiClient client;

//Web/Server address to read/write from 
const char *host = "http://1463d699a658.ngrok.io";   //https://circuits4you.com website or IP address of server
// String Link = "http://rtc-api-server.herokuapp.com/huinya"; //GET Data
String Link = "http://e36415b595b9.ngrok.io/arduino"; //GET Data from local server
 
// Use this to connect to localhost
// const IPAddress server(92, 112, 198, 38); //Local computer's IP 92.112.198.38
// const int httpPort = 5000;

//=======================================================================
//                    Power on setup
//=======================================================================

void setup() {
  delay(1000);
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot
  
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //Connect client
  // client.connect(server, httpPort);
  // if (!client.connect(host, httpPort)) {
  //   Serial.println("connection failed");
  //   return;
  // }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
}

int i = 0 ;
//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() {
  HTTPClient http;    //Declare object of class HTTPClient

  String ADCData, station, getData;
  int adcvalue=analogRead(A0);  //Read Analog value of LDR
  ADCData = String(adcvalue);   //String to interger conversion
  station = "B";
  
  http.begin(client, Link);     //Specify request destination
  
  http.addHeader("Content-Type", "text/plain");  //Specify content-type header

  // int httpCode = http.GET();            //Send the request
  int httpCode = http.POST((String) "Message from ESP8266: " + i);
  String payload = http.getString();    //Get the response payload

  Serial.println((String) "Code: " + httpCode);   //Print HTTP return code
  Serial.println(payload);    //Print request response payload

  http.end();  //Close connection
  i++;
  delay(100);  //GET Data at every 5 seconds
}
//=======================================================================