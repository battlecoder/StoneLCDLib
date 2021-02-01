
// ************************************************
// StoneLCDLib.h                                 **
// ***************************************************************************
/* Header for StoneLCDLib; a library to control Stone HMI Displays.
 *
 * Author: Elias Zacarias
 * URL: https://github.com/battlecoder/StoneLCDLib
*/
  
#ifndef _STONE_LCD_LIB_H__
#define _STONE_LCD_LIB_H__

#include <Arduino.h>

/*############################################################################
 *##                                                                        ##
 *##                               D E F I N E S                            ##
 *##                                                                        ##
 *############################################################################*/
#define STONE_DATETIME_BDC_BUFFER_SIZE  7

// --- STONE CMD Constants -------------------------------------
#define STONE_CMD_REGISTER_WRITE        0x80
#define STONE_CMD_REGISTER_READ         0x81
#define STONE_CMD_VARIABLE_WRITE        0x82
#define STONE_CMD_VARIABLE_READ         0x83
#define STONE_CMD_CURVE_BUFFER_WRITE    0x84
#define STONE_CMD_EXTENSION_FUNCTION    0x84

// --- STONE REG Constants -------------------------------------
#define STONE_REG_VERSION               0x00
#define STONE_REG_LED_NOW               0x01
#define STONE_REG_BZ_TIME               0x02
#define STONE_REG_PIC_ID                0x03 // 2 bytes
#define STONE_REG_TP_FLAG               0x05 // 0x5A = Update
#define STONE_REG_TP_STATUS             0x06 // 0x01 = First press, 0x03 = Still pressed, 0x02 released
#define STONE_REG_TP_POSITION           0x07 // 4 bytes, XH:XL, YH:YL
#define STONE_REG_TPC_ENABLE            0x0B // 0x00 = Touch Control Disabled
#define STONE_REG_RUNTIME               0x0C // 4 bytes. HHHH,MM,SS in BCD encoding 
#define STONE_REG_R0                    0x10
#define STONE_REG_R1                    0x11
#define STONE_REG_R2                    0x12
#define STONE_REG_R3                    0x13
#define STONE_REG_R4                    0x14
#define STONE_REG_R5                    0x15
#define STONE_REG_R6                    0x16
#define STONE_REG_R7                    0x17
#define STONE_REG_R8                    0x18
#define STONE_REG_R9                    0x19
#define STONE_REG_RA                    0x1A
#define STONE_REG_RB                    0x1B
#define STONE_REG_RC                    0x1C
#define STONE_REG_CONFIG_EN             0x1D
#define STONE_REG_RTC_COM_ADJ           0x1F
#define STONE_REG_RTC_NOW               0x20 // 7 bytes, YY,MM,DD,WW,HH,MM,SS in BCD format
#define STONE_REG_EN_LIB_OP             0x40
#define STONE_REG_LIB_OP_MODE           0x41 // Fixed to 0xA0
#define STONE_REG_LIB_ID                0x42
#define STONE_REG_LIB_ADDRESS           0x43 // 3 bytes
#define STONE_REG_LIB_VP                0x46 // 2 bytes
#define STONE_REG_LIB_OP_LENGTH         0x48 // 2 bytes
#define STONE_REG_TIMER0                0x4A // 2 bytes
#define STONE_REG_TIMER1                0x4C
#define STONE_REG_TIMER2                0x4D
#define STONE_REG_TIMER3                0x4E
#define STONE_REG_KEY_CODE              0x4F
#define STONE_REG_MUSIC_SET             0x50 // 0x5B = Play, 0x5C = Stop
#define STONE_REG_MUSIC_NUM             0x51 // 2 bytes
#define STONE_REG_VOL_ADJ_EN            0x53 // 0x5A = Apply adjustment
#define STONE_REG_VOL                   0x54 // Range: 0x00 - 0x40
#define STONE_REG_VOL_STATUS            0x55 // 0x00 = Stop, 0x01 = Play
#define STONE_REG_EN_DBL_OP				0x56 // 0x5A = Apply DB operation
#define STONE_REG_DBL_OP_MODE			0x57 // 0x50 = write, 0xA0 = read
#define STONE_REG_DBL_ADDRESS			0x58 // 4 bytes
#define STONE_REG_DBL_VP                0x5C // 2 bytes
#define STONE_REG_DBL_OP_LENGTH         0x5E // 2 bytes
#define STONE_REG_PLAY_AVI_SET			0x60 // 0x5A = apply video operation.
#define STONE_REG_PLAY_AVI_TYPE		    0x61 // 0x00 = Single(LCD), 0x01 = Single(USB Flash), 0x02 = Seq(LCD), 0x03 = Seq(USB)?
#define STONE_REG_PLAY_POSITION_X       0x62 // 2 bytes
#define STONE_REG_PLAY_POSITION_Y       0x64 // 2 bytes
#define STONE_REG_PLAY_AVI_NUM          0x66 // 2 bytes. Only usef for single play.
#define STONE_REG_VOL_ADJ_EN			0x68 // 0x5A = Adjust volume
#define STONE_REG_VOL                   0x69 // Range: 0x00 - 0x3F (Default)
#define STONE_REG_PLAY_CONTROL          0x6A // 0x5A = Play / Pause
#define STONE_REG_PLAY_STOP             0x6B // 0x5A = Stop
#define STONE_REG_PLAY_NEXT				0x6C // 0x5A = Play next. Ends with single play.
#define STONE_REG_PLAY_STATUS			0x6D // 0x00 = IDLE, 0x01 = Playing, 0x02 = Paused
#define STONE_REG_SCAN_STATUS			0xE9 // Read: 0x00 = Touch screen not scanning. 0x01 = touch screen scanning?. Write 0x00 to force exit.
#define STONE_REG_TPCAL_TRIGGER		    0xEA // 0x5A = Start Touch Screen calibration.
#define STONE_REG_TRENDLINE_CLEAR		0xEB // 0x55 = Clear all 8 curve buffers, 0x56-0x5D = curve channel to clear (0-7)
#define STONE_REG_RESET_TRIGGER			0xEE // 2 bytes: Write 0x5AA5 to reset the screen.

