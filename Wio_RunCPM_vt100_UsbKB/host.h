#ifndef HOST_H
#define HOST_H

uint16 hostbdos(uint16 dmaaddr) {

  uint8 func  = LOW_REGISTER(dmaaddr);
  uint8 param = HIGH_REGISTER(dmaaddr);

  switch (func) {
    case 1:
      return (F_CPU / 1000000L); // CPU Speed
      break;
    default:
      return (0x0000);
  }
}

#endif
