

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>



//#include <sys/stat.h>

#include "defs.h"
#include "regs.h"
#include "mem.h"
#include "hw.h"
#include "rtc.h"
#include "rc.h"


char *strdup();

static int mbc_table[256] =
{
	0, 1, 1, 1, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 3,
	3, 3, 3, 3, 0, 0, 0, 0, 0, 5, 5, 5, MBC_RUMBLE, MBC_RUMBLE, MBC_RUMBLE, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, MBC_HUC3, MBC_HUC1
};

static int rtc_table[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0
};

static int batt_table[256] =
{
	0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0,
	1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0,
	0
};

static int romsize_table[256] =
{
	2, 4, 8, 16, 32, 64, 128, 256, 512,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 128, 128, 128
	/* 0, 0, 72, 80, 96  -- actual values but bad to use these! */
};

static int ramsize_table[256] =
{
	1, 1, 1, 4, 16,
	4 /* FIXME - what value should this be?! */
};


#ifndef __PALM__
static
#endif
 char *romfile;
static char *sramfile;
static char rtcfile[512];
static char *saveprefix;

static char *savename;
static char *savedir;

static int saveslot;

static int forcebatt, nobatt;
static int forcedmg=0, gbamode;

static int memfill = 0, memrand = -1; //-1,-1


static void initmem(void *mem, int size)
{
	char *p = mem;
/*	if (memrand >= 0)
	{
		srand(memrand ? memrand : time(0));
		while(size--) *(p++) = rand();
	}
	else*/ if (memfill >= 0)
		memset(p, memfill, size);
}


#ifdef __PALM__
       byte *loadfile(FILE *f, int *len);
#else
static byte *loadfile(FILE *f, int *len)
{
	int c, l = 0, p = 0;
	byte *d = 0, buf[512];

	for(;;)
	{
		c = fread(buf, 1, sizeof buf, f);
		if (c <= 0) break;
		l += c;
		d = realloc(d, l);
		if (!d) return 0;
		memcpy(d+p, buf, c);
		p += c;
	}
	*len = l;
	return d;
}
#endif


static byte *inf_buf;
static int inf_pos, inf_len;

static void inflate_callback(byte b)
{
	if (inf_pos >= inf_len)
	{
		inf_len += 512;
		inf_buf = realloc(inf_buf, inf_len);
		if (!inf_buf) die("out of memory inflating file @ %d bytes\n", inf_pos);
	}
	inf_buf[inf_pos++] = b;
}

static byte *decompress(byte *data, int *len)
{
	unsigned long pos = 0;
	if (data[0] != 0x1f || data[1] != 0x8b)
		return data;
	inf_buf = 0;
	inf_pos = inf_len = 0;
/*	if (unzip(data, &pos, inflate_callback) < 0)
		return data;*/
	*len = inf_pos;
	return inf_buf;
}


extern unsigned long fgbZ_romCrc;

int rom_load()
{
	FILE *f;
	byte c, *data, *header;
	int len = 0, rlen;

#ifndef __PALM__
	//if (strcmp(romfile, "-")) f = fopen(romfile, "rb");
	//else f = stdin;
	//if (!f) die("cannot open rom file: %s\n", romfile);

	//data = loadfile(f, &len);
	//header = data = decompress(data, &len);
	
	f = fopen(romfile, "rb");
	if (!f) die("cannot open rom file: %s\n", romfile);
    header = data = loadfile(f, &len);	
#else
       header = data = loadfile(f, &len);

       fgbZ_romCrc = crc32(0,NULL,0);
       fgbZ_romCrc = crc32(fgbZ_romCrc,data,len);
#endif

    memcpy(rom.name, header+0x0134, 16);
	if (rom.name[14] & 0x80) rom.name[14] = 0;
	if (rom.name[15] & 0x80) rom.name[15] = 0;
	rom.name[16] = 0;

	c = header[0x0147];
	mbc.type = mbc_table[c];
	mbc.batt = (batt_table[c] && !nobatt) || forcebatt;
	rtc.batt = rtc_table[c];
	mbc.romsize = romsize_table[header[0x0148]];
	mbc.ramsize = ramsize_table[header[0x0149]];

	if (!mbc.romsize) die("unknown ROM size %02X\n", header[0x0148]);
	if (!mbc.ramsize) die("unknown SRAM size %02X\n", header[0x0149]);

	rlen = 16384 * mbc.romsize;
       rom.bank =
#ifndef __PALM__
	(void *)data;
    //   realloc(data, rlen);
	//if (rlen > len) memset(rom.bank[0]+len, 0xff, rlen - len);
#else
       (void *)data;
#endif
	ram.sbank = malloc(8192 * mbc.ramsize);

       initmem(ram.sbank, 8192 * mbc.ramsize);
       initmem(ram.ibank, 4096 * 8);

	mbc.rombank = 1;
	mbc.rambank = 0;

	c = header[0x0143];
	hw.cgb = ((c == 0x80) || (c == 0xc0)) && !forcedmg;
	hw.gba = (hw.cgb && gbamode);

#ifndef __PALM__
	//if (strcmp(romfile, "-")) fclose(f);
	fclose(f);
#endif

	return 0;
}

extern int sram_load();
extern int sram_save();

