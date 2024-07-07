#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "fns.h"
#include "dat.h"

#define sysfatal(fmt, ...){printf(fmt"\n", ##__VA_ARGS__); exit(EXIT_FAILURE);}

uint16_t ram[32768] = {0};
uint16_t vram[32768];
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
uint16_t ram_bank;
uint8_t ram_enabled = 0;

int slotaddr[3] = {0, 0, 0};
int nbank = 16;

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
		return rom[(a - 0x4000) + slotaddr[1]];
	else if (a < 0xC000){
		if (ram_enabled)
			return ram[(a - 0x8000) + ram_bank];
		else{
			printf("== page 2 %x %x %x\n", a, (a - 0x8000) + slotaddr[2], slotaddr[2]);
			printf("== rom[(a - 0x8000) + slotaddr[2]] %x\n", rom[(a - 0x8000) + slotaddr[2]]);
			return rom[(a - 0x8000) + slotaddr[2]];
		}
	}else
		return mem[a];
}

void
z80write(uint16_t a, uint8_t v)
{
	printf("z80write %x %x\n", a, v);
	if (a < 0x8000)
		printf("wrong z80write page 0 or 1 %x %x\n", a, v);
	else if (a < 0xC000)
		if (ram_enabled)
			ram[(a - 0x8000) + ram_bank] = v;
		else
			printf("wrong z80write page 2 %x %x\n", a, v);
	else if (a < 0xE000)
	{
		mem[a] = v;
		mem[a + 0x2000]  = v;
	}
	else
	{
		// printf("z80write > 0xE000 %x %x\n", a, v);
		mem[a] = v;
		mem[a - 0x2000]  = v;

		switch (a)
		{
			case 0xFFFC:
				ram_bank = (v & (1 << 2)) != 0 ? 0x4000 : 0;
				printf("RAM bank %x\n", ram_bank);
				ram_enabled = (v & (1 << 3)) != 0 ? 1 : 0;
				printf("RAM enabled %x\n", ram_enabled);
				break;
			case 0xFFFD:
				printf("Switch mapper slot 0 to %d\n", (v & nbank-1));
				slotaddr[0] = (v & nbank-1) * 0x4000;
				break;
			case 0xFFFE:
				printf("Switch mapper slot 1 to %d\n", (v & nbank-1));
				slotaddr[1] = (v & nbank-1) * 0x4000;
				break;
			case 0xFFFF:
				printf("Switch mapper slot 2 to %d\n", (v & nbank-1));
				slotaddr[2] = (v & nbank-1) * 0x4000;
				break;
		}
	}
}

uint8_t
z80in(uint8_t a)
{
	printf("z80in %x\n", a);
	if (a == 0xBF)
		return 0x1F;
	return 0xff;
}

void
z80out(uint8_t port, uint8_t v)
{
	printf("z80out %x %x\n", port, v);

	if (port < 0x40)
		printf("  write to control register\n");
	else if ((port >= 0x40) && (port < 0x80))
		printf("  write to SN76489 PSG\n");
	else if ((port >= 0x80) && (port < 0xC0))
		printf("  write to VDP\n");
	else
		printf("  write with no effect\n");
}
