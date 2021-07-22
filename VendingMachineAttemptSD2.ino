#include <Adafruit_Fingerprint.h>
#include <stdint.h>
#include "TouchScreen.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <MCUFRIEND_kbv.h>
#include <Fonts\FreeSans18pt7b.h>
#include <Fonts\FreeSansBold24pt7b.h>
#include <Fonts\FreeSansBold12pt7b.h>
#include <Fonts\FreeSansBold18pt7b.h>
#include <Fonts\FreeSans9pt7b.h>
#include <EEPROM.h>
#include <Servo.h>


SoftwareSerial mySerial(12,3);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
Servo servo_3;

#include <SPI.h>          // f.k. for Arduino-1.5.2
//#define USE_SDFAT
#include <SD.h>
MCUFRIEND_kbv tft;
#define SD_CS 5

File root;
char namebuf[32] = "plswork.bmp";
int pathlen;
uint8_t         spi_save;


// TFT Breakout  -- Arduino UNO / Mega2560 / OPEN-SMART UNO Black
// GND              -- GND
// 3V3               -- 3.3V
// CS                 -- A3
// RS                 -- A2
// WR                -- A1
// RD                 -- A0
// RST                -- RESET
// LED                -- GND
// DB0                -- 8
// DB1                -- 9
// DB2                -- 10
// DB3                -- 11
// DB4                -- 4
// DB5                -- 13
// DB6                -- 6
// DB7                -- 7

// Assign human-readable names to some common 16-bit color values:
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define MINPRESSURE 10
#define MAXPRESSURE 1000
#define DARKGREEN 0x1420
#define DARKRED 0x8000


int startScan=2;
int waitUntil=1;
int waitUntil_2=1;
int waitUntil_3=1;
int waitUntil_4=1;
int waitUntil_5=1;
int waitUntil_6=1;
int amountCount=2;
bool darkCheck=false;
uint16_t ID;
uint8_t YP = A1;  // must be an analog pin, use "An" notation!
uint8_t XM = A2;  // must be an analog pin, use "An" notation!
uint8_t YM = 7;   // can be a digital pin
uint8_t XP = 6;   // can be a digital pin
uint16_t pLEFT = 950;
uint16_t pRIGHT  = 170;
uint16_t pTOP = 200;
uint16_t pBOTTOM = 890;
//uint16_t pLEFT = 880;
//uint16_t pRIGHT  = 170;
//uint16_t pTOP = 180;
//uint16_t pBOTTOM = 950;
int v1;
uint16_t xpos, ypos; 
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
 uint16_t read16(File f) {
   uint16_t result;
   ((uint8_t *)&result)[0] = f.read(); // LSB
   ((uint8_t *)&result)[1] = f.read(); // MSB
   return result;
 }
 
 uint32_t read32(File f) {
   uint32_t result;
   ((uint8_t *)&result)[0] = f.read(); // LSB
   ((uint8_t *)&result)[1] = f.read();
   ((uint8_t *)&result)[2] = f.read();
   ((uint8_t *)&result)[3] = f.read(); // MSB
   return result;
 }

