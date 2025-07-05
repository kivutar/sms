#include <stdint.h>

extern uint32_t irq;

extern uint8_t reg[16];

extern uint8_t z80bus, z80irq;
extern uint16_t pc, curpc;

extern uint16_t ram[32768];
extern uint8_t *prg;
extern uint8_t *rom;
extern uint8_t *mem;

extern uint8_t vram[32768], vsram[40];
extern uint8_t cram[64];
extern uint32_t cramc[64];
extern int vdpx, vdpy, frame, intla;

extern uint8_t portDC;
extern uint8_t portDD;

enum {
	MODE1   = 0x00,
	MODE2   = 0x01,
	PANT    = 0x02,
	PWNT    = 0x03,
	PBNT    = 0x04,
	SPRTAB  = 0x05,
	SPRADDR = 0x06,
	BGCOL   = 0x07,
	HORSCR  = 0x08,
	VERSCR  = 0x09,
	HORCTR  = 0x0a,

	IE0     = 0x20,
	IE1     = 0x10,

	STATCOLL= 0x20,
	STATOVR = 0x40,
	STATINT = 0x80,
};

enum {
	BUSREQ = 1,
	BUSACK = 2,
	RESET  = 4,

	INTVBL = 1,
	INTHOR = 2,
};

enum {
	FREQ = 53203400,
	Z80DIV = 15,
	RATE = 44100,
	SAVEFREQ = FREQ / 4,
	PSGCLOCK = 3579545,
	PSGDIV = 16,
};

enum {
	SRAM = 0x01,
	BATTERY = 0x02,
	ADDRMASK = 0x0c,
	ADDRBOTH = 0x00,
	ADDREVEN = 0x08,
	ADDRODD = 0x0c,
	SRAMEN = 0x10,
};
