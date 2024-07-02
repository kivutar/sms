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

int slotaddr[3] = {0, 0, 0};

uint8_t
z80read(uint16_t a)
{
	printf("z80read %x\n", a);
	uint16_t v;

	if (a < 0x400)
		return rom[a];
	else if (a < 0x4000)
		return rom[a + slotaddr[0]];
	else if (a < 0x8000)
		return rom[a - 0x4000 + slotaddr[1]];
	else if (a < 0xC000)
		return rom[a - 0x8000 + slotaddr[2]];
	else
		return mem[a];
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

        switch (a)
        {
            case 0xFFFC:
                printf("Persistent RAM");
                break;
            case 0xFFFD:
                printf("Switch mapper slot 0 to %d\n", v);
				slotaddr[0] = v * 0x4000;
                break;
            case 0xFFFE:
                printf("Switch mapper slot 1 to %d\n", v);
				slotaddr[1] = v * 0x4000;
                break;
            case 0xFFFF:
                printf("Switch mapper slot 2 to %d\n", v);
				slotaddr[2] = v * 0x4000;
                break;
		}
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
