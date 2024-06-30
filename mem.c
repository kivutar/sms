#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "fns.h"
#include "dat.h"

#define sysfatal(fmt, ...){printf(fmt"\n", ##__VA_ARGS__); exit(EXIT_FAILURE);}

uint16_t ram[32768], vram[32768];
uint16_t cram[64], vsram[40];
uint32_t cramc[64];
uint8_t zram[8192];
uint8_t reg[32];
uint8_t ctl[15];

uint8_t dma;
uint8_t vdplatch;
uint16_t vdpaddr, vdpdata;

uint8_t yma1, yma2;

uint8_t z80bus = 0;
uint16_t z80bank;

uint8_t
z80read(uint16_t a)
{
	printf("z80read %x\n", a);
	uint16_t v;

	if (a < 0x8000)
		return rom[a];
	else
		printf("z80read > 0x8000 %x\n", a);
}

void
z80write(uint16_t a, uint8_t v)
{
	if (a < 0x8000)
		printf("wrong z80write %x %x\n", a, v);
    else if (a < 0xE000)
    {
        rom[a] = v;
        rom[a + 0x2000]  = v;
    }
	else
	{
		printf("z80write > 0xE000 %x %x\n", a, v);
        rom[a] = v;
        rom[a - 0x2000]  = v;
	}
}

uint8_t
z80in(uint8_t a)
{
	return 0xff;
}

void
z80out(uint8_t a, uint8_t b)
{
}
