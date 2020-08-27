#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Fonts/DS_DIGI16pt7b.h> //DS_DIGI 16pt
#include <Fonts/DS_DIGI6pt7b.h> //DS_DIGI 6pt
#include <Fonts/DS_DIGI60pt7b.h> //DS_DIGI 60pt
#include <Fonts/Furara8pt7b.h> //Futara 8pt

#define TFT_DC 2
#define TFT_CS 5
#define TFT_RST 4

#define debug Serial
#define bike Serial2
#define PACKET_BUFFER_SIZE (128)
#define TX_PIN 17
#define RX_PIN 16

//SoftwareSerial bike(RX_PIN, TX_PIN); // RX, TX

byte ECU_WAKEUP_MESSAGE[] = {0xFE, 0x04, 0x72, 0x8C}; 
byte ECU_INIT_MESSAGE[] = {0x72, 0x05, 0x00, 0xF0, 0x99};
int ECU_SUCCESS_CHECKSUM = 0x6FB;

//values to display

int rpm = 0;
int engTempC = 0;
float ambiC = 0;
float battV = 0;
int tpsP = 0;
float iatC = 0;
int gear = 0;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

int RPM_CONST = 310;
int TEMP_CONST = 10;

void setup() {

  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  drawUIElements();
  
  debug.begin(115200);
  initHonda();
  delay(50);
 
}

void loop() {

  showDataTable11();

  //draw code
  drawRPMBar(rpm);
  drawTempBar(engTempC);
  drawAmbiTemp(ambiC);
  drawBattVolt(battV);
  drawTPS(tpsP);
  drawIATC(iatC);
  drawGear(gear);
  
  //delay(50);

}

// UI Code
void drawUIElements() {
  //used for static elements
  drawRPMUI();
  drawGearIndBox();
  drawIndicatorLightBox();
  drawEngTempBar();
}

void drawRPMBar(int rpm) { //32 bars
  int numBars = rpm / RPM_CONST;
  
  for (int i = 0; i <= 32; i++) { //32 bars max
    if(i <= numBars) {
        if(i <= 20) {
          tft.fillRect(i*10,32,6,25, ILI9341_GREEN);
        }
        if(i > 20 && i <= 25) {
          tft.fillRect(i*10,32,6,25, ILI9341_YELLOW);
        }
        if(i > 25 && i <= 32) {
          tft.fillRect(i*10,32,6,25, ILI9341_RED);
        }
    }
    else {
      tft.fillRect(i*10,32,6,25, 0x0200);
    }
  }   
  
  char buf[6];
  sprintf(buf, "%05d", rpm);
  GFXcanvas16 canvas(90, 24);
  canvas.setTextColor(ILI9341_WHITE);
  canvas.setFont(&DS_DIGI16pt7b);
  canvas.setCursor(0,20);
  canvas.print(buf);
  tft.drawRGBBitmap(240,72,canvas.getBuffer(),90,24);
}

void drawGear(int gear) {

  String gearS;

  if(gear == 0) {
    gearS = "N";
  }
  else {
    gearS = gear;
  }
  
  GFXcanvas16 canvas(58, 78);
  canvas.setTextColor(ILI9341_WHITE);
  canvas.setFont(&DS_DIGI60pt7b);
  canvas.setCursor(0,75);
  canvas.print(gearS);
  tft.drawRGBBitmap(255,102,canvas.getBuffer(),58,78);
}

void drawTempBar(int temp) { // 12 bars

  int numBars = temp / TEMP_CONST;
  
  for (int i = 0; i <= 12; i++) { //12 bars max
    if(i <= numBars) {
        if(i < 9) {
          tft.fillRect(0,190-(i*10),20,6, ILI9341_GREEN);
        }
        if(i == 9) {
          tft.fillRect(0,190-(i*10),20,6, ILI9341_YELLOW);
        }
        if(i > 9 && i <= 12) {
          tft.fillRect(0,190-(i*10),20,6, ILI9341_RED);
        }
    }
    else {
      tft.fillRect(0,190-(i*10),20,6, 0x0200);
    }
  }   
  
  char buf[4];
  sprintf(buf, "%03d", temp);
  GFXcanvas16 canvas(40, 24);
  canvas.setTextColor(ILI9341_WHITE);
  canvas.setFont(&DS_DIGI16pt7b);
  canvas.setCursor(0,20);
  canvas.print(buf);
  tft.drawRGBBitmap(0,200,canvas.getBuffer(),40,24);
}

void drawBattVolt(float volts) {
  GFXcanvas16 canvas(50, 22);
  canvas.setTextColor(ILI9341_WHITE);
  canvas.setFont(&DS_DIGI16pt7b);
  canvas.setCursor(0,20);
  canvas.print(volts,1);
  tft.drawRGBBitmap(64,200,canvas.getBuffer(),50,22);
}

void drawTPS(int tps) {
  char buf[6];
  sprintf(buf, "%03d", tps);
  
  GFXcanvas16 canvas(50, 24);
  canvas.setTextColor(ILI9341_WHITE);
  canvas.setFont(&DS_DIGI16pt7b);
  canvas.setCursor(0,20);
  canvas.print(buf);
  tft.drawRGBBitmap(128,200,canvas.getBuffer(),50,24);
}

void drawIATC(float iatc) {
  GFXcanvas16 canvas(50, 22);
  canvas.setTextColor(ILI9341_WHITE);
  canvas.setFont(&DS_DIGI16pt7b);
  canvas.setCursor(0,20);
  canvas.print(iatc,1);
  tft.drawRGBBitmap(192,200,canvas.getBuffer(),50,22);
}

