o#ifndef ESP32_H
#define ESP32_H

SdFat SD;
#define SDINIT 18,19,23,4 // M5Stack
//#define SDINIT 18,38,23,4 // M5Stack Core2
#define SDMHZ 20
#define LED 26
#define LEDinv 1
#define BOARD "M5Stack"
#define board_esp32
#define board_digital_io

uint8 esp32bdos(uint16 dmaaddr) {
	return(0x00);
}

#endif