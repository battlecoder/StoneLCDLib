
// ************************************************
// StoneLCDLib.cpp                               **
// ***************************************************************************
/* Implementation of StoneLCDLib; a library to control Stone HMI Displays.
 *
 * Author: Elias Zacarias
 * URL: https://github.com/battlecoder/StoneLCDLib
*/

#include "StoneLCDLib.h"

/*############################################################################
 *##                                                                        ##
 *##                            M A C R O S                                 ##
 *##                                                                        ##
 *############################################################################*/
#define tryOrReturnFalse(f)         if(!(f)) return false
#define minVal(v, minV)             ((v) < (minV) ? (minV) : (v))
#define maxVal(v, maxV)             ((v) > (maxV) ? (maxV) : (v))
#define constraint(v, minV, maxV)   minVal(v, maxVal(v, maxV))
#define wordFromBytes(h,l)          ((h<<8) | (l))

/*############################################################################
 *##                                                                        ##
 *##                       A U X   F U N C T I O N S                        ##
 *##                                                                        ##
 *############################################################################*/
uint8_t BCDEncode (uint8_t v){
  uint8_t decHi = v / 10;
  uint8_t decLo = v - (10*decHi);
  return ((decHi & 0xf) << 4) | (decLo & 0xf);
}

uint8_t BCDDecode (uint8_t v){
  uint8_t hsn = v >> 4;
  uint8_t lsn = v & 0xf;
  return lsn + ((hsn<<3) + hsn + hsn); // lsn + 10*hsn . We are doing the " x10" multiplication by fast shifted multiplication x8 + 2 times the value.
}


/*############################################################################
 *##                                                                        ##
 *##                     S t o n e L C D D a t e T i m e                    ##
 *##                                                                        ##
 *############################################################################*/

// ****************************************************
// ** Contructors
// ****************************************************
StoneLCDDateTime::StoneLCDDateTime(uint16_t y, uint8_t m, uint8_t d) {
  this->setYear(y);
  this->setMonth(m);
  this->setDay(d);
  this->setWeek(0); // Automatic
}

StoneLCDDateTime::StoneLCDDateTime(uint16_t y, uint8_t m, uint8_t d, uint8_t w, uint8_t h, uint8_t mm, uint8_t ss) {
  this->setYear(y);
  this->setMonth(m);
  this->setWeek(w);
  this->setDay(d);
  this->setHour(h);
  this->setMinutes(mm);
  this->setSeconds(ss);
}

StoneLCDDateTime::StoneLCDDateTime() {
  StoneLCDDateTime (2000, 1, 1, 0, 0, 0, 0);
}

// ****************************************************
// ** Getters
// ****************************************************
uint16_t StoneLCDDateTime::getYear(){
  return 2000 + this->year;
}

uint8_t StoneLCDDateTime::getMonth() {
  return this->month;
}

uint8_t StoneLCDDateTime::getDay(){
  return this->day;  
}

uint8_t StoneLCDDateTime::getWeek(){
  return this->week;
}

uint8_t StoneLCDDateTime::getHour() {
  return this->hour;
}

uint8_t StoneLCDDateTime::getMinutes() {
  return this->minutes;
}

uint8_t StoneLCDDateTime::getSeconds() {
  return this->seconds;
}

// ****************************************************
// ** Setters
// ****************************************************
void StoneLCDDateTime::setYear(uint16_t y){
  this->year = constraint(y-2000, 0, 99);
}

void StoneLCDDateTime::setMonth(uint8_t m){
  this->month = constraint (m, 1, 12);
}

void StoneLCDDateTime::setDay(uint8_t d){
  this->day = constraint (d, 1, 31);
}

void StoneLCDDateTime::setWeek(uint8_t w){
  this->week = w; // TO-DO: Check constraints
}

void StoneLCDDateTime::setHour(uint8_t h){
  this->hour = constraint (h, 0, 23);
}

void StoneLCDDateTime::setMinutes(uint8_t m){
  this->minutes = constraint (m, 0, 59);
}

