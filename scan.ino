#include <SoftwareSerial.h>

// Serial Debug USB
#define debug Serial
#define PACKET_BUFFER_SIZE (64)
SoftwareSerial bike(2, 3); // RX, TX

//initialise communications messages
byte MessageWakeup[] = {0xFE, 0x04, 0x72, 0x8C}; 
byte MessageInitialise[] = {0x72, 0x05, 0x00, 0xF0, 0x99};

void setup() {
  debug.begin(115200);
  debug.println("Starting Communications");
  initHonda();
  delay(50);
  debug.print("Init Response: ");
  debugSerialInput(); //TODO: move this to init method;
}

void loop() {
  scanECUTables();
}

uint8_t calcChecksum(const uint8_t* data, uint8_t len) {
   uint8_t cksum = 0;
   for (uint8_t i = 0; i < len; i++)
      cksum -= data[i];
   return cksum;
}

void debugSerialInput() {
  while (bike.available() > 0) {
      uint8_t val = bike.read();
      debug.print(val,HEX);
      debug.print(";");
  }
  debug.write("\n");
}

void scanECUTables() {

  for(int i=0; i<255; i++) { //scan all 256 tables
    debug.print("Sending table: ");
    debug.print(i, HEX);
    debug.print(" (Decimal: ");
    debug.print(i, DEC);
    debug.print(")");
    debug.print("\n");

    byte data[] = {0x72, 0x05, 0x71, i};
    int chk = calcChecksum(data, sizeof(data));
    byte dataWithChk[] = {0x72, 0x05, 0x71, i, chk};
   
    bike.write(dataWithChk, sizeof(dataWithChk));
    debug.print("Response: ");
    bufferECUResponse();
    delay(500);
  }

}

byte bufferECUResponse() {
  int buffCount = 0;
  byte buff[PACKET_BUFFER_SIZE];
  byte sum = 0;
  
  while ( (bike.available() > 0 ) && ( buffCount < PACKET_BUFFER_SIZE)) {
    buff[buffCount++] = bike.read();
  }

  for(int i=0; i<buffCount; i++) {
    debug.print(buff[i], HEX);
    if(i != buffCount-1) {
        debug.print(";");  
    }
  }
  debug.println();  
  return 0;
}

int initComms() {
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);
  delay(70);
  digitalWrite(3, HIGH);
  delay(120);
  return 1;
}

byte initHonda(){

  //TODO: Hold in while loop until we recieve ECU handshake
  
  debug.println("Setting line low 70ms, high 120ms");
  initComms();
  debug.println("Sending First Init String");
  bike.begin(10400);
  bike.write(MessageWakeup, sizeof(MessageWakeup)); //Send WakeUp
  delay(200);
  debug.println("Sending Second Init String");
  bike.write(MessageInitialise, sizeof(MessageInitialise)); // initialise communications
  bike.flush(); // wait to send all
  return 1;
}
