#ifndef HOST_H
#define HOST_H

uint16 hostbdos(uint16 dmaaddr) {
	switch(dmaaddr) {
  case 1:
    return(F_CPU / 1000000L); // CPU Speed
    break;
	default:
    return(0x0000);
	}
}

#endif