void StoneLCDDateTime::setSeconds(uint8_t s){
  this->seconds = constraint (s, 0, 59);
}

// ****************************************************
// ** Methods
// ****************************************************
uint8_t StoneLCDDateTime::getBCD(uint8_t *destBuffer) {
  destBuffer[0] = BCDEncode(this->year);
  destBuffer[1] = BCDEncode(this->month);
  destBuffer[2] = BCDEncode(this->day);
  destBuffer[3] = BCDEncode(this->week);
  destBuffer[4] = BCDEncode(this->hour);
  destBuffer[5] = BCDEncode(this->minutes);
  destBuffer[6] = BCDEncode(this->seconds);
  return STONE_DATETIME_BDC_BUFFER_SIZE;
}

void StoneLCDDateTime::setFromBCDBuffer(uint8_t *srcBuffer){
  this->setYear(2000 + BCDDecode(srcBuffer[0]));
  this->setMonth(BCDDecode(srcBuffer[1]));
  this->setDay(BCDDecode(srcBuffer[2]));
  this->setWeek(BCDDecode(srcBuffer[3]));
  this->setHour(BCDDecode(srcBuffer[4]));
  this->setMinutes(BCDDecode(srcBuffer[5]));
  this->setSeconds(BCDDecode(srcBuffer[6]));
}

/*############################################################################
 *##                                                                        ##
 *##                            S t o n e L C D                             ##
 *##                                                                        ##
 *############################################################################*/
void  StoneLCD::clearInputStream(){
  while (this->ioBytesAvailable() > 0) this->readIOStream();
}


// ****************************************************
// ** Constructor
// ****************************************************
StoneLCD::StoneLCD (Stream *ioPort, uint8_t cmdHi, uint8_t cmdLo) {
	this->interface = ioPort;
	this->cmdFrameLSB = cmdLo;
	this->cmdFrameHSB = cmdHi;
}

// ****************************************************
// ** Private Methods
// ****************************************************
uint8_t StoneLCD::ioBytesAvailable() {
  if (this->interface == NULL) return 0;
  return this->interface->available();
}

uint8_t StoneLCD::readIOStream(){
  if (this->interface == NULL) return 0;
  return this->interface->read();
}

uint8_t StoneLCD::waitAndReadIOStream(){
  long startTime = millis();
  while (millis() - startTime < this->timeOutMs) {
    if (this->ioBytesAvailable() > 0) return this->readIOStream();
  }
  return 0;
}

// Pending: CRC
boolean StoneLCD::sendCmdFrameStart (uint8_t cmd, uint8_t len){
  if (this->interface != NULL){
    this->interface->write(this->cmdFrameHSB);
    this->interface->write(this->cmdFrameLSB);
    this->interface->write(len);
    this->interface->write(cmd);
    return true;
  }
  return false;
}

boolean StoneLCD::sendByte (uint8_t b){
  if (this->interface != NULL) {
    this->interface->write(b);
    return true;
  }
  return false;
}

boolean StoneLCD::sendWord (uint16_t w){
  if (this->interface != NULL) {
    this->interface->write((uint8_t)(w>>8));
    this->interface->write((uint8_t)(w&0xff));
    return true;
  }
  return false;
}

boolean StoneLCD::sendBuffer (uint8_t *b, byte bufflen){
  if (this->interface != NULL) {
    this->interface->write(b, bufflen);
    return true;
  }
  return false;
}

// ****************************************************
// ** Setters
// ****************************************************
void StoneLCD::setTimeoutMs(long timeout){
	this->timeOutMs = timeout;
}

// ****************************************************
// ** Getters
// ****************************************************
long StoneLCD::getTimeoutMs(){
	return this->timeOutMs;
}

