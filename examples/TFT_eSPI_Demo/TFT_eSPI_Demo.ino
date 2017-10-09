#include "FS.h"
#include <SPI.h>
#include <TFT_eSPI.h>
#include <SvgParser.h>
#include "SvgOutput_TFT_eSPI.h"

TFT_eSPI tft = TFT_eSPI();
SvgOutput_TFT_eSPI svgOutput = SvgOutput_TFT_eSPI();
SvgParser svg = SvgParser(&svgOutput);


#define CALIBRATION_FILE "/calibrationData"


int counter = 42;

char * increment(int argc, char* argv[]) {
  counter++;
  return NULL;
}
char * decrement(int argc, char* argv[]) {
  counter--;
  return NULL;
}

char * printCounter(int argc, char* argv[]) {
  char *ptr = (char *)malloc(10);
  if (ptr == NULL) return NULL;
  sprintf(ptr, "%i", counter);
  return ptr;
}

char * printTime(int argc, char* argv[]) {
  char *ptr = (char *)malloc(10);
  if (ptr == NULL) return NULL;
  sprintf(ptr, "%i", millis());
  return ptr;
}


void setup(void) {
  uint16_t calibrationData[5];
  uint8_t calDataOK = 0;

  Serial.begin(115200);
  Serial.println("starting");

  tft.init();

  tft.setRotation(2);
  tft.fillScreen((0xFFFF));

  tft.setCursor(20, 0, 2);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);  tft.setTextSize(1);
  tft.println("calibration run");

  // check file system
  if (!SPIFFS.begin()) {
    Serial.println("formating file system");

    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    File f = SPIFFS.open(CALIBRATION_FILE, "r");
    if (f) {
      if (f.readBytes((char *)calibrationData, 14) == 14)
        calDataOK = 1;
      f.close();
    }
  }
  if (calDataOK) {
    // calibration data valid
    calibrationData[4] ^= 0x06;
    tft.setTouch(calibrationData);
  } else {
    // data not valid. recalibrate
    tft.calibrateTouch(calibrationData, TFT_WHITE, TFT_RED, 15);
    calibrationData[4] ^= 0x06;
    tft.setTouch(calibrationData);
    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calibrationData, 14);
      f.close();
    }
  }

  tft.fillScreen((0xFFFF));

  svg.addCallback("time", printTime);
  svg.addCallback("nr", printCounter);
  svg.addCallback("dec", decrement);
  svg.addCallback("inc", increment);

  svg.readFile((char *)"/index.svg");

  svg.print();
  // list links
  svg.linkManagement();
}



void loop() {
  uint16_t x, y;
  static uint16_t color;
  char * link;
  static bool clicked = false;

  if (tft.getTouch(&x, &y)) {
    if (svg.onClick(x, y, &link)) {
      Serial.printf("pressed: #%s#\n", link);
      free(link);
    }
  } 
}



