// Seeed Wio Terminal
#ifndef WIOTERM_H
#define WIOTERM_H

SdFat SD;
// The Wio Terminal uses SPI1. CS pin is SDCARD_SS_PIN
#define SDMHZ 50
#define SDINIT SDCARD_SS_PIN, SD_SCK_MHZ(SDMHZ)
#define LED LED_BUILTIN
#define LEDinv 0
#define BOARD "Seeed Wio Terminal"
#define board_wioterm
#define board_analog_io
#define board_digital_io

#define HW_KEY
#define HW_KEY_A WIO_KEY_A
#define HW_KEY_B WIO_KEY_B
#define HW_KEY_C WIO_KEY_C

#define HW_5S
#define HW_5S_UP    WIO_5S_UP
#define HW_5S_LEFT  WIO_5S_LEFT
#define HW_5S_RIGHT WIO_5S_RIGHT
#define HW_5S_DOWN  WIO_5S_DOWN
#define HW_5S_PRESS WIO_5S_PRESS

#define HW_BUZZER   WIO_BUZZER  

#define USE_RTC

// ジョイスティック用
struct TJ2K {
  uint8_t BtnA : 1;   // 1
  uint8_t BtnB : 1;   // 2
  uint8_t BtnC : 1;   // 3
  uint8_t Up : 1;     // 4
  uint8_t Left : 1;   // 5
  uint8_t Right : 1;  // 6
  uint8_t Down : 1;   // 7
  uint8_t Press : 1;  // 8
};

union J2K {
  uint8_t value;
  struct TJ2K Bits;
};

PROGMEM const uint8_t defaultj2k = 0b11111111;

QueueHandle_t xQueue;
RTC_SAMD51 rtc;
DateTime now;
union J2K j2k = {defaultj2k};

uint16 wiobdos(uint16 dmaaddr) {

  uint16 HL = 0x0000;
  uint16 i = WORD16(dmaaddr);
  uint8 func = _RamRead(i);
  uint8 param;
  ++i;

  switch (func) {
    case 2: // 2 (02h): get Joy to Key
      HL = j2k.value;
      return(HL);
      break;
    case 3: // 3 (03h): set Joy to Key
      j2k.value = _RamRead(i);
      return(HL);
      break;
    case 42: // 42 (2Ah): get date
      param = _RamRead(i);
      ++i;
      switch (param) {
        case 1:  // 1: get Month and Day
          HL = (now.month() << 8) | now.day();
          break;
        case 2:  // 2: get Day of the Week
          HL = now.dayOfTheWeek();
          break;
        default: // 0: get Year
          HL = now.year();
          break;
      }
      return(HL);
      break;
    case 43: // 43 (2Bh): set date
      {
        param = _RamRead(i);
        ++i;
        uint16_t v = _RamRead(i) << 8 | _RamRead(++i);
        switch (param) {
          case 1:  // 1: set Month and Day
            now = DateTime(now.year(), HIGH_REGISTER(v), LOW_REGISTER(v),
                           now.hour(), now.minute(), now.second());
            break;
          default: // 0: set Year
            now = DateTime(v, now.month(), now.day(),
                           now.hour(), now.minute(), now.second());
            break;
        }
        rtc.adjust(now);
      }
      return(HL);
      break;
    case 44: // 44 (2Ch): get time
      param = _RamRead(i);
      ++i;
      switch (param) {
        case 1:  // 1: get Second
          HL = (now.second() << 8);
          break;
        default: // 0: get Hour and Minute
          HL = (now.hour() << 8) | now.minute();
          break;
      }
      return(HL);
      break;
    case 45: // 45 (2Dh): set time
      {
        param = _RamRead(i);
        ++i;
        uint16_t v = _RamRead(i) << 8 | _RamRead(++i);
        switch (param) {
          case 1:  // 1: set Second
            now = DateTime(now.year(), now.month(), now.day(),
                           now.hour(), now.minute(), HIGH_REGISTER(v));
            break;
          default: // 0: Hour and Minute
            now = DateTime(now.year(), now.month(), now.day(),
                           HIGH_REGISTER(v), LOW_REGISTER(v), now.second());
            break;
        }
        rtc.adjust(now);
      }
      return(HL);
      break;
    default:
      return (HL);
  }
}

#endif
