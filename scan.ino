#include <SoftwareSerial.h>

#define debug Serial
#define PACKET_BUFFER_SIZE (128)
#define TX_PIN 3
#define RX_PIN 2

SoftwareSerial bike(RX_PIN, TX_PIN); // RX, TX

byte ECU_WAKEUP_MESSAGE[] = {0xFE, 0x04, 0x72, 0x8C}; 
byte ECU_INIT_MESSAGE[] = {0x72, 0x05, 0x00, 0xF0, 0x99};
int ECU_SUCCESS_CHECKSUM = 0x6FB;

void setup() {
  debug.begin(115200);
  initHonda();
  delay(50);
}

void loop() {
  //scanECUTables();

  showDataTable11();

  /*
  byte data[] = {0x72, 0x05, 0x71, 0xD1};
  int chk = calcChecksum(data, sizeof(data));
  byte dataWithChk[] = {0x72, 0x05, 0x71, 0xD1, chk};

  bike.write(dataWithChk, sizeof(dataWithChk));
  delay(50);

  bufferECUResponse();*/

  
  delay(500);
}

void showDataTable11() {

  byte data[] = {0x72, 0x05, 0x71, 0x11};
  int chk = calcChecksum(data, sizeof(data));
  byte dataWithChk[] = {0x72, 0x05, 0x71, 0x11, chk};

  bike.write(dataWithChk, sizeof(dataWithChk));
  delay(50);
  
  int buffCount = 0;
  byte buff[PACKET_BUFFER_SIZE];  
  while ( (bike.available() > 0 ) && ( buffCount < PACKET_BUFFER_SIZE)) {
    buff[buffCount++] = bike.read();
  }

  debug.print("RAW: ");
    for(int i=0; i<buffCount; i++) {
    debug.print(buff[i], HEX);
    if(i != buffCount-1) {
        debug.print(";");  
    }
  }
  debug.println();  
 /*
  //RPM - 8/9
  debug.print("RPM: ");
  int rpm = (uint16_t) (buff[8] << 8) + buff[9];
  debug.print(rpm);

  debug.println();

  debug.print("TPS V: ");
  int tpsv = calcValueDivide256(buff[10]);
  debug.print(tpsv);
  debug.println();

  debug.print("TPS %: ");
  int tpsp = calcValueDivide16(buff[11]);
  debug.print(tpsp);
  debug.print(" ");

  debug.print("ECT V: ");
  int ectv = calcValueDivide256(buff[12]);
  debug.print(ectv);
  debug.print(" ");

  debug.print("ECT C: ");
  int ectc = calcValueMinus40(buff[13]);
  debug.print(ectc);
  debug.print(" ");

  debug.print("IAT V: ");
  int iatv = calcValueDivide256(buff[14]);
  debug.print(iatv);
  debug.print(" ");

  debug.print("IAT C: ");
  int iatc = calcValueMinus40(buff[15]);
  debug.print(iatc);
  debug.print(" ");

  debug.print("MAP V: ");
  int mapv = calcValueDivide256(buff[16]);
  debug.print(mapv);
  debug.print(" ");

  debug.print("MAP kPa: ");
  int mapk = buff[17];
  debug.print(mapk);
  debug.print(" ");

  debug.print("BATT V: ");
  int batv = calcValueDivide10(buff[20]);
  debug.print(batv);
  debug.print(" ");

  debug.print("SPEED KMH ");
  int spdk = buff[21];
  debug.print(spdk);
  debug.print(" ");
*/
  debug.print("?? 24 ");
  int uk1 = buff[24];
  debug.print(uk1);
  debug.print(" ");

  debug.print("?? 25 ");
  int uk2= buff[225];
  debug.print(uk2);
  debug.print(" ");

  debug.print("?? 26 ");
  int uk3 = buff[26];
  debug.print(uk3);
  debug.print(" ");

  debug.print("?? 27 ");
  int uk4 = buff[27];
  debug.print(uk4);
  debug.print(" ");

  debug.println();


  
  
  /*
   * 00 - 0000rpm
00 - 0000rpm
1B TPS Volt - 0
00 TPS - 0
9E ECT Volt - 3v Coolant
3D ECT Deg - 21c Coolant
9E IAT Volt - 3v Air
3D IAT Deg - 21c Air
93 MAP Volt - 2v 
64 MAP Kpa - 100kpa
FF ??
FF ??
69 Batt Volt - 10.5v
0 Speed - 
0 Fuel Inj
0 Fuel Inj
80 ??
57 ??
28 ??
80 ??
   */



  
}



//Calc methods

float calcValueDivide256(int val) {
  //convert to dec, multiple by 5, then divide result by 256
  //used for TPS Volt, ECT Volt, IAT Volt, MAP Volt
  
  return (val*5)/256;
}

float calcValueMinus40(int val) {
  //value minus 40
  //used for ECT Temp, IAT Temp
  
  return val-40;
}

float calcValueDivide10(int val) {
  //value divided by 10
  //used for Batt Volt
  
  return val/10;
}

float calcValueDivide16(int val) {
  //value divided by 16
  //used for TPS%
  
  return val/16;
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
}

byte initHonda() {
  //Honda ecu communication handshake

  int initSuccess = 0;

  //while(initSuccess = 0) {
    debug.println("Starting up...");
    debug.println("Setting line low 70ms, high 120ms");
    initComms();
    
    bike.begin(10400);
    debug.println("Sending ECU Wakeup");
    bike.write(ECU_WAKEUP_MESSAGE, sizeof(ECU_WAKEUP_MESSAGE));
    delay(200);
    debug.println("Sending ECU Init String");
    bike.write(ECU_INIT_MESSAGE, sizeof(ECU_INIT_MESSAGE));
    bike.flush();
    delay(50);

    int initBuffCount = 0;
    byte initBuff[32];
    while ( bike.available() > 0  && initBuffCount < 32 ) {
      initBuff[initBuffCount++] = bike.read();
    }

    int initSum = 0;
    for(int i=0; i<initBuffCount; i++) {
      initSum += initBuff[i];
    }
  
    if(initSum == ECU_SUCCESS_CHECKSUM) {
      debug.println("Successfully opened connection to ECU");
      initSuccess = 1;
      return 1;
    }
    else {
      debug.println("Failed to open connection to ECU, trying again in 2s");
      delay(2000);
    }
  //}
}

int initComms() {
  //Honda ECU Init sequence
  pinMode(TX_PIN, OUTPUT);
  digitalWrite(TX_PIN, LOW); //TX Low for 70ms
  delay(70);
  digitalWrite(TX_PIN, HIGH); //TX High for 120ms
  delay(120);
  return 1;
}

uint8_t calcChecksum(const uint8_t* data, uint8_t len) {
   uint8_t cksum = 0;
   for (uint8_t i = 0; i < len; i++)
      cksum -= data[i];
   return cksum;
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
    delay(100);
    
    debug.print("Response: ");
    bufferECUResponse();
    //delay(500);
  }
}