void setup() {
uint16_t ID;

    Serial.begin(9600);
    Serial.print("Show BMP files on TFT with ID:0x");
    finger.begin(57600);
    finger.getTemplateCount();
    ID = tft.readID();
    Serial.println(ID, HEX);
    if (ID == 0x0D3D3) ID = 0x9481;
    tft.begin(ID);
    tft.fillScreen(0x0000);
  /*  if (tft.height() > tft.width()) tft.setRotation(1);    //LANDSCAPE
    tft.setTextColor(0xFFFF, 0x0000);*/
    bool good = SD.begin(SD_CS);
    if (!good) {
        Serial.print(F("cannot start SD"));
        while (1);
    }
tft.setRotation(1);
randomSeed(analogRead(0));
tft.fillScreen(WHITE);
//if (darkCheck==false) EEPROM.write(0, 2);
//else if (darkCheck==true) EEPROM.write(0, 1);
    pinMode(51,OUTPUT);
    digitalWrite(51,HIGH);
    delay(600);
    digitalWrite(51,LOW);
bmpDraw("pog.bmp",0,0);
}
#define BUFFPIXEL 20
 
 void bmpDraw(char *filename, int x, int y) {
   File   bmpFile;
   int    bmpWidth, bmpHeight;   // W+H in pixels
   uint8_t  bmpDepth;        // Bit depth (currently must be 24)
   uint32_t bmpImageoffset;      // Start of image data in file
   uint32_t rowSize;         // Not always = bmpWidth; may have padding
   uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel in buffer (R+G+B per pixel)
   uint16_t lcdbuffer[BUFFPIXEL];  // pixel out buffer (16-bit per pixel)
   uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
   boolean  goodBmp = false;     // Set to true on valid header parse
   boolean  flip  = true;      // BMP is stored bottom-to-top
   int    w, h, row, col;
   uint8_t  r, g, b;
   uint32_t pos = 0, startTime = millis();
   uint8_t  lcdidx = 0;
   boolean  first = true;
 
   if((x >= tft.width()) || (y >= tft.height())) return;
 
   Serial.println();
   Serial.print("Loading image '");
   Serial.print(filename);
   Serial.println('\'');
   // Open requested file on SD card
   SPCR = spi_save;
   if ((bmpFile = SD.open(filename)) == NULL) {
   Serial.print("File not found");
   return;
   }
 
   // Parse BMP header
   if(read16(bmpFile) == 0x4D42) { // BMP signature
   Serial.print(F("File size: ")); Serial.println(read32(bmpFile));
   (void)read32(bmpFile); // Read & ignore creator bytes
   bmpImageoffset = read32(bmpFile); // Start of image data
   Serial.print(F("Image Offset: ")); Serial.println(bmpImageoffset, DEC);
   // Read DIB header
   Serial.print(F("Header size: ")); Serial.println(read32(bmpFile));
   bmpWidth  = read32(bmpFile);
   bmpHeight = read32(bmpFile);
   if(read16(bmpFile) == 1) { // # planes -- must be '1'
     bmpDepth = read16(bmpFile); // bits per pixel
     Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
     if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed
 
     goodBmp = true; // Supported BMP format -- proceed!
     Serial.print(F("Image size: "));
     Serial.print(bmpWidth);
     Serial.print('x');
     Serial.println(bmpHeight);
 
     // BMP rows are padded (if needed) to 4-byte boundary
     rowSize = (bmpWidth * 3 + 3) & ~3;
 
     // If bmpHeight is negative, image is in top-down order.
     // This is not canon but has been observed in the wild.
     if(bmpHeight < 0) {
       bmpHeight = -bmpHeight;
       flip    = false;
     }
 
     // Crop area to be loaded
     w = bmpWidth;
     h = bmpHeight;
     if((x+w-1) >= tft.width())  w = tft.width()  - x;
     if((y+h-1) >= tft.height()) h = tft.height() - y;
 
     // Set TFT address window to clipped image bounds
     SPCR = 0;
     tft.setAddrWindow(x, y, x+w-1, y+h-1);
 
     for (row=0; row<h; row++) { // For each scanline...
       // Seek to start of scan line.  It might seem labor-
       // intensive to be doing this on every line, but this
       // method covers a lot of gritty details like cropping
       // and scanline padding.  Also, the seek only takes
       // place if the file position actually needs to change
       // (avoids a lot of cluster math in SD library).
       if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
       pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
       else   // Bitmap is stored top-to-bottom
       pos = bmpImageoffset + row * rowSize;
       SPCR = spi_save;
       if(bmpFile.position() != pos) { // Need seek?
       bmpFile.seek(pos);
       buffidx = sizeof(sdbuffer); // Force buffer reload
       }
 
       for (col=0; col<w; col++) { // For each column...
       // Time to read more pixel data?
       if (buffidx >= sizeof(sdbuffer)) { // Indeed
         // Push LCD buffer to the display first
         if(lcdidx > 0) {
         SPCR = 0;
         tft.pushColors(lcdbuffer, lcdidx, first);
         lcdidx = 0;
         first  = false;
         }
         SPCR = spi_save;
         bmpFile.read(sdbuffer, sizeof(sdbuffer));
         buffidx = 0; // Set index to beginning
       }
 
       // Convert pixel from BMP to TFT format
       b = sdbuffer[buffidx++];
       g = sdbuffer[buffidx++];
       r = sdbuffer[buffidx++];
       lcdbuffer[lcdidx++] = tft.color565(r,g,b);
       } // end pixel
     } // end scanline
     // Write any remaining data to LCD
     if(lcdidx > 0) {
       SPCR = 0;
       tft.pushColors(lcdbuffer, lcdidx, first);
     } 
     Serial.print(F("Loaded in "));
     Serial.print(millis() - startTime);
     Serial.println(" ms");
     } // end goodBmp
   }
   }
 
   bmpFile.close();
   if(!goodBmp) Serial.println("BMP format not recognized.");
 
   delay(2000);
   bootScreen();
 }