// ****************************************************
// ** "Register" Methods
// ****************************************************
boolean StoneLCD::writeRegister(uint8_t regStartAddr, uint8_t *buffer, uint8_t buffLen){
  // Header
	tryOrReturnFalse (this->sendCmdFrameStart(STONE_CMD_REGISTER_WRITE, 2 + buffLen)); // cmd (1) + address (1) + data size (buffLen)
  // Address
  tryOrReturnFalse (this->sendByte(regStartAddr));
  // Data
  tryOrReturnFalse (this->sendBuffer((uint8_t *)buffer, buffLen));
  // TO-DO: CRC
  return true;
}

boolean StoneLCD::writeRegisterByte(uint8_t regStartAddr, uint8_t b){
  uint8_t buffer[1];
  buffer[0] = b;
  return this->writeRegister(regStartAddr, buffer, 1);
}

boolean StoneLCD::writeRegisterWord(uint8_t regStartAddr, uint16_t w){
  uint8_t buffer[2];
  buffer[0] = (uint8_t)(w >> 8);
  buffer[1] = (uint8_t)(w  & 0xff);
  return this->writeRegister(regStartAddr, buffer, 2);
}

boolean StoneLCD::readRegister(uint8_t regStartAddr, void *dest_buffer, uint8_t buffLen) {
  byte r;

  this->clearInputStream();
  // Header
  tryOrReturnFalse (this->sendCmdFrameStart(STONE_CMD_REGISTER_READ, 3)); // cmd (1) + address (1) + bytes to read (1 byte)
  // Address
  tryOrReturnFalse (this->sendByte(regStartAddr));
  // Bytes to read
  tryOrReturnFalse (this->sendByte(buffLen));

  // Wait for data to be available and expect a valid response
  tryOrReturnFalse (this->waitAndReadIOStream() == this->cmdFrameHSB);
  tryOrReturnFalse (this->waitAndReadIOStream() == this->cmdFrameLSB);
  tryOrReturnFalse (this->waitAndReadIOStream() == buffLen + 3); // cmd (1) + address (1) + requested bytes (1)
  tryOrReturnFalse (this->waitAndReadIOStream() == STONE_CMD_REGISTER_READ);
  tryOrReturnFalse (this->waitAndReadIOStream() == regStartAddr);
  tryOrReturnFalse (this->waitAndReadIOStream() == buffLen);
  for (r = 0; r < buffLen; r++) {
    ((uint8_t *)dest_buffer)[r] = this->waitAndReadIOStream();
  }
  return true;
}

uint8_t StoneLCD::readRegisterByte(uint8_t regStartAddr) {
  byte b;
  boolean readOk = this->readRegister(regStartAddr, &b, 1);
  return readOk ? b : 0;
}

uint16_t StoneLCD::readRegisterWord(uint8_t regStartAddr) {
  uint8_t buffer[2];
  boolean readOk = this->readRegister(regStartAddr, buffer, 2);

  return readOk ? wordFromBytes(buffer[0], buffer[1]) : 0;
}

// ****************************************************
// ** "Variable" Methods
// ****************************************************
boolean StoneLCD::writeVariable(uint16_t varStartAddr, uint16_t *buffer, uint8_t buffLen){
  uint8_t buffSizeInBytes = buffLen<<1;

  // Header
  tryOrReturnFalse (this->sendCmdFrameStart(STONE_CMD_VARIABLE_WRITE, 3 + buffSizeInBytes)); // cmd (1) + address (2) + data size (buffLen*2)
  // Address
  tryOrReturnFalse (this->sendWord(varStartAddr));
  // Data (word-based write)
  tryOrReturnFalse (this->sendBuffer((uint8_t *)buffer, buffSizeInBytes));
  // TO-DO: CRC
  return true;
}

boolean StoneLCD::writeVariableWord(uint16_t varStartAddr, uint16_t w){
  uint8_t buffer[2];
  buffer[0] = (uint8_t)(w >> 8);
  buffer[1] = (uint8_t)(w  & 0xff);
  return this->writeVariable(varStartAddr, (uint16_t *)buffer, 1);
}

