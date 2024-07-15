#include <stdint.h>

extern uint32_t irq;

extern uint8_t reg[16];
extern uint8_t dma;

extern uint8_t z80bus, z80irq;
extern uint16_t pc, curpc;

extern uint16_t ram[32768];
extern uint8_t *prg;
extern uint8_t *rom;
extern uint8_t *mem;

extern uint16_t vram[32768], vsram[40];
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
	BGCOL   = 0x07,
	HORCTR  = 0x0a,
	MODE3   = 0x0b,
	MODE4   = 0x0c,
	HORSCR  = 0x0d,
	AUTOINC = 0x0f,
	PLSIZ   = 0x10,
	WINH    = 0x11,
	WINV    = 0x12,
	DMACL   = 0x13,
	DMACH   = 0x14,
	DMASRC0 = 0x15,
	DMASRC1 = 0x16,
	DMASRC2 = 0x17,

	IE0     = 0x20,
	IE1     = 0x10,
	DMAEN   = 0x10,
	SHI     = 0x08,

	WIDE    = 0x01,

	STATDMA = 0x02,
	STATHBL = 0x04,
	STATVBL = 0x08,
	STATFR  = 0x10,
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
	YMDIV = 7 * 6,
	CPUDIV = 7,
	Z80DIV = 15,
	RATE = 44100,
	SAMPDIV = FREQ / RATE,
	SAVEFREQ = FREQ / 4,
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