void bootScreen(){
  tft.fillScreen(WHITE); 
  if (EEPROM.read(0) == 1) {
  tft.invertDisplay(false);
  darkCheck=false;
  }
  else if (EEPROM.read(0) == 2) {
  tft.invertDisplay(true);
  darkCheck=true;
  }
  tft.setFont(&FreeSansBold24pt7b);
  tft.setCursor(0,50);
  tft.setTextColor(BLACK);
  tft.print("Secure Vending Machine");
  tft.setFont(&FreeSansBold12pt7b);
  tft.setCursor(0,150);
  tft.print("Made by Shimon");
  tft.setCursor(290,230);
  tft.print("1.0 BETA");
  delay(4000);
  
  homeScreen();
}

void homeScreen(){  
  startScan=2;
  waitUntil=1;
  waitUntil_4=1;
  waitUntil_5=1;
  v1 = random(0, 2);
  //tft.invertDisplay(true);
  tft.fillScreen(WHITE); 
  tft.setFont(&FreeSansBold24pt7b);
  tft.setTextColor(BLACK);
  if (v1 ==0){
  tft.setCursor(90,65);
  tft.print("Welcome!");
  }
  else if (v1 == 1){
  tft.setCursor(135,65);
  tft.print("Hello!");
  }

   tft.drawLine(20, 85, 380, 85, BLACK);
   tft.fillRoundRect(150, 100, 100, 100, 20, BLACK);
   tft.fillRoundRect(157, 107, 86, 86, 15, WHITE);
   tft.fillTriangle(175, 180, 175, 120, 225, 150, BLACK);
   tft.fillRoundRect(5, 205, 73, 30, 5, BLACK);
   tft.fillRoundRect(7, 206, 69, 26, 3, WHITE);
   tft.setFont(&FreeSans9pt7b);
   tft.setCursor(10,225);
   tft.print("Options");
   playTouch();
}


void playTouch(){
    do {
    TSPoint p = ts.getPoint();
    xpos = map(p.y, pLEFT, pRIGHT, 0, 400);
    ypos = map(p.x, pTOP, pBOTTOM, 0, 240);
    if ( p.z > MINPRESSURE && p.z < MAXPRESSURE && xpos<70 && xpos>0 && ypos>200 && ypos<240 ){
    highlightOptions();
    }
    if ( p.z > MINPRESSURE && p.z < MAXPRESSURE && xpos<250 && xpos>150 && ypos>100 && ypos<200 ){
    highlightPlay();
    }
   } 
    while (waitUntil<2); 
    
}

