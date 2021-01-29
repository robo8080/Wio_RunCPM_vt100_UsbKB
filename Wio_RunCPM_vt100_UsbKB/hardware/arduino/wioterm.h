#ifndef WIOTERM_H
#define WIOTERM_H

#define USE_SDIO 0
SdFat SD;
#define SDINIT SDCARD_SS_PIN
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
  uint8 func  = LOW_REGISTER(dmaaddr);
  uint8 param = HIGH_REGISTER(dmaaddr);

  switch (func) {

    default:
      return (0x0000);
  }
}

#endif
