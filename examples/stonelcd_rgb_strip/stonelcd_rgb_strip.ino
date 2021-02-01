#include <StoneLCDLib.h>
#include <Adafruit_NeoPixel.h>

 /************************************************/
 /* stone_rgb_demo.ino                           */
 /*****************************************************************************/
 /* Intended to be the Arduino equivalent of the STM32-based RGB LED strip
  *  Controller demo that comes with Stone LCD Screens, but using the StoneLCD
  *  Arduino Library.
  *  
  *
  * NOTE: THIS PROJECT REQUIRES THE ADAFRUIT NEOPIXEL LIBRARY.
  *  Install from the library manager.
  * 
  * ***********************************************
  * Steps
  * ***********************************************
  * 1. Reduce the baud rate to 9600 in the included RGB Strip Demo Project.
  * 2. Replace the STM32 with an Arduino Uno/Nano/Pro Mini/ etc
  * 
  *  ARDUINO      MODULE                PIN
  *  -----------------------------------------------
  *  0 (RX)       TTL-RS232 Converter   RX <-
  *  1 (TX)       TTL-RS232 Converter   TX ->
  *  9            RGB Strip             DI
  *  
  *  (Check "pin assignment" section to change).
  */
 
/*############################################################################
 *##                                                                        ##
 *##                               D E F I N E S                            ##
 *##                                                                        ##
 *############################################################################*/
 /* Pin assignment *******************************/
#define LED_PIN               9
#define LCD_RX                4
#define LCD_TX                5

 /* Buttons and Labels IDs ***********************/
#define BTTN_ONOFF            0x0001
#define ICON_WHITE            0x0002
#define ICON_RED              0x0003
#define ICON_GREEN            0x0004
#define ICON_BLUE             0x0005

#define TEXT_WHITE            0x0006
#define TEXT_RED              0x0007
#define TEXT_GREEN            0x0008
#define TEXT_BLUE             0x0009

#define BTTN_BLINK1           0x000A
#define BTTN_BLINK2           0x000B
#define BTTN_BLINK3           0x000C
#define BTTN_BLINK4           0x000D
#define NUM_OF_LEDS           0x0033

#define ICON_OFF              0
#define ICON_ON               1

/* Other constants ******************************/
#define MAX_RECV_BUFFER       4
#define MAX_LED_COUNT         27

/*############################################################################
 *##                                                                        ##
 *##                                G L O B A L                             ##
 *##                                                                        ##
 *############################################################################*/
struct {
  byte white;
  byte red;
  byte green;
  byte blue;
}rgbStatus;

StoneLCD          myLCD (&Serial);  // Can use a soft-serial port if care is taken
                                    //  to constantly scan for incoming messages.
