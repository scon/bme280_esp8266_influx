#include <Arduino.h>
//WiFi libs
#include <ESP8266WiFi.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include <Wire.h>                                                       // required by BME280 library
#include <BME280_t.h>

int freq = 5000;


//ADC_MODE(ADC_VCC);

char incomingChar;
int conState = 0;

  float temp, hum, pres;
// Wifi Config
char MEASUREMENT_NAME[34] = "upload_test";
const char* AutoConnectAPName = "AutoConnectAP";
const char* AutoConnectAPPW = "password";

// Server Config
const char* InfluxDB_Server_IP = "130.149.67.141";
const int InfluxDB_Server_Port = 8086;
const char* InfluxDB_Database = "MESSCONTAINER";

BME280<> BMESensor;

// Initialisiert WiFiClient als "client"
WiFiClient client;


String Uploadstring, serverResponse;

String Daten1 = "";
String Daten2 = "";
String teststr = "";

//char incomingChar; // Dummy variable um Serverresponse zu lesen, aufs Display auszugeben und ggf. auf Serial auszugeben.


void WiFiStart(){
  /* Baut Verbindung mit dem letzten bekannten WiFi auf, wenn nicht vorhanden
  wird eigener AP erstellt -> IP: 192.168.4.1*/
  WiFiManager wifiManager;
  wifiManager.autoConnect(AutoConnectAPName, AutoConnectAPPW);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Setup");
  WiFiStart();
  //if you get here you have connected to the WiFi
  Serial.println("Connected to Wifi");
  Serial.print("IP:"); Serial.println(WiFi.localIP());
  Serial.print("Start Loop()");
  Wire.begin();                                                      // initialize I2C that connects to sensor
  BMESensor.begin();

}

void loop() {
  // Print the buffer on the serial line to see how it looks


  unsigned long entry = millis();


  Daten2 = "uploadtest,host=esp adc=" + String(analogRead(A0));

  Serial.println("Sending following dataset to InfluxDB: ");

  BMESensor.refresh();                                                  // read current sensor data

Serial.print("Temperature: ");
Serial.print(BMESensor.temperature);                                  // display temperature in Celsius
Serial.println("C");



  Daten1 = "uploadtest,host=esp t=" + String(BMESensor.temperature) + ",P=" + String(BMESensor.pressure) + ",H=" + String(BMESensor.humidity);
  Serial.println(Daten1);
  Serial.println(Daten2);

  //send to InfluxDB
  // Verbindungstest mit dem InfluxDB Server connect() liefert bool false / true als return
  conState = client.connect(InfluxDB_Server_IP, InfluxDB_Server_Port);


teststr = "ABCD";
teststr += "\r\n";
int Length = Daten1.length() + Daten2.length() +2;
int rntest = teststr.length();

Serial.println("Length: " + String(Length));
Serial.println("LengthRN: " + String(rntest));
Serial.println("VCC:  " + String(analogRead(A0)));



  //Sende HTTP Header und Buffer

    client.println("POST /write?db=ALPHASENSE&u=public&p=public HTTP/1.1");
    client.println("User-Agent: esp8266/0.1");
    client.println("Host: localhost:8086");
    client.println("Accept: */*");
    client.print("Content-Length: " + String(Length) + "\r\n");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println(); /*HTTP Header und Body müssen durch Leerzeile getrennt werden*/
    client.print(Daten1 + "\n"); // Übermittlung des eigentlichen Data-Strings
    client.print(Daten2 + "\n");
    client.flush(); //

    while(client.available()==0 ) {yield();}
          // Antwort des Servers wird gelesen, ausgegeben und anschließend die Verbindung geschlossen
    Serial.println("Antwort des Servers");
    serverResponse= "";


    while(client.available()) { // Empfange Antwort
      incomingChar=char(client.read());
      serverResponse += incomingChar;
    }
    Serial.println(serverResponse);

  Serial.println();
  client.stop();

  while (millis()- entry < freq) {
    yield();
  }
}
