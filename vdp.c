#include <stdint.h>
#include <stdio.h>
#include "dat.h"
#include "fns.h"

extern uint8_t *pic;

uint8_t vdpcode, vdpstat = 0;
uint16_t vdpaddr;
uint8_t vdpbuf;
int vdpx = 0, vdpy, vdpyy, frame, intla;
int first = 1;
uint16_t hctr;
static int xmax, xdisp;
static int sx, snx, col, pri, lum;
enum { ymax = 262, yvbl = 234 };

void
vdpmode(void)
{
	if((reg[MODE4] & WIDE) != 0){
		xmax = 406;
		xdisp = 320;
	}else{
		xdisp = 256;
		xmax = 342;
	}
	intla = (reg[MODE4] & 6) == 6;
}

static void
pixeldraw(int x, int y, int v)
{
	uint32_t *p;
	union { uint32_t l; uint8_t b[4]; } u;

	p = (uint32_t *)pic + (x + y * 320);
	u.b[0] = v >> 16;
	u.b[1] = v >> 8;
	u.b[2] = v;
	u.b[3] = 0;
	*p = u.l;
}

static void
plane(int n, int vis)
{

}

static void
planes(void)
{

}

static struct sprite {
	uint16_t y, x;
	uint8_t w, h;
	uint16_t t;
	uint32_t c[4];
} spr[21], *lsp;

int sprlst[64] = {-1};

static void
spritesinit(void)
{
	printf("spritesinit\n");

	uint16_t t1 = (reg[SPRTAB] << 7 & 0x3f00);
	uint16_t t2 = t1 + 0x80;

	int bufidx = 0;
	for(int i = bufidx; i < 8; i++)
		sprlst[i] = -1;

	for (int i = 0; i < 64; i++) {
		int spridx = t1 + i;

		if (vram[spridx] == 0xd0)
			break;

		int y = vram[spridx] + 1;
		int h = (reg[1] & 1 << 1) != 0 ? 16 : 8;

		if(vdpy >= y && vdpy < y+h && y > 1){
			if(bufidx >= 8){
				if (vdpy < 192)
					vdpstat |= STATOVR;
				break;
			}

			sprlst[bufidx] = i;
			bufidx++;
		}
	}
}

static void
sprites(void)
{
	if (vdpy > 192) return;

	uint16_t t1 = (reg[SPRTAB] << 7 & 0x3f00);
	uint16_t t2 = t1 + 0x80;
	uint16_t t3 = (reg[6] << 11) & 0x2000;

	for (int i = 7; i >= 0; i--) {
		if (sprlst[i] < 0) continue;

		int spr = sprlst[i];
		int spridx = t1 + spr;
		int y = vram[spridx] + 1;
		uint16_t info = t2 + (spr << 1);
		int x = vram[info];
		int h = (reg[1] & 1 << 1) != 0 ? 16 : 8;

		if (x >= 256) continue;

		int t = vram[info + 1];
		t &= (reg[1] & 1 << 1) != 0 ? 0xfe : 0xff;
		int taddr = t3 + (t << 5) +  ((vdpy - y) << 2);

		for (int xx = 0; xx < 8; xx++){
			if (x+xx > 256) continue;

			int c = ((vram[taddr] >> (7 - xx)) & 0x01) +
					(((vram[taddr + 1] >> (7 - xx)) & 0x01) << 1) +
					(((vram[taddr + 2] >> (7 - xx)) & 0x01) << 2) +
					(((vram[taddr + 3] >> (7 - xx)) & 0x01) << 3);

			if (c == 0) continue;

			pixeldraw(x+xx, vdpy, cramc[c + 16]);
		}
	}
}

void
vdpctrl(uint8_t v)
{
	printf("	vdp write to control port %x\n", v);

	if(first){
		printf("first\n");
		first = 0;
		vdpaddr = (vdpaddr & 0xFF00) | v;
		return;
	}

	vdpcode = (v >> 6) & 0x03;
	vdpaddr = (vdpaddr & 0x00ff) | ((v & 0x3f) << 8);

	printf("vdp code and address %x %x\n", vdpcode, vdpaddr);
	first = 1;

	switch(vdpcode){
		case 0:
			vdpbuf = vram[vdpaddr];
			vdpaddr++;
			vdpaddr &= 0x3fff;
			break;
		case 2: reg[v & 0x0f] = (vdpaddr & 0x00ff); break;
	}
}

void
vdpdata(uint8_t v)
{
	printf("	vdp (code: %x) write to data port %x\n", vdpcode, v);
	first = 1;
	vdpbuf = v;
	switch(vdpcode){
		case 0: case 1: case 2:
			vram[vdpaddr] = v;
			printf("vramwrite %x %x\n", vdpaddr, v);
		break;
		case 3: cramwrite(vdpaddr, v); break;
	}
	vdpaddr++;
	vdpaddr &= 0x3fff;
}

uint8_t
vdpdataport(void)
{
	uint8_t v = vdpbuf;
	vdpbuf = vram[vdpaddr];
	vdpaddr++;
	vdpaddr &= 0x3fff;
	printf("	vdp read from data port %x\n", v);
	first = 1;
	return v;
}

uint8_t
vdpstatus(void)
{
	uint8_t v = vdpstat | 0x1f;
	vdpstat = 0;
	z80irq = 0;
	printf("	vdp read status flags %x\n", v);
	first = 1;
	return v;
}

uint8_t
vdphcounter(void)
{
	printf("	vdp read hcounter %x\n", vdpx);
	return vdpx;
}

uint8_t
vdpvcounter(void)
{
	printf("	vdp read vcounter %x\n", vdpy);
	return vdpy;
}

void
vdpstep(void)
{
	uint32_t v;

	if(vdpx == 0)
		spritesinit();

	if(vdpx < 320 && vdpy < 192)
		if(vdpx < xdisp){
			col = reg[BGCOL] & 0x0f + 16;
			pri = 0;
			lum = 0;
			planes();
			sprites();
			pixeldraw(vdpx, vdpy, 0);
		}else
			pixeldraw(vdpx, vdpy, 0xcccccc);
	if(++vdpx >= xmax){
		z80irq = 0;
		vdpx = 0;
		if(++vdpy >= ymax){
			vdpy = 0;
			irq &= ~INTVBL;
			vdpstat ^= STATFR;
			vdpstat &= ~(STATINT | STATVBL | STATOVR | STATCOLL);
			flush();
		}
		if(intla)
			vdpyy = vdpy << 1 | frame;
		if(vdpy == 0 || vdpy >= 193)
			hctr = reg[HORCTR];
		else
			if(hctr-- == 0){
				if((reg[MODE1] & IE1) != 0)
					irq |= INTHOR;
				hctr = reg[HORCTR];
			}
		if(vdpy == yvbl){
			vdpstat |= STATVBL | STATINT;
			frame ^= 1;
			z80irq = 1;
			if((reg[MODE2] & IE0) != 0)
				irq |= INTVBL;
		}
	}
}
