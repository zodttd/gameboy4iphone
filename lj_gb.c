
/**
    Copyright (C) 2006-2008  The Little John Project (http://www.little-john.net)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
    lj_gb.c --
    Interworking with the LJ framework.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "defs.h"
#include "regs.h"
#include "lcd.h"
#include "fb.h"
#include "input.h"
#include "rc.h"
#include "pcm.h"
#include "mem.h"
#include "hw.h"
#include "rtc.h"
#include "sound.h"
#include "save.h"

#define INP_BUTTON_UP				(0)
#define INP_BUTTON_DOWN				(1)
#define INP_BUTTON_LEFT				(2)
#define INP_BUTTON_RIGHT			(3)
#define INP_BUTTON_HARDB			(4)
#define INP_BUTTON_HARDA			(5)
#define INP_BUTTON_HARDAB			(6)
#define INP_BUTTON_START			(7)
#define INP_BUTTON_SELECT			(8)


#define BIT_U			(1<<INP_BUTTON_UP)
#define BIT_D			(1<<INP_BUTTON_DOWN)
#define BIT_L 			(1<<INP_BUTTON_LEFT)
#define BIT_R		 	(1<<INP_BUTTON_RIGHT)
#define BIT_B			(1<<INP_BUTTON_HARDB)
#define BIT_A			(1<<INP_BUTTON_HARDA)
#define BIT_AB			(1<<INP_BUTTON_HARDAB)
#define BIT_ST			(1<<INP_BUTTON_START)
#define BIT_SEL			(1<<INP_BUTTON_SELECT)

#define BIT_LPAD1		(1<<29)
#define BIT_RPAD1		(1<<30)

extern void app_DemuteSound(void);
extern void app_MuteSound(void);
extern unsigned short  *BaseAddress;
extern int   __emulation_run;

char save_filename[512];
char rom_filename[512];
	
int sndfreq = 44100;

int skip_next_frame = 1;
int save_sram = 0;

/*void system_manage_sram(uint8 *sram, int slot, int mode)
{
     //
}*/

int rom_loaded = 0;

struct fb fb;

int	 dummy_align1;
char dummy_hq2x_stuff1[160*3*2];
char vidram[160*144*2];
char dummy_hq2x_stuff2[160*3*2];

void vid_preinit(void) {}
void vid_init(void)
{
       fb.w = 160;
       fb.h = 144;
       fb.pelsize = 2;
       fb.pitch = 160*2;
       fb.ptr = (unsigned char *)&vidram[0]; //0x0C7B4000;
       fb.enabled = 1;
       fb.dirty = 1; ///1????
       fb.yuv = 0;

       fb.indexed = 0;
       fb.cc[0].l = 11; fb.cc[0].r = 3;
       fb.cc[1].l = 5; fb.cc[1].r = 2;
       fb.cc[2].l = 0; fb.cc[2].r = 3;
}
void vid_begin(void)
{
}

void vid_close(void) {}

void vid_settitle(void) {}
int monopal_backup[256];
void vid_setpal(int i, int r, int g, int b)
{
 /*if (i>=253)  return;
 if (hw.cgb) SetColor(i,r,g,b);
  else {
       switch(fgbZ_mGBPal)
       {
        case 0: SetColor(i,g/2,g,0);break;   //green
        case 1: SetColor(i,g*3/4,g,0);break; //lime
        case 2: SetColor(i,g,g,0);break;     //yellow
        case 3: SetColor(i,g,g,g);break;     //gray
        case 4: SetColor(i,0,g*3/4,g);break; //indigo
       }
       monopal_backup[i]=g;
       }*/
}

void GetInfo(byte *header)
{

}


rcvar_t vid_exports[] =
{
	RCV_END
};

rcvar_t joy_exports[] =
{
	RCV_END
};

struct pcm pcm;
rcvar_t pcm_exports[] =
{
	RCV_END
};

void pcm_init(void)
{
   pcm.hz = sndfreq;
   pcm.stereo = 1;
   pcm.len = 512 * 1;
   pcm.buf = NULL; //malloc(pcm.len);
   pcm.pos = 0;
}

int pcm_submit()
{
	if (!pcm.buf) return 0;
	if (pcm.pos < pcm.len) return 1;

	pcm.pos = 0;
	return 1;
}

void pcm_close(void)
{
	//fgbZ_pause(1);
}

static void simpleWait(int thissec, int lim_time)
{
	struct timeval tval;

	//spend_cycles(1024);
	gettimeofday(&tval, 0);
	if(thissec != tval.tv_sec) tval.tv_usec+=1000000;

	while(tval.tv_usec < lim_time)
	{
		//spend_cycles(1024);
		gettimeofday(&tval, 0);
		if(thissec != tval.tv_sec) tval.tv_usec+=1000000;
	}
}

int EMU_MainLoop(void)
{
    emu_doframe();

    if(fb.enabled) 
	{
		static int framecount = 0;
		static struct timeval tval;
		static int thissec = 0;
		int lim_time;
		
		memcpy(BaseAddress, vidram, 160*144*2);
		updateScreen();
		
		// second changed?
		if (thissec != tval.tv_sec)
		{
			thissec = tval.tv_sec;
			framecount  -= 60; if (framecount  < 0) framecount  = 0;
		}

		lim_time = (framecount+1) * 16666;
		framecount++;
		gettimeofday(&tval, 0);
		if(thissec != tval.tv_sec) tval.tv_usec+=1000000;
		if(tval.tv_usec < lim_time) { // we are too fast
			static int turbomode = 0;
			unsigned long newkeys = joystick_read();
			
			if(!turbomode)
			{
				simpleWait(thissec, lim_time);
			}
			
			if((newkeys & (BIT_LPAD1)) && (newkeys & (BIT_RPAD1)))
			{
				turbomode = !turbomode;
				usleep(2000000);
				
				framecount = 0;
				thissec = 0;
			}			
		}
	}
    return 1;
}

