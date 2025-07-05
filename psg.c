#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "dat.h"

#define NOISE_TAPPED 0x9

static const uint8_t noise_table[3] = { 0x10, 0x20, 0x40 };
static const short vol_table[16] = {
	8191, 6507, 5168, 4105, 3261, 2590, 2057,
	1642, 1298, 1031, 819, 650, 516, 410, 326, 0
};

static double cyclespersample;
static double cycles;
static uint16_t freqreg[4];
static uint16_t countreg[4];
static uint8_t attn[4];
static bool flipflop[4];
static uint16_t noise;
static uint8_t curr_reg;
static uint8_t curr_type;

void
psginit(const uint16_t rate, const int clockspd)
{
	cyclespersample = (double)clockspd / (double)PSGDIV/ (double)rate;
	cycles = cyclespersample;
}

void
psgwrite(const uint8_t data)
{
	bool first = data & 128;
	if(first){
		curr_reg = (data >> 5) & 3;
		curr_type = (data >> 4) & 1;
	}

	if(curr_type){
		attn[curr_reg] = data & 0x0F;
	}else if(first && curr_reg == 3){
		freqreg[3] = data & 7;
		noise = 0x8000;
	}
	else if(first)
		freqreg[curr_reg] = (freqreg[curr_reg] & 0x3F0) | (data & 0x0F);
	else
		countreg[curr_reg] = freqreg[curr_reg] = (freqreg[curr_reg] & 0x0F) | ((data & 0x3F) << 4);
}

static inline uint16_t
parity(uint16_t v)
{
	v ^= v >> 8;
	v ^= v >> 4;
	v ^= v >> 2;
	v ^= v >> 1;
	v &= 1;
	return v;
}

static inline uint16_t
vol(uint8_t chn)
{
	return (flipflop[chn] ? 1 : -1) * vol_table[attn[chn]];
}

int16_t
psgstep()
{
	while(cycles > 0){
		for(uint8_t i = 0; i < 4; i++){
			countreg[i]--;
			if(!countreg[i]){
				if(i < 3){
					countreg[i] = freqreg[i];
					flipflop[i] = !flipflop[i];
				}else{
					uint8_t nf = freqreg[3] & 3;
					uint8_t fb = (freqreg[3] >> 2) & 1;
					countreg[3] = nf == 3 ? freqreg[2] : (0x10 << nf);

					noise = (noise >> 1) | ((fb ? parity(noise & NOISE_TAPPED) : noise & 1) << 15);
					flipflop[3] = (noise & 1);
				}
			}
		}

		cycles--;
	}

	cycles += cyclespersample;

	return vol(0) + vol(1) + vol(2) + vol(3);
}
