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

QueueHandle_t xQueue;
RTC_SAMD51 rtc;
DateTime now;

uint16 wiobdos(uint16 dmaaddr) {

  uint16 HL = 0x0000;
  uint16 i = WORD16(dmaaddr);
  uint8 func = _RamRead(i);
  uint8 param;
  ++i;

  switch (func) {
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