void highlightOptions(){
  waitUntil=3;
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
   tft.fillRoundRect(5, 205, 73, 30, 5, GREEN);
   tft.fillRoundRect(7, 206, 69, 26, 3, WHITE);
   tft.setFont(&FreeSans9pt7b);
   tft.setTextColor(GREEN);
   tft.setCursor(10,225);
   tft.print("Options");
   delay(100);
   waitUntil=1;
   optionScreen();
   
  
}
void optionScreen(){
  tft.fillScreen(WHITE);
  //dark mode
  tft.drawLine(20, 85, 380, 85, BLACK);
  tft.drawLine(20, 15, 380, 15, BLACK);
  tft.setFont(&FreeSansBold18pt7b);
  tft.setTextColor(BLACK);
  tft.setCursor(35,65);
  tft.print("Dark Mode:");
  tft.fillRoundRect(305, 31,70, 40, 7, BLACK);
  tft.fillRoundRect(309, 35, 62, 32, 4, WHITE);
  tft.setFont(&FreeSans9pt7b);
  if (darkCheck==false){
  tft.setCursor(320,55);
  tft.print("OFF");
  }
  else {
  tft.setCursor(324,55);
  tft.print("ON");
  }
  //back button
   tft.fillRoundRect(5, 205, 60, 30, 5, BLACK);
   tft.fillRoundRect(7, 206, 56, 26, 3, WHITE);
   tft.setFont(&FreeSans9pt7b);
   tft.setCursor(14,225);
   tft.print("Back");
   //fingerprint manager
  tft.drawLine(20, 90, 380, 90, BLACK);
  tft.drawLine(20, 160, 380, 160, BLACK);
  tft.setFont(&FreeSansBold12pt7b);
  tft.setTextColor(BLACK);
  tft.setCursor(35,135);
  tft.print("Fingerprints:");
  tft.setCursor(255,131);
  tft.setFont(&FreeSans9pt7b);
  tft.print("-Coming soon!-");
   optionTouch();
  
}

void optionTouch(){
    pinMode(XM, INPUT);
    pinMode(YP, INPUT);
do {
    TSPoint p = ts.getPoint();
    xpos = map(p.y, pLEFT, pRIGHT, 0, 400);
    ypos = map(p.x, pTOP, pBOTTOM, 0, 240);
    if( p.z > MINPRESSURE && p.z < MAXPRESSURE && xpos<70 && xpos>0 && ypos>200 && ypos<240 ){
    highlightBack();
    }
    else if ( p.z > MINPRESSURE && p.z < MAXPRESSURE && xpos<375 && xpos>305 && ypos>31 && ypos<71 ){
    highlightDarkMode();
    }
   } 
    while (waitUntil_5<2); 
}

void highlightBack(){
   waitUntil_5=3;
   waitUntil_6=3;
   pinMode(XM, OUTPUT);
   pinMode(YP, OUTPUT);
   tft.fillRoundRect(5, 205, 60, 30, 5, GREEN);
   tft.fillRoundRect(7, 206, 56, 26, 3, WHITE);
   tft.setFont(&FreeSans9pt7b);
   tft.setTextColor(GREEN);
   tft.setCursor(14,225);
   tft.print("Back");
   delay(100);
   homeScreen();
}

void highlightDarkMode(){
  waitUntil_5=3;
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  
  if (darkCheck==true){
  tft.fillRoundRect(305, 31,70, 40, 7, GREEN);
  tft.fillRoundRect(309, 35, 62, 32, 4, WHITE);
  tft.setCursor(320,55);
  tft.setTextColor(GREEN);
  tft.print("OFF");
  delay(100);
  tft.fillRoundRect(305, 31,70, 40, 7, BLACK);
  tft.fillRoundRect(309, 35, 62, 32, 4, WHITE);
  tft.setTextColor(BLACK);
  tft.setCursor(320,55);
  tft.print("OFF");
  EEPROM.write(0, 1);
  tft.invertDisplay(false);
  darkCheck=false;



  }
  else {
  tft.fillRoundRect(305, 31,70, 40, 7, GREEN);
  tft.fillRoundRect(309, 35, 62, 32, 4, WHITE);
  tft.setCursor(324,55);
  tft.setTextColor(GREEN);
  tft.print("ON");
  delay(100);
  tft.fillRoundRect(305, 31,70, 40, 7, BLACK);
  tft.fillRoundRect(309, 35, 62, 32, 4, WHITE);
  tft.setTextColor(BLACK);
  tft.setCursor(324,55);
  tft.print("ON");
  tft.invertDisplay(true);
  EEPROM.write(0, 2);
  darkCheck=true;
  
  }
   waitUntil_5=1;
   optionTouch();
}
   int y=1;

