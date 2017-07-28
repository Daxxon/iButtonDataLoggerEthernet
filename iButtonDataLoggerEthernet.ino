#include <Ethernet.h>
#include <OneWire.h>
#include <SPI.h>
#include <SD.h>

EthernetServer server(80);
OneWire ibutton (2);
// maximum length of file name including path
#define FILE_NAME_LEN  20
// pin used for Ethernet chip SPI chip select
#define PIN_ETH_SPI   10

int doorPin = 6;
int ledPin = 5;
byte buffer[8];
byte result[1];
boolean locked;
File myFile;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0x02, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};
byte ip[] = {
  192,168,1,99
};
byte ddns[] = {
  192,168,1,254
};
byte gateway[] = {
  192,168,1,254
};
byte ibuttonid[6][8] = {
  {1,242,255,16,1,0,0,123},
  {1,243,74,177,1,0,0,247},
  {1,242,220,144,1,0,0,20},
  {1,247,116,228,1,0,0,84},
  {1,241,128,32,1,0,0,104},
  {1,245,199,197,1,0,0,196}
};

void setup() {
  Serial.begin(9600);
  pinMode(doorPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(PIN_ETH_SPI, OUTPUT);
  digitalWrite(PIN_ETH_SPI, HIGH);
  beginEthernet();
  beginServer();
  beginSD();
}

void loop() {
  EthernetClient client = server.available();
  delay(100);
  if (client) {
    while (client.connected()){
      if (client.available()){
        Serial.println("Serving web page");
        client.println("<!DOCTYPE html>");
        client.println("<html>");
        client.println("<head>");
        client.println("<title>Arduino Web Page</title>");
        client.println("</head>");
        client.println("<body>");
        client.print("<h1>Access History</h1>");
        client.println("<p>");
        myFile = SD.open("entryLog.txt");
        if (myFile){
          Serial.println("File fetched");
          while (myFile.available()){
            client.write(myFile.read());
          }
          myFile.close();
        }
        client.println("</p>");
        client.println("</body>");
        client.println("</html>");
        break;
      }
    }
    delay(10);
    client.stop();
    delay(10);
  }
  scanForButton();
  delay(5);
  cleanUp();
}

void beginSD(){
  if (!SD.begin(4)) {
    Serial.println("SD initialization failed");
    beginSD();  // SD card initialization failed
  } else {
    Serial.println("SD initialized");
  }
}

void scanForButton(){
  Serial.println("Searching for iButton");
  if (ibutton.search(buffer)){
    authenticate();
  }
}

void printBuffer(){
  for (int x = 0; x<8; x++){
    Serial.print(buffer[x],DEC);
    Serial.print(" ");
  }
  Serial.println();
}

void authenticate(){
  for (int i=0; i<6; i++){
    for (int x=0; x<8; x++){
      int compare1 = buffer[x];
      int compare2 = ibuttonid[i][x];
      if (compare1 == compare2){
        result[0] |= 1 << x;
      }
    }
    if (result[0] == 255){
      locked = false;
      releaseLock();
      logData();
    }
  }
}

void releaseLock(){
  if (locked != true){
    Serial.println("Door open for 3 seconds.");
    digitalWrite(doorPin,HIGH);
    digitalWrite(ledPin, HIGH);
    delay(3000);
    Serial.println("Door closed");
    digitalWrite(doorPin,LOW);
    digitalWrite(ledPin, LOW);
  }
}

void cleanUp(){
  lockLock();
  result[0] = 0;
  for (int x=0; x<8; x++){
    buffer[x] = 0;
  }
}

void lockLock(){
  if (locked != true){
    locked = true;
  }
}

void logData(){
  File logFile;
  logFile = SD.open("entryLog.txt", FILE_WRITE);
  if (logFile) {
    for (int x = 0; x<8; x++){
      logFile.print(buffer[x],DEC);
      logFile.print(" ");
    }
    logFile.println("</p><p>");
    logFile.close();
  }
}

void beginEthernet(){
  Ethernet.begin(mac, ip, ddns, gateway);
  if(!Ethernet.localIP()){
    beginEthernet();
  }
}

void beginServer(){
  server.begin();
  if(!server.available()){
    server.begin();
  }
}
 