Adafruit_NeoPixel strip(MAX_LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

StoneLCDEvent evt;
uint16_t      recvBuffer[MAX_RECV_BUFFER];
uint8_t       led_count = MAX_LED_COUNT;

// The following variables are used by the blinking functions, which were copied
// directly from the example provided by STONE. 
uint8_t       blink_type = 0;
uint8_t       times = 0;
uint8_t       BLINK_2 = 0;
uint8_t       USER_R = 0, USER_G = 0, USER_B = 0, COLOR_TYPE = 0, COLOR_DIR = 0;
uint16_t      k, q;

/*############################################################################
 *##                                                                        ##
 *##                      U I   H A N D L I N G                             ##
 *##                                                                        ##
 *############################################################################*/
void updateWRGBIcons(){
  myLCD.writeVariableWord(ICON_WHITE,   (rgbStatus.white > 0));
  myLCD.writeVariableWord(ICON_RED,     (rgbStatus.red > 0));
  myLCD.writeVariableWord(ICON_GREEN,   (rgbStatus.green > 0));
  myLCD.writeVariableWord(ICON_BLUE,    (rgbStatus.blue > 0));
}

void updateUIFromrgbStatus(){
  myLCD.writeVariableWord(TEXT_WHITE,   rgbStatus.white);
  myLCD.writeVariableWord(TEXT_RED,     rgbStatus.red);
  myLCD.writeVariableWord(TEXT_GREEN,   rgbStatus.green);
  myLCD.writeVariableWord(TEXT_BLUE,    rgbStatus.blue);

  updateWRGBIcons();
}

void updatergbStatusFromUI() {
  rgbStatus.white     = myLCD.readVariableWord(TEXT_WHITE);
  rgbStatus.red       = myLCD.readVariableWord(TEXT_RED);
  rgbStatus.green     = myLCD.readVariableWord(TEXT_GREEN);
  rgbStatus.blue      = myLCD.readVariableWord(TEXT_BLUE);
}

void checkForSwitchAutoOff(){
  if ((rgbStatus.red == 0) && (rgbStatus.green == 0) && (rgbStatus.blue == 0) && (rgbStatus.white == 0) && (blink_type == 0)){
    myLCD.writeVariableWord(BTTN_ONOFF, ICON_OFF);
  }else{
    myLCD.writeVariableWord(BTTN_ONOFF, ICON_ON);
  }
}
/*############################################################################
 *##                                                                        ##
 *##                          R G B   S T R I P                             ##
 *##                                                                        ##
 *############################################################################*/
void setFullRGBStrip(byte r, byte g, byte b){
  byte i;
  for (i=0; i < led_count; i++) strip.setPixelColor(i, r, g, b);
  for (i=led_count; i < MAX_LED_COUNT; i++) strip.setPixelColor(i, 0, 0, 0);
  strip.show();
}

void updateRGBStrip(){
  byte r, g, b;

  if (rgbStatus.white == 0){
    r = rgbStatus.red;
    g = rgbStatus.green;
    b = rgbStatus.blue;
  }else{
    r = rgbStatus.white;
    g = rgbStatus.white;
    b = rgbStatus.white;
  }
  setFullRGBStrip(r, g, b);
}

/*############################################################################
 *##                                                                        ##
 *##                       C O N T R O L   L O G I C                        ##
 *##                                                                        ##
 *############################################################################*/
 void allRGBOff(){
  rgbStatus.white = 0;
  rgbStatus.red   = 0;
  rgbStatus.green = 0;
  rgbStatus.blue  = 0;
}

void blink1(){
  times++;
  if (times >= 14) {
    times = 0;
    if (COLOR_DIR == 0) {
      if (COLOR_TYPE == 0) {
        USER_R++;
        USER_G = 0;
        USER_B = 0;
      } else if (COLOR_TYPE == 1) {
        USER_R = 0;
        USER_G++;
        USER_B = 0;
      } else if (COLOR_TYPE == 2) {
        USER_R = 0;
        USER_G = 0;
        USER_B++;
      } else if (COLOR_TYPE == 3) {
        USER_R++;
        USER_G++;
        USER_B = 0;
      } else if (COLOR_TYPE == 4) {
        USER_R = 0;
        USER_G++;
        USER_B++;
      } else if (COLOR_TYPE == 5) {
        USER_R++;
        USER_G = 0;
        USER_B++;
      }
      if ((USER_R >= 250) || (USER_G >= 250) || (USER_B >= 250)) {
        COLOR_DIR = 1;
      }
    } else {
      if (COLOR_TYPE == 0) {
        USER_R--;
        USER_G = 0;
        USER_B = 0;
      } else if (COLOR_TYPE == 1) {
        USER_R = 0;
        USER_G--;
        USER_B = 0;
      } else if (COLOR_TYPE == 2) {
        USER_R = 0;
        USER_G = 0;
        USER_B--;
      } else if (COLOR_TYPE == 3) {
        USER_R--;
        USER_G--;
        USER_B = 0;
      } else if (COLOR_TYPE == 4) {
        USER_R = 0;
        USER_G--;
        USER_B--;
      } else if (COLOR_TYPE == 5) {
        USER_R--;
        USER_G = 0;
        USER_B--;
      }
      if ((USER_R == 0x02) || (USER_G == 0x02) || (USER_B == 0x02)) {
        COLOR_DIR = 0;
        COLOR_TYPE++;
        if (COLOR_TYPE > 5) COLOR_TYPE = 0;
      }
    }
    setFullRGBStrip (USER_R, USER_G, USER_B);
  }
  delay(1);
}

void blink2(){
  k++;
  if (k >= 150){
    k = 0;
    q = 200;

    BLINK_2++;
    if (BLINK_2 > 8) BLINK_2 = 0;
    
    if (BLINK_2 == 0)       setFullRGBStrip(q, 0, 0);
    else if (BLINK_2 == 1)  setFullRGBStrip(0, q, 0);
    else if (BLINK_2 == 2)  setFullRGBStrip(0, 0, q);
    else if (BLINK_2 == 3)  setFullRGBStrip(q, q, 0);
    else if (BLINK_2 == 4)  setFullRGBStrip(0, q, q);
    else if (BLINK_2 == 5)  setFullRGBStrip(q, 0, q);
    else if (BLINK_2 == 6)  setFullRGBStrip(q - 100, q, 0);
    else if (BLINK_2 == 7)  setFullRGBStrip(0, q - 80, q);
    else if (BLINK_2 == 8)  setFullRGBStrip(q, 0, q - 120);
  }
  delay(1);
}

void blink3(){
  k++;
  if (k >= 1000) {
    k = 0;

    BLINK_2++;
    if (BLINK_2 > 5) BLINK_2 = 0;

    if (BLINK_2 == 0)      setFullRGBStrip(q, 0, 0);
    else if (BLINK_2 == 1) setFullRGBStrip(0, q, 0);
    else if (BLINK_2 == 2) setFullRGBStrip(0, 0, q);
    else if (BLINK_2 == 3) setFullRGBStrip(q, q, 0);
    else if (BLINK_2 == 4) setFullRGBStrip(0, q, q);
    else if (BLINK_2 == 5) setFullRGBStrip(q, 0, q);
  }
  delay(1);
}

void blink4(){
  k++;
  if (k >= 500) {
    k = 0;
    q = 0;
    BLINK_2++;
    if (BLINK_2 > 5) BLINK_2 = 0;
  }
  q++;
  if (q >= 250) q = 0;

  if (BLINK_2 == 0)      setFullRGBStrip(q, 0, 0);
  else if (BLINK_2 == 1) setFullRGBStrip(0, q, 0);
  else if (BLINK_2 == 2) setFullRGBStrip(0, 0, q);
  else if (BLINK_2 == 3) setFullRGBStrip(q, q, 0);
  else if (BLINK_2 == 4) setFullRGBStrip(0, q, q);
  else if (BLINK_2 == 5) setFullRGBStrip(q, 0, q);
  delay(1);
}

/*############################################################################
 *##                                                                        ##
 *##                    M I S C    F U N C T I O N S                        ##
 *##                                                                        ##
 *############################################################################*/
void setDateTime(){
  StoneLCDDateTime dateTime(2020, 12, 31, 0, 23, 59, 58);
  myLCD.setRTC(&dateTime);
}

/*############################################################################
 *##                                                                        ##
 *##                                 S E T U P                              ##
 *##                                                                        ##
 *############################################################################*/
void setup() {
  // *** Initialize RGB strip
  strip.begin();

  // *** Initialize serial port
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // *** Wait a bit before starting, and then set the time,
  delay(1000);
  // Comment the following line if you don't want the time/date of your
  // LCD to be overwritten
  setDateTime();

  // Read UI values into our app data
  updatergbStatusFromUI();

  // Also read initial LED count
  led_count = myLCD.readVariableWord(NUM_OF_LEDS);

  // Let's start the loop() cycle discarding pending bytes from the screen,
  // if any.
  myLCD.clearInputStream();
}

/*############################################################################
 *##                                                                        ##
 *##                                  L O O P                               ##
 *##                                                                        ##
 *############################################################################*/
void loop() {
  byte i;
  byte value;
  boolean recvEvent;

  recvEvent = myLCD.checkForIOEvent(&evt, recvBuffer, MAX_RECV_BUFFER);
  if (recvEvent){
    // Let's clear any remaining of pending messages
    myLCD.clearInputStream();

    // All values the UI handles for this example are bytes, so let's get the
    // first byte in the message content, and use it as the value from now on
    value = recvBuffer[0];

    switch (evt.address){
      case NUM_OF_LEDS:
        led_count = value;
        updateRGBStrip();
        break;

      case BTTN_ONOFF:
        blink_type = 0;
        if (value == 0){
          allRGBOff();
        } else {
          rgbStatus.white = 0;
          rgbStatus.red   = 0x24;
          rgbStatus.green = 0x10;
          rgbStatus.blue  = 0x32;
        }
        updateRGBStrip();
        break;

      case TEXT_WHITE:
        allRGBOff();
        blink_type      = 0;
        rgbStatus.white = value;
        updateRGBStrip();
        break;

      case TEXT_RED:
        blink_type      = 0;
        rgbStatus.red   = value;
        rgbStatus.white = 0;
        updateRGBStrip();
        break;

      case TEXT_GREEN:
        blink_type      = 0;
        rgbStatus.green = value;
        rgbStatus.white = 0;
        updateRGBStrip();
        break;

      case TEXT_BLUE:
        blink_type      = 0;
        rgbStatus.blue  = value;
        rgbStatus.white = 0;
        updateRGBStrip();
        break;

      case BTTN_BLINK1:
        blink_type = 1;
        allRGBOff();
        break;

      case BTTN_BLINK2:
        blink_type = 2;
        allRGBOff();
        break;

      case BTTN_BLINK3:
        blink_type = 3;
        allRGBOff();
        break;

      case BTTN_BLINK4:
        blink_type = 4;
        allRGBOff();
        break;
    }
    checkForSwitchAutoOff();
    updateUIFromrgbStatus();
  } else {
    // No event received from the UI,
    // process / animate current blinking seq.
    if (blink_type == 1)      blink1();
    else if (blink_type == 2) blink2();
    else if (blink_type == 3) blink3();
    else if (blink_type == 4) blink4();
  }
}