/*############################################################################
 *##                                                                        ##
 *##                              S T R U C T S                             ##
 *##                                                                        ##
 *############################################################################*/
typedef struct {
  uint8_t cmd;
  uint16_t address;
  uint8_t dataLen;
} StoneLCDEvent;

/*############################################################################
 *##                                                                        ##
 *##                     S t o n e L C D D a t e T i m e                    ##
 *##                                                                        ##
 *############################################################################*/
class StoneLCDDateTime {
private:
  uint8_t year;
  uint8_t month;
  uint8_t day;
  uint8_t week;
  uint8_t hour;
  uint8_t minutes;
  uint8_t seconds;

public:
  StoneLCDDateTime();
  StoneLCDDateTime(uint16_t y, uint8_t m, uint8_t d);
  StoneLCDDateTime(uint16_t y, uint8_t m, uint8_t d, uint8_t w, uint8_t h, uint8_t mm, uint8_t ss);

  uint16_t getYear();
  uint8_t getMonth();
  uint8_t getDay();
  uint8_t getWeek();
  uint8_t getHour();
  uint8_t getMinutes();
  uint8_t getSeconds();
  
  void setYear(uint16_t y);
  void setMonth(uint8_t m);
  void setDay(uint8_t d);
  void setWeek(uint8_t w);
  void setHour(uint8_t h);
  void setMinutes(uint8_t m);
  void setSeconds(uint8_t s);

  uint8_t getBCD(uint8_t *destBuffer);
  void setFromBCDBuffer(uint8_t *srcBuffer);
};

/*############################################################################
 *##                                                                        ##
 *##                            S t o n e L C D                             ##
 *##                                                                        ##
 *############################################################################*/
// Pending: CRC
class StoneLCD {
private:
  uint8_t cmdFrameLSB, cmdFrameHSB;
  Stream *interface;
  long timeOutMs = 200;

  uint8_t  ioBytesAvailable();
  uint8_t  readIOStream();
  uint8_t  waitAndReadIOStream();

  boolean sendCmdFrameStart (uint8_t cmd, uint8_t len);
  boolean sendByte (uint8_t b);
  boolean sendWord (uint16_t w);
  boolean sendBuffer (uint8_t *b, byte bufflen);
  
public:
  StoneLCD (Stream *ioPort, uint8_t cmdHi = 0xA5, uint8_t cmdLo = 0x5A);

  void setTimeoutMs(long timeout);
  long getTimeoutMs();

  // Register functions **********
  boolean writeRegister(uint8_t regStartAddr, uint8_t *buffer, uint8_t buffLen);
  boolean writeRegisterByte(uint8_t regStartAddr, uint8_t b);
  boolean writeRegisterWord(uint8_t regStartAddr, uint16_t w);

  boolean readRegister(uint8_t regStartAddr, void *dest_buffer, uint8_t buffLen);
  uint8_t readRegisterByte(uint8_t regStartAddr);
  uint16_t readRegisterWord(uint8_t regStartAddr);

  // Variable functions **********
  boolean writeVariable(uint16_t varStartAddr, uint16_t *buffer, uint8_t buffLen);
  boolean writeVariableWord(uint16_t varStartAddr, uint16_t w);

  boolean readVariable(uint16_t varStartAddr, uint16_t *dest_buffer, uint8_t buffLen);
  uint16_t readVariableWord(uint16_t varStartAddr);

  // RTC functions ***************
  boolean getRTC(StoneLCDDateTime *dst);
  boolean setRTC(StoneLCDDateTime *src);

  // I/O Stream functions ********
  boolean checkForIOEvent(StoneLCDEvent *dst, uint16_t *dataDest, uint8_t maxLen);
  void    clearInputStream();
};
#endif
