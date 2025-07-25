#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libretro.h"
#include "dat.h"
#include "fns.h"

static retro_input_state_t input_state_cb;
static retro_input_poll_t input_poll_cb;
static retro_video_refresh_t video_cb;
static retro_environment_t environ_cb;
retro_audio_sample_t audio_cb;

int t = 0;
uint32_t r[16];
extern uint16_t pc, curpc, sp;
uint32_t irq;
int doflush = 0;
uint8_t *rom = NULL;
uint8_t *mem = NULL;
uint8_t *pic = NULL;
int vdpclock = 0;
int psgclock = 0;
uint8_t keys[2];

void
loadrom(const uint8_t *data)
{
	pic = malloc(320 * 224 * 4);
	rom = malloc(0x40000);
	memcpy(rom, data, 0x40000);
	mem = malloc(0xC000+0x8000);
	memcpy(mem, rom, 0xC000);
	pc = 0;
}

void
retro_init(void)
{
}

void
retro_get_system_info(struct retro_system_info *info)
{
	memset(info, 0, sizeof(*info));
	info->library_name = "sms";
	info->library_version = "1.0";
	info->need_fullpath = false;
	info->valid_extensions = "sms";
}

void
retro_get_system_av_info(struct retro_system_av_info *info)
{
	info->timing.fps = 60.0;
	info->timing.sample_rate = RATE;

	info->geometry.base_width = 320;
	info->geometry.base_height = 224;
	info->geometry.max_width = 320;
	info->geometry.max_height = 224;
	info->geometry.aspect_ratio = 4.0 / 3.0;
}

unsigned
retro_api_version(void)
{
	return RETRO_API_VERSION;
}

bool
retro_load_game(const struct retro_game_info *game)
{
	printf("Loading game\n");
	enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
	if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
		return false;

	loadrom(game->data);
	psginit(RATE, PSGCLOCK);
	vdpmode();
	return true;
}

static const int retro_bind[] = {
	[RETRO_DEVICE_ID_JOYPAD_UP]    = 0,
	[RETRO_DEVICE_ID_JOYPAD_DOWN]  = 1<<1,
	[RETRO_DEVICE_ID_JOYPAD_LEFT]  = 1<<2,
	[RETRO_DEVICE_ID_JOYPAD_RIGHT] = 1<<3,
	[RETRO_DEVICE_ID_JOYPAD_A]     = 1<<4,
	[RETRO_DEVICE_ID_JOYPAD_B]     = 1<<5,
	[RETRO_DEVICE_ID_JOYPAD_START] = 1<<6,
};

void
process_inputs()
{
	for(int p = 0; p < 2; p++)
	{
		keys[p] = 0xff;
		for(int id = 0; id < RETRO_DEVICE_ID_JOYPAD_X; id++)
			if(input_state_cb(p, RETRO_DEVICE_JOYPAD, 0, id))
				keys[p] = keys[p] & ~retro_bind[id];
	}

	portDC = (keys[0] & 0x3f) + ((keys[1] << 6) & 0xc0);
	portDD = ((keys[1] >> 2) & 0x0f) | 0xf0;
}

int counter = 0;
int total = 0;
int samplescounter = 0;

void
retro_run(void)
{
	// printf("%d ================================\n", counter++);
	input_poll_cb();
	process_inputs();

	while(!doflush){
		t = z80step();
		// printf("cycles: %d\n", t);
		vdpclock -= t * Z80DIV;
		total += t;

		while(vdpclock < 0){
			vdpstep();
			vdpclock += 8;
		}
		// printf("    vdp registers: %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n", reg[0], reg[1], reg[2], reg[3], reg[4], reg[5], reg[6], reg[7], reg[8], reg[9], reg[10], reg[11], reg[12], reg[13], reg[14], reg[15]);

		// printf("vram:\n");
		// for(int i=14000;i<17000;i++)
		// 	printf("%x ", vram[i]);
		// printf("\n");

		if (total > 702240){
			flush();
			total = 0;
			vdpx = 0;
			vdpy = 0;
		}
	}

	video_cb(pic, 256, 192, 320*4);

	for(int i = 0; i < 736; i++){
		int16_t frame = psgstep();
		audio_cb(frame, frame);
		samplescounter++;
	}

	//printf("Samples: %d\n", samplescounter);
	samplescounter = 0;

	doflush = 0;
	total = 0;
}

void
flush(void)
{
	doflush = 1;
}

void
retro_set_input_poll(retro_input_poll_t cb)
{
	input_poll_cb = cb;
}

void
retro_set_input_state(retro_input_state_t cb)
{
	input_state_cb = cb;
}

void
retro_set_video_refresh(retro_video_refresh_t cb)
{
	video_cb = cb;
}

void
retro_set_environment(retro_environment_t cb)
{
	environ_cb = cb;
}

void
retro_set_audio_sample(retro_audio_sample_t cb)
{
	audio_cb = cb;
}

void
retro_reset(void)
{
	doflush = 0;
}

size_t
retro_serialize_size(void)
{
	return 0;
}

bool
retro_serialize(void *data, size_t size)
{
	return false;
}

bool
retro_unserialize(const void *data, size_t size)
{
	return false;
}

void retro_set_controller_port_device(unsigned port, unsigned device) {}
size_t retro_get_memory_size(unsigned id) { return 0; }
void * retro_get_memory_data(unsigned id) { return NULL; }
void retro_unload_game(void) {}
void retro_deinit(void) {}
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) {}
void retro_cheat_reset(void) {}
void retro_cheat_set(unsigned index, bool enabled, const char *code) {}
bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info) { return false; }
unsigned retro_get_region(void) { return 0; }
