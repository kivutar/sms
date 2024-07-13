#include <fcntl.h>
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
retro_audio_sample_batch_t audio_cb;

int t = 0;
uint32_t r[16];
extern uint16_t pc, curpc, sp;
uint32_t asp, irq, stop;
int doflush = 0;
// uint8_t header[0x7fff];
// uint8_t *prg = NULL;
uint8_t *rom = NULL;
uint8_t *mem = NULL;
uint8_t *pic = NULL;

int vdpclock = 0;

void
loadrom(const uint8_t *data)
{
	// memcpy(header, data, sizeof(header));
	// prg = malloc(0xC000);
	// memcpy(prg, data+sizeof(header), 0xC000);
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
	info->timing.sample_rate = 48000;

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
	vdpmode();
	return true;
}

void
process_inputs()
{
}

int counter = 0;
int total = 0;

void
retro_run(void)
{
	printf("%d ================================\n", counter++);
	input_poll_cb();
	process_inputs();

	while(!doflush){
		t = z80step();
		printf("cycles: %d\n", t);
		vdpclock -= t * Z80DIV;
		total += t * Z80DIV;

		while(vdpclock < 0){
			vdpstep();
			vdpclock += 8;
		}

		if (total > 702240){
			flush();
			total = 0;
			vdpx = 0;
			vdpy = 0;
		}
	}

	video_cb(pic, 320, 224, 320*4);
	// audioout();
	doflush = 0;
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
retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
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
void retro_set_audio_sample(retro_audio_sample_t cb) {}
void retro_cheat_reset(void) {}
void retro_cheat_set(unsigned index, bool enabled, const char *code) {}
bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info) { return false; }
unsigned retro_get_region(void) { return 0; }