void drawAmbiTemp(float temp) {
  GFXcanvas16 canvas(50, 24);
  canvas.setTextColor(ILI9341_WHITE);
  canvas.setFont(&DS_DIGI16pt7b);
  canvas.setCursor(0,20);
  canvas.print(temp,1);
  tft.drawRGBBitmap(256,200,canvas.getBuffer(),50,24);
}

void drawRPMUI() {

  //bar under gauge
  tft.drawFastHLine(0,59,250,ILI9341_WHITE);
  tft.drawFastHLine(0,60,250,ILI9341_WHITE);
  tft.drawFastHLine(251,59,70,ILI9341_RED);
  tft.drawFastHLine(251,60,70,ILI9341_RED);

  //text for numbers under bar
  tft.setFont();
  tft.setTextColor(ILI9341_WHITE,ILI9341_BLACK);
  tft.setTextSize(1);
  
  int cursorPos = 0;
  for (int i = 0; i <= 9; i++) { 
    cursorPos = cursorPos+32;
    int gaugeNum = i+1;  
    
    if(gaugeNum >= 8) { //red text for 8-10k
        tft.setTextColor(ILI9341_RED,ILI9341_BLACK);
    }
    if(gaugeNum == 1) { // bring 1 in by 2px
      cursorPos = cursorPos - 2;
    }
    if(gaugeNum == 10) { // bring 10 in by 8px
      cursorPos = cursorPos - 10;
    }
    
    tft.setCursor(cursorPos,62);
    tft.print(gaugeNum);
  }
}

void drawGearIndBox() {

  tft.drawRect(254,102,60,80, ILI9341_GREEN);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(&Furara8pt7b);
  tft.setCursor(264,194);
  tft.print("GEAR");
  
}

void drawEngTempBar() {

  tft.drawFastVLine(24,110,86,ILI9341_WHITE);
  tft.drawFastVLine(25,110,86,ILI9341_WHITE);
  tft.fillRect(25,194,4,2, ILI9341_WHITE);
  tft.fillRect(25,110,4,2, ILI9341_WHITE);
  
  tft.drawFastVLine(24,70,26,ILI9341_RED);
  tft.drawFastVLine(25,70,26,ILI9341_RED);
  tft.fillRect(25,70,4,2, ILI9341_RED);
  tft.fillRect(25,94,4,2, ILI9341_RED);
  
  tft.setFont(&Furara8pt7b);
  tft.setCursor(0,236);
  tft.print("TEMP °C");

  tft.setCursor(64,236);
  tft.print("BATT V");

  tft.setCursor(128,236);
  tft.print("TPS %");

  tft.setCursor(192,236);
  tft.print("IAT °C");

  tft.setCursor(256,236);
  tft.print("AMBI °C");
}

void drawIndicatorLightBox() {

  tft.drawRect(0,0,320,32, ILI9341_GREEN);
  //for bottom vals
  tft.drawRect(0,200,64,40, ILI9341_GREEN);
  tft.drawRect(64,200,64,40, ILI9341_GREEN);
  tft.drawRect(128,200,64,40, ILI9341_GREEN);
  tft.drawRect(192,200,64,40, ILI9341_GREEN);
  tft.drawRect(256,200,64,40, ILI9341_GREEN);
}

//Shows most other info
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
   * int rpm = 0;
int engTempC = 0;
float ambiC = 0;
float battV = 0;
int tpsP = 0;
float iatC = 0;
int gear = 0;
  */

  //RPM - 8/9
  debug.print("RPM: ");
  rpm = (uint16_t) (buff[9] << 8) + buff[10];
  debug.print(rpm);
  debug.print(" ");

  debug.print("TPS V: ");
  int tpsv = calcValueDivide256(buff[11]);
  debug.print(tpsv);
  debug.print(" ");

  debug.print("TPS %: ");
  tpsP = calcValueDivide16(buff[12]);
  debug.print(tpsP);
  debug.print(" ");

  debug.print("ECT V: ");
  int ectv = calcValueDivide256(buff[13]);
  debug.print(ectv);
  debug.print(" ");

  debug.print("ECT C: ");
  engTempC = calcValueMinus40(buff[14]);
  debug.print(engTempC);
  debug.print(" ");

  debug.print("IAT V: ");
  int iatv = calcValueDivide256(buff[15]);
  debug.print(iatv);
  debug.print(" ");

  debug.print("IAT C: ");
  iatC = calcValueMinus40(buff[16]);
  debug.print(iatC);
  debug.print(" ");

  debug.print("MAP V: ");
  int mapv = calcValueDivide256(buff[17]);
  debug.print(mapv);
  debug.print(" ");

  debug.print("MAP kPa: ");
  int mapk = buff[18];
  debug.print(mapk);
  debug.print(" ");

  debug.print("BATT V: ");
  battV = calcValueDivide10(buff[21]);
  debug.print(battV);
  debug.print(" ");

  debug.print("SPEED KMH ");
  int spdk = buff[22];
  debug.print(spdk);
  debug.print(" ");

  debug.println();
}

//Shows info like neutral switch, engine on
void showDataTableD1() { 
  byte data[] = {0x72, 0x05, 0x71, 0xD1};
  int chk = calcChecksum(data, sizeof(data));
  byte dataWithChk[] = {0x72, 0x05, 0x71, 0xD1, chk};
  
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

  debug.print("Switch State: ");
  int sws = buff[9];
  debug.print(sws);
  debug.print(" ");

  debug.print("Engine State: ");
  int ens = buff[13];
  debug.print(ens);
  debug.print(" ");
  
  debug.println();

  
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
