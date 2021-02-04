#ifndef HOST_H
#define HOST_H

uint16 hostbdos(uint16 dmaaddr) {

  uint16 i = WORD16(dmaaddr);
  uint8 func = _RamRead(i);
  ++i;

  switch (func) {
    case 1: // CPU Speed
      return (F_CPU / 1000000L);
      break;
    case 2: // Screen Width (Pixels)
      return (RSP_W);
      break;
    case 3: // Screen Hight (Pixels)
      return (RSP_H);
      break;
    case 4: // Screen Width (Text)
      return (SC_W);
      break;
    case 5: // Screen Hight (Text)
      return (SC_H);
      break;
    case 6: // Font Width (Pixels)
      return (CH_W);
      break;
    case 7: // Font Hight (Pixels)
      return (CH_H);
      break;
    default:
      return (0x0000);
  }
}

#endif