void lockScreen(){
  y=1;
  servo_3.write(100);
  tft.fillScreen(WHITE);
  tft.setCursor(50,60);
  tft.setTextColor(BLACK);
  tft.setFont(&FreeSansBold12pt7b);
  tft.print("Please Enter Fingerprint");
  tft.drawLine(20, 80, 380, 80, BLACK);
  tft.fillCircle(200, 140, 30, BLACK);
  tft.fillCircle(200, 140, 22, WHITE);
  tft.fillRoundRect(157, 140, 86, 70, 15, BLACK);
  startScan=1;
  tft.setCursor(150,215);
  tft.setTextColor(BLACK);
  tft.setFont(&FreeSans9pt7b);
  //tft.print("Scanning");
  tft.fillRoundRect(5, 205, 60, 30, 5, BLACK);
  tft.fillRoundRect(7, 206, 56, 26, 3, WHITE);
  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(14,225);
  tft.print("Back");
  waitUntil_6=1;
  backLockTouch();

}
void backLockTouch(){
    pinMode(XM, INPUT);
    pinMode(YP, INPUT);
do {
    TSPoint p = ts.getPoint();
    xpos = map(p.y, pLEFT, pRIGHT, 0, 400);
    ypos = map(p.x, pTOP, pBOTTOM, 0, 240);
    if (getFingerprintIDez()==true){
    waitUntil_2=1;
    startScan=3;
    unlockScreen();
   }
    if( p.z > MINPRESSURE && p.z < MAXPRESSURE && xpos<70 && xpos>0 && ypos>200 && ypos<240 ){
    highlightBack();
    }

   } 
    while (waitUntil_6<2); 
}
//void scanAnimation(){
  // tft.setCursor(170,215);
  // tft.print(".");
  // delay(500);
  // tft.setCursor(173,215);
  // tft.print(".");
  // delay(500);
  // tft.setCursor(176,215);
   //tft.print(".");
   //delay(500);
   //fillRoundRect(160,213,8,5,1,WHITE);
//}

bool getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return false;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return false;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return false;
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return true;
   
}

void unlockScreen(){
    digitalWrite(51,HIGH);
    delay(100);
    digitalWrite(51,LOW);
    delay(100);
    digitalWrite(51,HIGH);
    delay(100);
    digitalWrite(51,LOW);
  startScan=2;
  pinMode(XM,OUTPUT);
  pinMode(YP,OUTPUT);
  y=3;
  tft.fillScreen(WHITE);
  tft.setCursor(85,60);
  tft.setTextColor(GREEN);
  tft.setFont(&FreeSansBold12pt7b);
  tft.print("ACCESS APPROVED");
  tft.drawLine(20, 80, 380, 80, BLACK);
  tft.fillCircle(245, 140, 30, BLACK);
  tft.fillCircle(245, 140, 22, WHITE);
  tft.fillRoundRect(157, 140, 200, 200, 15, WHITE);
  tft.fillRoundRect(157, 140, 86, 70, 15, BLACK);
  delay(1000);
  amountPicker();
}

void accesesDenied(){
  y=3;
   tft.fillScreen(WHITE); 
   tft.setFont(&FreeSansBold18pt7b);
   tft.setTextColor(RED);
   tft.setCursor(55,70);
   tft.print("ACCESS DENIED");
   tft.drawLine(20, 85, 380, 85, BLACK);
   tft.fillRoundRect(150, 100, 100, 100, 20, BLACK);
   tft.fillRoundRect(157, 107, 86, 86, 15, WHITE);
   tft.fillCircle(200, 150, 30, BLACK);
   tft.fillCircle(200, 150, 22, WHITE);
   tft.fillRect(162, 112, 40, 40, WHITE);
   tft.fillTriangle(185, 125, 201, 115, 201, 135, BLACK);
   tft.setFont(&FreeSans9pt7b);
   tft.setCursor(10,65);
   tft.setTextColor(BLACK);
   againTouch();
   //tft.print("If you aren't trying to steal my candy,Please try again");
   
  
}

