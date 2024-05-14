#include <SPI.h>
#include <mcp_can.h>
#include <WiFi.h>

struct CanData {
  long unsigned int canId;
  unsigned char data[8];
  byte length;
};

MCP_CAN CAN(10);
char *ssid = ""; 
char *password = ""; 

const char *host = ""; // IP-address for pythonapp
const int port = 8080; // port to Python

void setup() {
  Serial.begin(115200);
  // connect to wifi
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to WiFi");
  

  if (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK) { // begin
    Serial.println("CAN-BUS Shield init ok!");
  } else {
    Serial.println("Fel vid initialisering av CAN-BUS Shield");
  }
}

void loop() {
  CanData canMessage;

  // Överför CAN-meddelanden
  sendCanMessage();

  // Tar emot och bearbetar CAN-meddelanden
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    readCanMessage(canMessage);
    sendToPython(canMessage);
    printCanMessage(canMessage);
  }
}

void sendCanMessage() {
  unsigned char stmp[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  stmp[7] = stmp[7] + 1;
  if (stmp[7] == 100) {
    stmp[7] = 0;
    stmp[6] = stmp[6] + 1;
    if (stmp[6] == 100) {
      stmp[6] = 0;
      stmp[5] = stmp[5] + 1;
    }
  }
  CAN.sendMsgBuf(0x00, 0, 8, stmp);
  delay(100);
}

void readCanMessage(CanData &canMessage) {
  byte len = 0;
  CAN.readMsgBuf(&canMessage.canId, &canMessage.length, canMessage.data); // readMsgBuf-anrop
}

void sendToPython(const CanData &canMessage) {
  // Anslut till Python-mottagaren via TCP/IP
  WiFiClient client;
  if (client.connect(host, port)) {
    // Skicka data till Python-mottagaren
    client.print("ID: ");
    client.print(canMessage.canId, HEX);
    client.print(" Data: ");
    for (int i = 0; i < canMessage.length; i++) {
      client.print(canMessage.data[i], HEX);
      client.print(" ");
    }
    client.println();
    client.stop();
  } else {
    Serial.println("Connection to server failed!");
  }
}

void printCanMessage(const CanData &canMessage) {
  Serial.print("ID: ");
  Serial.print(canMessage.canId, HEX);
  Serial.print(" Data: ");
  for (int i = 0; i < canMessage.length; i++) {
    Serial.print(canMessage.data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
} 

