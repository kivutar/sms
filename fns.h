#include <stdint.h>

uint16_t memread(uint32_t);
void memwrite(uint32_t, uint16_t, uint16_t);
int z80step(void);
uint8_t z80read(uint16_t);
void z80write(uint16_t, uint8_t);
uint8_t z80in(uint8_t);
void z80out(uint8_t, uint8_t);
void vdpstep(void);
void flush(void);
void vdpmode(void);
void ymwrite(uint8_t, uint8_t, uint8_t);