void againTouch(){
    pinMode(XM, INPUT);
    pinMode(YP, INPUT);
   do {
    TSPoint p = ts.getPoint();

    if ( p.z > MINPRESSURE && p.z < MAXPRESSURE && xpos<250 && xpos>150 && ypos>100 && ypos<200 ){
    Serial.print("X = "); Serial.print(p.x);
    Serial.print("\tY = "); Serial.print(p.y);
    Serial.print("\tPressure = "); Serial.println(p.z);
    //tft.fillRoundRect(150, 100, 100, 100, 20, GREEN);
    highlightAgain();

    }
   } 
    while (waitUntil_2<2);
}
void highlightAgain(){
   waitUntil_2=3;
   pinMode(XM, OUTPUT);
   pinMode(YP, OUTPUT);
   tft.fillRoundRect(150, 100, 100, 100, 20, GREEN);
   tft.fillRoundRect(157, 107, 86, 86, 15, WHITE);
   tft.fillCircle(200, 150, 30, GREEN);
   tft.fillCircle(200, 150, 22, WHITE);
   tft.fillRect(162, 112, 40, 40, WHITE);
   tft.fillTriangle(185, 125, 201, 115, 201, 135, GREEN);
   delay(100);
   lockScreen();
}
void highlightPlay(){
  waitUntil=3;
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  tft.setCursor(0,0);
  tft.fillRoundRect(150, 100, 100, 100, 20, GREEN);
  tft.fillRoundRect(157, 107, 86, 86, 15, WHITE);
  tft.fillTriangle(175, 180, 175, 120, 225, 150, GREEN);
  delay(100);
  
  lockScreen();
}

void amountPicker(){
  tft.fillScreen(WHITE); 
  tft.setFont(&FreeSansBold18pt7b);
  tft.setTextColor(BLACK);
  tft.setCursor(60,65);
  tft.print("Choose Amount:");
  tft.drawLine(20, 85, 380, 85, BLACK);
  //- button
  tft.fillRoundRect(20,105,80,80,20,BLACK);
  tft.fillRoundRect(27,112,66,66,15,WHITE);
  //+ button
  tft.fillRoundRect(300,105,80,80,20,BLACK);
  tft.fillRoundRect(307,112,66,66,15,WHITE);
  //mid button
  tft.fillRoundRect(105,105,190,80,20,BLACK);
  tft.fillRoundRect(112,112,176,66,15,WHITE);
  //X button
  tft.fillRoundRect(118,190,80,40,20,BLACK);
  tft.fillRoundRect(123,195,70,30,15,WHITE);
  //v button
  tft.fillRoundRect(203,190,80,40,20,BLACK);
  tft.fillRoundRect(208,195,70,30,15,WHITE);
  // v icon
  tft.drawLine(239,220,249,200,DARKGREEN);
  tft.drawLine(238,220,248,200,DARKGREEN);
  tft.drawLine(240,220,250,200,DARKGREEN);
  tft.drawLine(239,220,232,214,DARKGREEN);
  tft.drawLine(240,220,233,214,DARKGREEN);
  tft.drawLine(241,220,234,214,DARKGREEN);
  //X icon
  tft.drawLine(143+4,200,173-4,220,DARKRED);
  tft.drawLine(144+4,200,174-4,220,DARKRED);
  tft.drawLine(142+4,200,172-4,220,DARKRED);
  tft.drawLine(173-4,200,143+4,220,DARKRED);
  tft.drawLine(172-4,200,142+4,220,DARKRED);
  tft.drawLine(174-4,200,144+4,220,DARKRED);
  //- icon
  tft.drawLine(37,145,83,145,BLACK);
  tft.drawLine(37,144,83,144,BLACK);
  tft.drawLine(37,146,83,146,BLACK);
  //+ icon
  tft.drawLine(317,145,363,145,BLACK);
  tft.drawLine(317,144,363,144,BLACK);
  tft.drawLine(317,146,363,146,BLACK);
  tft.drawLine(340,122,340,168,BLACK);
  tft.drawLine(339,122,339,168,BLACK);
  tft.drawLine(341,122,341,168,BLACK);
  tft.setFont(&FreeSansBold12pt7b);
  tft.setCursor(150,153);
  tft.setTextColor(BLACK);
  tft.print("Not Alot");
  waitUntil_3=1;
  amountTouch();
}