extern void state_save(int n);
extern void state_load(int n);


void rtc_save()
{
	FILE *f;
	if (!rtc.batt) return;
	if (!(f = fopen(rtcfile, "wb"))) return;
	rtc_save_internal(f);
	fclose(f);
}

void rtc_load()
{
	FILE *f;
	if (!rtc.batt) return;
	if (!(f = fopen(rtcfile, "r"))) return;
	rtc_load_internal(f);
	fclose(f);
}


void loader_unload()
{
#if 0 //fndef __PALM__
	sram_save();
	if (romfile) free(romfile);
	if (sramfile) free(sramfile);
	if (saveprefix) free(saveprefix);
#endif
	if (rom.bank) free(rom.bank);
	if (ram.sbank) free(ram.sbank);
	romfile = sramfile = saveprefix = 0;
	rom.bank = 0;
	ram.sbank = 0;
	mbc.type = mbc.romsize = mbc.ramsize = mbc.batt = 0;
}

static char *base(char *s)
{
	char *p;
	p = (char*)strrchr(s, '/');
	if (p) return p+1;
	return s;
}

static char *ldup(char *s)
{
	int i;
	char *n, *p;
	p = n = (char*)malloc(strlen(s));
	for (i = 0; s[i]; i++) if (isalnum(s[i])) *(p++) = tolower(s[i]);
	*p = 0;
	return n;
}

static void cleanup()
{
//	sram_save();
	rtc_save();
	/* IDEA - if error, write emergency savestate..? */
}

void loader_init(char *s)
{
	char *name, *p;

#ifndef __PALM__
//       sys_checkdir(savedir, 1); /* needs to be writable */
#endif
	sprintf(rtcfile, "%s.rtc", s);
	romfile = s;
	rom_load();

#if 0 //ndef __PALM__
	vid_settitle(rom.name);
	if (savename && *savename)
	{
		if (savename[0] == '-' && savename[1] == 0)
			name = ldup(rom.name);
		else name = strdup(savename);
	}
	else if (romfile && *base(romfile) && strcmp(romfile, "-"))
	{
		name = strdup(base(romfile));
		p = (char*)strchr(name, '.');
		if (p) *p = 0;
	}
	else name = ldup(rom.name);

	saveprefix = ljz_malloc(strlen(savedir) + strlen(name) + 2);
	sprintf(saveprefix, "%s/%s", savedir, name);

	sramfile = ljz_malloc(strlen(saveprefix) + 5);
	strcpy(sramfile, saveprefix);
	strcat(sramfile, ".sav");

	rtcfile = ljz_malloc(strlen(saveprefix) + 5);
	strcpy(rtcfile, saveprefix);
	strcat(rtcfile, ".rtc");

#endif
//       sram_load();
#ifndef __PALM__
		//init rtc
		/*if (rtc)*/ {
	    	time_t ltime;
	    	struct tm *Tm;
			unsigned long i;
			//february is always 28,....
			unsigned long month_len[12]={31,28,31,30,31,30,31,31,30,31,30,31};

	    	ltime=time(NULL);
	    	Tm=localtime(&ltime);

			rtc.d=(int)Tm->tm_wday;

			for (i=0;i<Tm->tm_mon;i++) {
				rtc.d+=month_len[i];
			}

			rtc.h=(int)Tm->tm_hour;
			rtc.m=(int)Tm->tm_min;
			rtc.s=(int)Tm->tm_sec;
			rtc.t=(int)0;

	/*		{
				char tmp[16];
				sprintf(tmp,"%d-%d-%d-%d-%d",rtc.d,rtc.h,rtc.m,rtc.s,rtc.t);
				menu_inform(tmp);
			}*/
		}
		
	rtc_load();


	//atexit(cleanup);
#else
	if (rtc.batt)
	{
/*		struct tm *localTime;
		time_t long_time;
		//Get the system time
		time(&long_time);
		localTime = localtime(&long_time);
	//date is an ofs from 01 Jan 1900 00:00:00
		rtc.d=localTime->tm_yday;
		rtc.h=localTime->tm_hour;
		rtc.m=localTime->tm_min;
		rtc.s=localTime->tm_sec;*/
		rtc.t=0;
		rtc.stop=0;
		rtc.carry=0;

		while (rtc.t >= 60) rtc.t -= 60;
		while (rtc.s >= 60) rtc.s -= 60;
		while (rtc.m >= 60) rtc.m -= 60;
		while (rtc.h >= 24) rtc.h -= 24;
		while (rtc.d >= 365) rtc.d -= 365;
		rtc.stop &= 1;
		rtc.carry &= 1;
		//if (rt) rt = (time(0) - rt) * 60;
		//if (syncrtc) while (rt-- > 0) rtc_tick();
	}
#endif
}

rcvar_t loader_exports[] =
{
	RCV_STRING("savedir", &savedir),
	RCV_STRING("savename", &savename),
	RCV_INT("saveslot", &saveslot),
	RCV_BOOL("forcebatt", &forcebatt),
	RCV_BOOL("nobatt", &nobatt),
	RCV_BOOL("forcedmg", &forcedmg),
	RCV_BOOL("gbamode", &gbamode),
	RCV_INT("memfill", &memfill),
	RCV_INT("memrand", &memrand),
	RCV_END
};