boolean StoneLCD::readVariable(uint16_t varStartAddr, uint16_t *dest_buffer, uint8_t buffLen){
  uint8_t r, h, l;
  uint8_t buffSizeInBytes = buffLen<<1;
  
  this->clearInputStream();
  // Header
  tryOrReturnFalse (this->sendCmdFrameStart(STONE_CMD_VARIABLE_READ, 4)); // cmd (1) + address (2) + words to read (1 byte)
  // Address
  tryOrReturnFalse (this->sendWord(varStartAddr));
  // Bytes to read
  tryOrReturnFalse (this->sendByte(buffLen));
  // Wait for data to be available and expect a valid response
  tryOrReturnFalse (this->waitAndReadIOStream() == this->cmdFrameHSB);
  tryOrReturnFalse (this->waitAndReadIOStream() == this->cmdFrameLSB);
  tryOrReturnFalse (this->waitAndReadIOStream() == buffSizeInBytes + 4); // cmd (1) + address (2) + requested bytes (1)
  tryOrReturnFalse (this->waitAndReadIOStream() == STONE_CMD_VARIABLE_READ);
  tryOrReturnFalse (this->waitAndReadIOStream() == (varStartAddr >> 8));
  tryOrReturnFalse (this->waitAndReadIOStream() == (varStartAddr & 0xff));
  tryOrReturnFalse (this->waitAndReadIOStream() == buffLen);
  for (r = 0; r < buffLen; r++) {
    h = this->waitAndReadIOStream();
    l = this->waitAndReadIOStream();
    dest_buffer[r] = wordFromBytes(h, l);
  }
  return true;
}

uint16_t StoneLCD::readVariableWord(uint16_t varStartAddr) {
  uint16_t w;
  boolean readOk = this->readVariable(varStartAddr, &w, 1);

  return readOk ? w : 0;
}

// ****************************************************
// ** "RTC" Methods
// ****************************************************
boolean StoneLCD::getRTC(StoneLCDDateTime *dst) {
  byte dateBuffer[STONE_DATETIME_BDC_BUFFER_SIZE];

  if (dst == NULL) return false;
  if (this->readRegister(STONE_REG_RTC_NOW, dateBuffer, STONE_DATETIME_BDC_BUFFER_SIZE) == false) return false;
  dst->setFromBCDBuffer(dateBuffer);
  return true;
}

boolean StoneLCD::setRTC(StoneLCDDateTime *src){
  byte dateBuffer[STONE_DATETIME_BDC_BUFFER_SIZE+1];

  if (src == NULL) return false;

  dateBuffer[0] = 0x5A; // Used to set the date
  src->getBCD(&dateBuffer[1]);
  return this->writeRegister(STONE_REG_RTC_COM_ADJ, dateBuffer, STONE_DATETIME_BDC_BUFFER_SIZE+1);
}

// ****************************************************
// ** Public I/O Methods
// ****************************************************
boolean StoneLCD::checkForIOEvent(StoneLCDEvent *dst, uint16_t *dataDest, uint8_t maxLen){
  int i;
  uint8_t len = 0;
  uint8_t h, l;
  if (dst == NULL || dataDest == NULL || this->interface == NULL) return false;

  dst->cmd = 0;
  dst->dataLen = 0;
  dst->address = 0;
if (this->interface->available()){
    // Try to identify the beginning of a command frame and then start parsing the rest
    if (this->interface->read() == this->cmdFrameHSB){
      tryOrReturnFalse (this->waitAndReadIOStream() == this->cmdFrameLSB);
      len = this->waitAndReadIOStream();
      dst->cmd = this->waitAndReadIOStream();
      h = this->waitAndReadIOStream();
      l = this->waitAndReadIOStream();
      dst->address = wordFromBytes(h, l);
      dst->dataLen = this->waitAndReadIOStream();
      
      for (i = 0; i < dst->dataLen; i++) {
        h = this->waitAndReadIOStream();
        l = this->waitAndReadIOStream();
        if (i < maxLen) dataDest[i] = wordFromBytes(h, l);
      }
      // Just as a sanity check, len should have been 4 + len*2
      return ( len == (4 + (dst->dataLen<<1)) );
    }
  }
  return false;
}