void amountTouch(){
    pinMode(XM, INPUT);
    pinMode(YP, INPUT);
   do {
    TSPoint p = ts.getPoint();
    xpos = map(p.y, pLEFT, pRIGHT, 0, 400);
    ypos = map(p.x, pTOP, pBOTTOM, 0, 240);
    if ( p.z > MINPRESSURE && p.z < MAXPRESSURE && xpos<380 && xpos>300 && ypos>105 && ypos<185 ){
    Serial.print("X = "); Serial.print(p.x);
    Serial.print("\tY = "); Serial.print(p.y);
    Serial.print("\tPressure = "); Serial.println(p.z);
    highlightPlus();
    
    }
    if ( p.z > MINPRESSURE && p.z < MAXPRESSURE && xpos<80 && xpos>20 && ypos>105 && ypos<185 ){
    Serial.print("X = "); Serial.print(p.x);
    Serial.print("\tY = "); Serial.print(p.y);
    Serial.print("\tPressure = "); Serial.println(p.z);
    highlightMinus();
    }
    if ( p.z > MINPRESSURE && p.z < MAXPRESSURE && xpos<283 && xpos>203 && ypos>190 && ypos<230 ){
    Serial.print("X = "); Serial.print(p.x);
    Serial.print("\tY = "); Serial.print(p.y);
    Serial.print("\tPressure = "); Serial.println(p.z);
    highlightVi();
    }
    if ( p.z > MINPRESSURE && p.z < MAXPRESSURE && xpos<198 && xpos>118 && ypos>190 && ypos<230 ){
    Serial.print("X = "); Serial.print(p.x);
    Serial.print("\tY = "); Serial.print(p.y);
    Serial.print("\tPressure = "); Serial.println(p.z);
    highlightX();
    }
    
    
   } 
    while (waitUntil_3<2);
}

void highlightPlus(){
  Serial.print("got to highlightPlus");
  waitUntil_3=3;
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  tft.fillRoundRect(300,105,80,80,20,GREEN);
  tft.fillRoundRect(307,112,66,66,15,WHITE);
  tft.drawLine(317,145,363,145,GREEN);
  tft.drawLine(317,144,363,144,GREEN);
  tft.drawLine(317,146,363,146,GREEN);
  tft.drawLine(340,122,340,168,GREEN);
  tft.drawLine(339,122,339,168,GREEN);
  tft.drawLine(341,122,341,168,GREEN);
  delay(100);
  tft.fillRoundRect(300,105,80,80,20,BLACK);
  tft.fillRoundRect(307,112,66,66,15,WHITE);
  tft.drawLine(317,145,363,145,BLACK);
  tft.drawLine(317,144,363,144,BLACK);
  tft.drawLine(317,146,363,146,BLACK);
  tft.drawLine(340,122,340,168,BLACK);
  tft.drawLine(339,122,339,168,BLACK);
  tft.drawLine(341,122,341,168,BLACK);
  amountCount++;
  if (amountCount>3){
    amountCount=3;
  }
  else if (amountCount>=3){
   tft.fillRoundRect(112,112,176,66,15,WHITE);
   tft.setCursor(142,153);
   tft.setTextColor(BLACK);
   tft.print("Too Much");
  }
   else if (amountCount==2){
   tft.fillRoundRect(112,112,176,66,15,WHITE);
   tft.setCursor(150,153);
   tft.setTextColor(BLACK);
   tft.print("Not Alot");
  }
  waitUntil_3=1;
  amountTouch();
}

void highlightMinus(){
  waitUntil_3=3;
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  tft.fillRoundRect(20,105,80,80,20,RED);
  tft.fillRoundRect(27,112,66,66,15,WHITE);
  tft.drawLine(37,145,83,145,RED);
  tft.drawLine(37,144,83,144,RED);
  tft.drawLine(37,146,83,146,RED);
  delay(100);
  tft.fillRoundRect(20,105,80,80,20,BLACK);
  tft.fillRoundRect(27,112,66,66,15,WHITE);
  tft.drawLine(37,145,83,145,BLACK);
  tft.drawLine(37,144,83,144,BLACK);
  tft.drawLine(37,146,83,146,BLACK);
  amountCount--;
  if (amountCount<1){
    amountCount=1;
  }
  else if (amountCount<=1){
   tft.fillRoundRect(112,112,176,66,15,WHITE);
   tft.setCursor(167,153);
   tft.setTextColor(BLACK);
   tft.print("A Bit");
  }

  else if (amountCount==2){
   tft.fillRoundRect(112,112,176,66,15,WHITE);
   tft.setCursor(150,153);
   tft.setTextColor(BLACK);
   tft.print("Not Alot");
  }
  waitUntil_3=1;
  amountTouch();
}

