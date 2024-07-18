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
enum { DARK, NORM, BRIGHT };
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

static uint32_t
shade(uint32_t v, int l)
{
	if(l == 1)
		return v;
	if(l == 2)
		return v << 1 & 0xefefef;
	return v >> 1 & 0xf7f7f7;
}

static void
pixel(int v, int p)
{
	if(p >= pri){
		col = v;
		pri = p;
	}
}

struct pctxt {
	uint8_t ws, w, hs, h;
	uint16_t tx, ty;
	uint8_t tnx, tny;
	uint16_t t;
	uint32_t c;
} pctxt[3];
int lwin, rwin;

static void
tile(struct pctxt *p)
{
	uint16_t a;
	int y;

	switch(p - pctxt){
	default: a = (reg[PANT] & 0x38) << 9; break;
	case 1: a = (reg[PBNT] & 7) << 12; break;
	case 2: a = (reg[PWNT] & 0x3e) << 9; break;
	}
	a += p->ty << p->ws;
	a += p->tx;
	p->t = vram[a];
	y = p->tny;
	if(intla){
		if((p->t & 0x1000) != 0)
			y = 15 - y;
		a = (p->t & 0x7ff) << 5 | y << 1;
	}else{
		if((p->t & 0x1000) != 0)
			y = 7 - y;
		a = (p->t & 0x7ff) << 4 | y << 1;
	}
	p->c = vram[a] << 16 | vram[a+1];
}

static void
planeinit(void)
{
	// printf("planeinit\n");
	static int szs[] = {5, 6, 6, 7};
	int v, a, i;
	struct pctxt *p;

	pctxt[0].hs = pctxt[1].hs = szs[reg[PLSIZ] >> 4 & 3];
	pctxt[0].ws = pctxt[1].ws = szs[reg[PLSIZ] & 3];
	pctxt[2].ws = (reg[MODE4] & WIDE) != 0 ? 6 : 5;
	pctxt[2].hs = 5;
	for(i = 0; i <= 2; i++){
		pctxt[i].h = 1<<pctxt[i].hs;
		pctxt[i].w = 1<<pctxt[i].ws;
	}
	a = reg[HORSCR] << 9 & 0x7fff;
	switch(reg[MODE3] & 3){
	case 1: a += vdpy << 1 & 0xe; break;
	case 2: a += vdpy << 1 & 0xff0; break;
	case 3: a += vdpy << 1 & 0xffe; break;
	}
	for(i = 0; i < 2; i++){
		p = pctxt + i;
		v = -(vram[a + i] & 0x3ff);
		p->tnx = v & 7;
		p->tx = v >> 3 & pctxt[i].w - 1;
		if(intla){
			v = vsram[i] + vdpyy;
			p->tny = v & 15;
			p->ty = v >> 4 & pctxt[i].h - 1;
		}else{
			v = vsram[i] + vdpy;
			p->tny = v & 7;
			p->ty = v >> 3 & pctxt[i].h - 1;
		}
		tile(p);
		if(p->tnx != 0)
			if((p->t & 0x800) != 0)
				p->c >>= p->tnx << 2;
			else
				p->c <<= p->tnx << 2;
	}
	sx = 0;
	snx = 0;
	v = reg[WINV] << 3 & 0xf8;
	if((reg[WINV] & 0x80) != 0 ? vdpy < v : vdpy >= v){
		lwin = 0;
		rwin = reg[WINH] << 4 & 0x1f0;
		if((reg[WINH] & 0x80) != 0){
			lwin = rwin;
			rwin = 320;
		}
	}else{
		lwin = 0;
		rwin = 320;
	}
	if(rwin > lwin){
		p = pctxt + 2;
		p->tx = lwin >> 3 & pctxt[2].w - 1;
		p->tnx = lwin & 7;
		p->tny = vdpy & 7;
		p->ty = vdpy >> 3 & pctxt[2].h - 1;
		tile(p);
	}
}

static void
plane(int n, int vis)
{
	struct pctxt *p;
	uint8_t v, pr;

	p = pctxt + n;
	if((p->t & 0x800) != 0){
		v = p->c & 15;
		p->c >>= 4;
	}else{
		v = p->c >> 28;
		p->c <<= 4;
	}
	if(vis != 0){
		if(v != 0){
			v |= p->t >> 9 & 48;
			pr = 2 - (n & 1) + (p->t >> 13 & 4);
			pixel(v, pr);
		}
		lum |= p->t >> 15;
	}
	if(++p->tnx == 8){
		p->tnx = 0;
		if(++p->tx == p->w)
			p->tx = 0;
		tile(pctxt + n);
	}
}

static void
planes(void)
{
	int i, w;
	uint8_t v;

	if((reg[MODE3] & 4) != 0 && ++snx == 16){
		snx = 0;
		sx++;
		for(i = 0; i < 2; i++){
			v = vsram[sx + i] + vdpy;
			pctxt[i].tny = v & 7;
			pctxt[i].ty = v >> 3 & pctxt[i].h - 1;
		}
	}
	w = vdpx < rwin && vdpx >= lwin;
	plane(0, !w);
	plane(1, 1);
	if(w)
		plane(2, 1);
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

		if(vdpy >= y && vdpy <= y+h && y > 1){
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

	if(vdpx == 0){
		planeinit();
		spritesinit();
	}
	if(vdpx < 320 && vdpy < 224)
		if(vdpx < xdisp){
			col = reg[BGCOL] & 0x3f;
			pri = 0;
			lum = 0;
			planes();
			sprites();
			if((reg[MODE2] & 0x40) != 0 && (vdpx >= 8 || (reg[MODE1] & 0x20) == 0)){
				v = cramc[col];
				if((reg[MODE4] & SHI) != 0)
					v = shade(v, lum);
				pixeldraw(vdpx, vdpy, v);
			}else
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
		if(vdpy == 0 || vdpy >= 225)
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
