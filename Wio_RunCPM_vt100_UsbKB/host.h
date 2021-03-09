#ifndef HOST_H
#define HOST_H

extern void initScreenEx(uint8_t r);

uint16 hostbdos(uint16 dmaaddr) {

  uint16 i = WORD16(dmaaddr);
  uint8 func = _RamRead(i++);

  switch (func) {
    case 0: // Reset
      resetSystem();
      break;
    case 1: // CPU Speed
      return (F_CPU / 1000000L);
    case 2: // Screen Width (Pixels)
      return (RSP_W);
    case 3: // Screen Hight (Pixels)
      return (RSP_H);
    case 4: // Screen Width (Text)
      return (SC_W);
    case 5: // Screen Hight (Text)
      return (SC_H);
    case 6: // Font Width (Pixels)
      return (CH_W);
    case 7: // Font Hight (Pixels)
      return (CH_H);
    case 8: // Get Screen Rotation Angle (0..3)
      return (ROTATION_ANGLE % 4);
    case 9: // Set Screen Rotation Angle (0..3)
      ROTATION_ANGLE = _RamRead(i) % 4;
      initScreenEx(ROTATION_ANGLE);
      return (0);
    case 10: // Debug Serial
      DS.value = _RamRead(i);
      return (0);
    default:
      return (0x0000);
  }
}

#endif