void highlightX(){
  waitUntil_3=3;
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  tft.fillRoundRect(118,190,80,40,20,RED);
  tft.fillRoundRect(123,195,70,30,15,WHITE);
  tft.drawLine(143+4,200,173-4,220,DARKRED);
  tft.drawLine(144+4,200,174-4,220,DARKRED);
  tft.drawLine(142+4,200,172-4,220,DARKRED);
  tft.drawLine(173-4,200,143+4,220,DARKRED);
  tft.drawLine(172-4,200,142+4,220,DARKRED);
  tft.drawLine(174-4,200,144+4,220,DARKRED);
  delay(100);
  //back to defult state
  amountCount=2;
  //back to home screen
  homeScreen();

}

void highlightVi(){
  waitUntil_3=3;
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  //tft.fillRoundRect(203,190,80,40,20,GREEN);
  //tft.fillRoundRect(208,195,70,30,15,WHITE);
  //tft.drawLine(239,220,249,200,GREEN);
  //tft.drawLine(238,220,248,200,GREEN);
  //tft.drawLine(240,220,250,200,GREEN);
  //tft.drawLine(239,220,232,214,GREEN);
  //tft.drawLine(240,220,233,214,GREEN);
  //tft.drawLine(241,220,234,214,GREEN);
  //delay(100);
  endScreen();
}

void endScreen(){
  servo_3.attach(49);
  if (amountCount==3){
  for (int k=100;k>=30 ;k--){
  servo_3.write(k);
  delay (3);
  }
  //delay(200);
  for (int i=30; i<=100 ;i++){
  servo_3.write(i);
  delay (3);
  }
  }
  else if (amountCount==2){
  for (int k=100;k>=40 ;k--){
  servo_3.write(k);
  delay (5);
  }
  delay(100);
  for (int i=40; i<=100 ;i++){
  servo_3.write(i);
  delay (3);
  }
  }
  else if (amountCount==1){
  for (int k=100;k>=37;k--){
  servo_3.write(k);
  delay (2);
  }
  delay(50);
  for (int i=37; i<=100 ;i++){
  servo_3.write(i);
  delay (2);
  }
  }
  
  servo_3.detach();
  amountCount=2;
  int v2 = random(0, 2);
  tft.fillScreen(WHITE); 
  tft.setFont(&FreeSansBold24pt7b);
  tft.setTextColor(BLACK);
  if (v2 ==0){
  tft.setCursor(120,65);
  tft.print("Enjoy!");
  }
  else if (v2 == 1){
  tft.setCursor(85,65);
  tft.setFont(&FreeSansBold18pt7b);
  tft.print("There you go!");
  }

   tft.drawLine(20, 85, 380, 85, BLACK);
   tft.fillRoundRect(150, 100, 100, 100, 20, BLACK);
   tft.fillRoundRect(157, 107, 86, 86, 15, WHITE);
   tft.fillTriangle(200, 117, 170, 145, 230, 145, BLACK);
   tft.fillRoundRect(180, 138, 40, 40, 5, BLACK);
   homeTouch();
}
void homeTouch(){
    pinMode(XM, INPUT);
    pinMode(YP, INPUT);
   do {
    TSPoint p = ts.getPoint();
    xpos = map(p.x, pLEFT, pRIGHT, 0, 400);
    ypos = map(p.y, pTOP, pBOTTOM, 0, 240);

    if ( p.z > MINPRESSURE && p.z < MAXPRESSURE && xpos<250 && xpos>150 && ypos>100 && ypos<200 ){
    highlightHome();

    }
   } 
    while (waitUntil_4<2);
}

void highlightHome(){
   waitUntil_4=3;
   pinMode(XM, OUTPUT);
   pinMode(YP, OUTPUT);
   tft.fillRoundRect(150, 100, 100, 100, 20,GREEN);
   tft.fillRoundRect(157, 107, 86, 86, 15, WHITE);
   tft.fillTriangle(200, 117, 170, 145, 230, 145, GREEN);
   tft.fillRoundRect(180, 135, 40, 43, 10, GREEN);
   delay(100);
   homeScreen();
  
}
void loop(){
  if (startScan==1){
    getFingerprintIDez();
    delay(50);
  }
}