int EMU_Init(int freq)
{
	setpriority(PRIO_PROCESS, 0, -20);
	
    sndfreq = freq;

    vid_init();
    pcm_init();

    emu_reset();

	app_DemuteSound();
	
    return 1;
}

int EMU_LoadRom (const char* filename)
{
    loader_init(filename);

    return 1;
}


#define GB_HANDLEKEY(inkey,outkey) if(newkeys & inkey) hw.pad |= PAD_##outkey;

int EMU_HandleInput(int* keys, int player)
{
	unsigned long newkeys = joystick_read();
	hw.pad = 0;
	
    GB_HANDLEKEY(BIT_U, UP)
    GB_HANDLEKEY(BIT_D, DOWN)
    GB_HANDLEKEY(BIT_L, LEFT)
    GB_HANDLEKEY(BIT_R, RIGHT)
    GB_HANDLEKEY(BIT_A, A)
    GB_HANDLEKEY(BIT_B, B)
    GB_HANDLEKEY(BIT_ST, START)
    GB_HANDLEKEY(BIT_SEL, SELECT)

    pad_refresh();

    return 1;
}

char* keynames[] = {"up", "down", "left", "right", "a", "b", "start", "select"};

char* EMU_KeyName(int key)
{
    return keynames[key];
}

void EMU_AudioCallback(unsigned char *stream, int len)
{
     sound_mix(stream, len/2);
}

void EMU_Shutdown(void)
{
	EMU_CheckSRAM();
	rtc_save();
	app_MuteSound();
     //system_shutdown();
}

int EMU_CheckSRAM(void)
{
    //if(save_sram)
    {
        save_sram = 0;
        EMU_SaveSRAM();
    }
}

int EMU_SaveSRAM(void)
{
	FILE* f;
	char sram_filename[512];
	
    /* If we crash before we ever loaded sram, DO NOT SAVE! */
    if (!mbc.batt || !ram.loaded || !mbc.ramsize)  return -1;
	
	/* Consider sram loaded at this point, even if file doesn't exist */
	ram.loaded = 1;

	
	sprintf(sram_filename, "%s.sav", rom_filename);
	
	f = fopen(sram_filename, "w");

    if(!f) return 0;

    fwrite((void *)&ram.sbank[0],8192*mbc.ramsize,1,f);

    fclose(f);

    return 1;
}

int EMU_LoadSRAM(void)
{
	FILE* f;
	char sram_filename[512];

    if (!mbc.batt) return 0;

	/* Consider sram loaded at this point, even if file doesn't exist */
	ram.loaded = 1;

	
	sprintf(sram_filename, "%s.sav", rom_filename);
	
	f = fopen(sram_filename, "r");
	
	if(!f) return 1; //not existent

    fread((void *)&ram.sbank[0],8192*mbc.ramsize,1,f);

    fclose(f);

    return 1;
}

int EMU_SaveState(const char* filename)
{
    FILE* f = fopen(filename, "w");
    if(!f) return 0;

    savestate(f);

    fclose(f);

    return 1;
}

int EMU_LoadState(const char* filename)
{
    FILE* f = fopen(filename, "r");
    if(!f) return 1; //non existent

    loadstate(f);

    fclose(f);

    vram_dirty();
    pal_dirty();
    sound_dirty();
    mem_updatemap();

    return 1;
}

void EMU_RenderFrame(int value)
{
     fb.enabled = value;
}

int EMU_SnapShot(void* ptr)
{
    //
}

void EMU_SetSound(int value)
{
     //snd.enabled = value;
}

void EMU_SetSoundFreq(int value)
{
    sndfreq = value;
    //pcm_init();
}

int iphone_main(char* filename)
{
    //Int32 timecpt,framecpt = 0;

	struct timeval detail_time;	
	strncpy(rom_filename, filename, 512);
	rom_filename[512-1] = 0;

	if( (!strcasecmp(rom_filename + (strlen(rom_filename)-4), ".svs")) )
	{
		unsigned long pos;
		sprintf(save_filename, "%s", rom_filename);
		pos = strlen(rom_filename)-18;
		rom_filename[pos] = '\0';
	}
	
	//Load the Rom
	EMU_LoadRom(rom_filename);

	EMU_Init(44100);

	if (strlen(save_filename) > 0) 
	{
		EMU_LoadState(save_filename);
		save_filename[0] = '\0';
	}
	
	EMU_LoadSRAM();
	
	
	//gettimeofday(&detail_time,NULL);
	//timecpt = detail_time.tv_usec / 1000;

	while (__emulation_run)
	{
		EMU_HandleInput(NULL, 0);

		EMU_MainLoop();
	}
	
	{
		char buffer[512];
		char tmpfilename[512];
		time_t curtime;
		struct tm *loctime;

		curtime = time (NULL);
		loctime = localtime (&curtime);
		strftime (buffer, 260, "%y%m%d-%I%M%p", loctime);
		sprintf(tmpfilename, "%s-%s.svs", rom_filename, buffer);

		EMU_SaveState(tmpfilename);
		app_SetSvsFile(tmpfilename);
	}
		
	EMU_Shutdown();
	
	pthread_exit(NULL);

	return 0;
}

void die(char *fmt, ...)
{
	char tmp[256];

	va_list ap;

	va_start(ap, fmt);
	vfprintf(tmp, fmt, ap);
	va_end(ap);

	//printf(dietext);
	//pthread_exit(NULL);
}

void doevents(void)
{
	
}
