//bugs: fskip impares causan perdida de setcolors en emulacion



#include "palmdep.h"
#include "ljPRsc.h"
#include "App_Cst.h"

#include "Z\Z_good.h"

#include "palmport.h"

#include "Z\resource.h"
#include "Z\bg.h"

#include "Z\Z_window.h"

#include "Z\Z_good.h"


//#include "stdio.h"
//#include "stdlib.h"

#include "zlib.h"
#include "unzip.h"
/*************
/*tinus*/
#include "filters.h"
/*********/

#include "font.h"
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


#include "PNOMain.h"
//#include "zodiac.h"

//#include "stat.h"

/*****************/
#include "LJZmenu.h"
/**************************/

extern UInt32 mStopEmulation;
extern UInt32 device_type;

UInt16 *fgbZ_zodvram;

uint16 *fgbZ_curRenderingScreenPtr;
uint32 fgbZ_curRenderingScreenIndex;
uint32 fgbZ_enabledSound,fgbZ_sndfreq;
uint32 fgbZ_renderingmode,fgbZ_smoothmode;
uint32 fgbZ_padState;
uint32 fgbZ_mGBPal=0;
int32 fgbZ_frameskipped,fgbZ_frameskip,fgbZ_fastforward;
int32 fgbZ_frameBeginTime;
int32 fgbZ_speedpct;
int32 fgbZ_autofireA,fgbZ_autofireB;
int32 fgbZ_flipping;
int32 fgbZ_framecount;


char CurrentROMFile[256];
char fgbZ_curVolume;
char shortrom_filename[256];

unsigned long fgbZ_romCrc;

int emu_running;
int framelimit_delay;

void state_save(int n);
int state_load(int n,int only_check);


char str_month[12][4]=
{"Jan","Feb","Mar","Apr","May","Jun",
"Jul","Aug","Sep","Oct","Nov","Dec"};

#define BLACK_COLOR 255


char g_string[255];
char szRomName[255];

extern UInt32 ljp_isLowRes,ljp_confirmOW,ljp_confirmSL;
extern char ljp_RomPath[256];

// Assertion Support
static void
assert_check(int error, const char* message)
{
    if (error) {
        ErrDisplayFileLineMsg("PalmOS", (UInt16) error, message);
    }
}

#define xassert(error) {assert_check(error, #error);}

static void showError(const char* aWhat, Err aError) {
    ErrDisplayFileLineMsg("oink", (UInt16) aError, aWhat);
}


#include "SoundMgr.h"
static SndStreamRef snd_stream=NULL;
Int32 snd_stream_vol;
typedef struct {uint32 __r9;uint32 __r10;} reg_sys_t;
static reg_sys_t reg_sys;

void fgbZ_pause(int pause);

int Z_KeyGet(void)
{
	return fgbZ_padState;
}

void emu_resettimer(void) {
	fgbZ_frameBeginTime=GetTickCount()+17*100/fgbZ_speedpct;
}

void fgbZ_togglesound(void) {
	char tmp[64];
	int i;
	
	fgbZ_pause(1);
	
	fgbZ_enabledSound++;
	if (fgbZ_enabledSound>2) fgbZ_enabledSound=0;
	
	if (fgbZ_enabledSound==0) strcpy(tmp,"Sound : OFF");
	else if (fgbZ_enabledSound==1) strcpy(tmp,"Sound : SIM");
	else if (fgbZ_enabledSound==2) strcpy(tmp,"Sound : ON");

	menu_message(tmp);
	
	LJP_WAIT(300);
	emu_resettimer();
	
	fgbZ_pause(0);
}

char fgbZ_reset(void) {
	fgbZ_pause(1);
	
  	if (menu_confirm("Reset GB ?")) emu_reset();
  	
  	emu_resettimer();
  	
  	fgbZ_pause(0);
}

void fgbZ_quicksave(void) {
	fgbZ_pause(1);
	
	if (CONFIRM_SAVE) state_save(0);
	emu_resettimer();
	ClearScreen();
	
	fgbZ_pause(0);
}

void fgbZ_quickload(void) {
	fgbZ_pause(1);
	
	if (state_load(0,1))
	{
		if (CONFIRM_LOAD) state_load(0,0);
		emu_resettimer();
		ClearScreen();
	}
	
	fgbZ_pause(0);
}

int init_video(void)
{
	//Init vid				
	signed long val1,val2,val;
	
	
	fgbZ_curRenderingScreenPtr=(uint16*)ljz_malloc(256*144*2);
	memset(fgbZ_curRenderingScreenPtr,0,256*144*2);
	FtrGet(CREATORID,FTR_OPT_SCREENMODE_ID,(unsigned long*)&val1);
	FtrGet(CREATORID,FTR_OPT_SMOOTHING_ID,(unsigned long*)&val2);
	fgbZ_renderingmode=val1;
	fgbZ_smoothmode=val2;
	
/*	if (val1==1) fgbZ_renderingmode=(val2?2:1);
	else
	if (val1==2) fgbZ_renderingmode=(val2?4:3);
	else
	fgbZ_renderingmode=(val2?6:5);*/
		

	fgbZ_fastforward=0;
	
	fgbZ_frameskipped=0;
	
	 FtrGet(CREATORID,FTR_OPT_FSKIP_ID,(unsigned long*)&val);     
	fgbZ_frameskip=val;
	
	FtrGet(CREATORID,FTR_OPT_SOUNDVOLUME_ID,(unsigned long*)&val); 
	snd_stream_vol=val;
	if (snd_stream_vol<0) snd_stream_vol=0;
	if (snd_stream_vol>128) snd_stream_vol=128;
	snd_stream_vol*=16;  

//tinnus
	filters_init();	    
    
	return 0;
}

struct fb fb;

int	 dummy_align1;
byte dummy_hq2x_stuff1[160*3*2];
byte vidram[160*144*2];
byte dummy_hq2x_stuff2[160*3*2];

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
	fgbZ_framecount++;
}

int tick=0;
int frames_rendered=0,frames_displayed=0; 
char fps_string[64];

void fgbZ_asmfastrender(byte *screen,byte *source,UInt32 offset_pitch)
{
	__asm{
		stmfd r13!,{r3-r12}
	
		mov	  r3,#144
		bic	  r0,r0,#3
		bic	  r1,r1,#3

	 loopY:	
		ldmia r1!,{r4-r12}
		stmia r0!,{r4-r12}
		
		ldmia r1!,{r4-r12}
		stmia r0!,{r4-r12}
		
		ldmia r1!,{r4-r12}
		stmia r0!,{r4-r12}
		
		ldmia r1!,{r4-r12}
		stmia r0!,{r4-r12}
		
		ldmia r1!,{r4-r12}
		stmia r0!,{r4-r12}
		
		ldmia r1!,{r4-r12}
		stmia r0!,{r4-r12}
		
		ldmia r1!,{r4-r12}
		stmia r0!,{r4-r12}
		
		ldmia r1!,{r4-r12}
		stmia r0!,{r4-r12}
		
		ldmia r1!,{r5-r12}
		stmia r0!,{r5-r12}
		
		add	  r0,r0,r2 //(256-160) * 2
		
		subs  r3,r3,#1
		bne   loopY
		
		ldmfd r13!,{r3-r12}	
		}	
}

void fgbZ_asmfastrenderx2(byte *screen,byte *source,UInt32 offset_pitch)
{
	__asm{
	
#define asm_mac() \
		mov	  r5,r4,lsr #16;\
		mov	  r4,r4,lsl #16;\
		orr	  r5,r5,r5,lsl #16;\
		orr	  r4,r4,r4,lsr #16;\
		mov	  r7,r6,lsr #16;\
		mov	  r6,r6,lsl #16;\
		orr	  r7,r7,r7,lsl #16;\
		orr	  r6,r6,r6,lsr #16;\
		mov	  r9,r8,lsr #16;\
		mov	  r8,r8,lsl #16;\
		orr	  r9,r9,r9,lsl #16;\
		orr	  r8,r8,r8,lsr #16;\
		mov	  r11,r10,lsr #16;\
		mov	  r10,r10,lsl #16;\
		orr	  r11,r11,r11,lsl #16;\
		orr	  r10,r10,r10,lsr #16;	
	
		stmfd r13!,{r3-r12}
	
		mov	  r3,#144
		bic	  r0,r0,#3
		bic	  r1,r1,#3
		add	  r12,r0,r2
		mov	  r2,r2,lsl #1
		sub	  r2,r2,#640
	 loopY:	
		ldmia r1!,{r4,r6,r8,r10}  //read 4*4/2=8pixels				
		asm_mac()		
		stmia r0!,{r4-r11}
		stmia r12!,{r4-r11}
		ldmia r1!,{r4,r6,r8,r10}  
		asm_mac()
		stmia r0!,{r4-r11}
		stmia r12!,{r4-r11}
		ldmia r1!,{r4,r6,r8,r10}  
		asm_mac()
		stmia r0!,{r4-r11}
		stmia r12!,{r4-r11}
		ldmia r1!,{r4,r6,r8,r10}  
		asm_mac()
		stmia r0!,{r4-r11}
		stmia r12!,{r4-r11}
		ldmia r1!,{r4,r6,r8,r10}  
		asm_mac()
		stmia r0!,{r4-r11}
		stmia r12!,{r4-r11}
		ldmia r1!,{r4,r6,r8,r10}  
		asm_mac()
		stmia r0!,{r4-r11}
		stmia r12!,{r4-r11}
		ldmia r1!,{r4,r6,r8,r10}  
		asm_mac()
		stmia r0!,{r4-r11}
		stmia r12!,{r4-r11}
		ldmia r1!,{r4,r6,r8,r10}  
		asm_mac()
		stmia r0!,{r4-r11}
		stmia r12!,{r4-r11}
		ldmia r1!,{r4,r6,r8,r10}  
		asm_mac()
		stmia r0!,{r4-r11}
		stmia r12!,{r4-r11}
		ldmia r1!,{r4,r6,r8,r10}  
		asm_mac()
		stmia r0!,{r4-r11}
		stmia r12!,{r4-r11}
		
		ldmia r1!,{r4,r6,r8,r10}  
		asm_mac()
		stmia r0!,{r4-r11}
		stmia r12!,{r4-r11}
		ldmia r1!,{r4,r6,r8,r10}  
		asm_mac()
		stmia r0!,{r4-r11}
		stmia r12!,{r4-r11}
		ldmia r1!,{r4,r6,r8,r10}  
		asm_mac()
		stmia r0!,{r4-r11}
		stmia r12!,{r4-r11}
		ldmia r1!,{r4,r6,r8,r10}  
		asm_mac()
		stmia r0!,{r4-r11}
		stmia r12!,{r4-r11}
		ldmia r1!,{r4,r6,r8,r10}  
		asm_mac()
		stmia r0!,{r4-r11}
		stmia r12!,{r4-r11}
		ldmia r1!,{r4,r6,r8,r10}  
		asm_mac()
		stmia r0!,{r4-r11}
		stmia r12!,{r4-r11}
		ldmia r1!,{r4,r6,r8,r10}  
		asm_mac()
		stmia r0!,{r4-r11}
		stmia r12!,{r4-r11}
		ldmia r1!,{r4,r6,r8,r10}  
		asm_mac()
		stmia r0!,{r4-r11}
		stmia r12!,{r4-r11}
		ldmia r1!,{r4,r6,r8,r10}  
		asm_mac()
		stmia r0!,{r4-r11}
		stmia r12!,{r4-r11}
		ldmia r1!,{r4,r6,r8,r10}  
		asm_mac()
		stmia r0!,{r4-r11}
		stmia r12!,{r4-r11}
		
#undef asm_mac		
		
		add	  r0,r0,r2 //(256-160) * 2
		add	  r12,r12,r2 //(256-160) * 2
		
		subs  r3,r3,#1
		bne   loopY
		
		ldmfd r13!,{r3-r12}	
		}	
}


int Do_Nothing(void)
{
	int j=0;
	j+=1;
	return j;
}


void vid_end(void)
{
	if (fb.enabled)
	{	
		UInt16 *fline, *tline;
		UInt16 *f, *t, *tline2, *t2;
		int x,y;
		Int32 offset;

		fline = (UInt16*)vidram;
		
/*		for ( y = 0; y < 144; y++ ) 
		{
	      f = fline;
	      t = tline;
	      for ( x = 0; x < 160; x++ ) 
	      {
			*t++ = *f++;
	      }
	      fline += 160;
	      tline += display_pitch;
	    }*/
	  	switch (fgbZ_renderingmode)
	  	{
	  		case 3:
	  		case 1:
	  			offset = ( ((display_height-288-(fgbZ_renderingmode<2?BELTBAR_SIZE:0))>>1) * display_pitch ) + ((display_width-320)>>1);
				if (offset < 0) offset = 0; else if (offset >= (display_height * display_pitch)) offset = (display_height - 1) * display_pitch + ((display_width-320)>>1);
	  			tline = ((UInt16*) g_pnobridge -> vram) + offset;
	  			switch (fgbZ_smoothmode&_SMOOTH_MASK_){	  			
		  			case 8:fgbZ_filterrender((UInt8*)tline,(UInt8*)fline,(UInt32)((display_pitch)<<1),160*2,SCANLINES_TV,160,144,320,288);
		  			break;
		  			case 7:fgbZ_filterrender((UInt8*)tline,(UInt8*)fline,(UInt32)((display_pitch)<<1),160*2,SCALE_2XSAI,160,144,320,288);
		  			break;
		  			case 6:fgbZ_filterrender((UInt8*)tline,(UInt8*)fline,(UInt32)((display_pitch)<<1),160*2,SUPER_EAGLE,160,144,320,288);
		  			break;
			  		case 5:fgbZ_filterrender((UInt8*)tline,(UInt8*)fline,(UInt32)((display_pitch)<<1),160*2,SUPER_2XSAI,160,144,320,288);
		  			break;
				  	case 4:fgbZ_filterrender((UInt8*)tline,(UInt8*)fline,(UInt32)((display_pitch)<<1),160*2,_2XSAI,160,144,320,288);	  			
	  				break;
				  	case 3:fgbZ_filterrender((UInt8*)tline,(UInt8*)fline,(UInt32)((display_pitch)<<1),160*2,HQ2X,160,144,320,288);
		  			break;	
		  			case 2:fgbZ_filterrender((UInt8*)tline,(UInt8*)fline,(UInt32)((display_pitch)<<1),160*2,LQ2X,160,144,320,288);
		  			break;
			  		case 1:fgbZ_filterrender((UInt8*)tline,(UInt8*)fline,(UInt32)((display_pitch)<<1),160*2,ADVMAME2X,160,144,320,288);
		  			break;
			  		case 0:
			  		
			  			if (device_type&BF_ZODIAC){	
				  			if (fgbZ_smoothmode&_ZOD_SMOOTHING_) {
							tline = ((UInt16*) fgbZ_zodvram);				
							fgbZ_asmfastrender((uint8*)tline,(uint8*)fline,(480-160)<<1);
							stubzodiacblit(3,160,144,6);
							break;
						}
						else if (!fgbZ_smoothmode) {
							tline = ((UInt16*) fgbZ_zodvram);				
							fgbZ_asmfastrender((uint8*)tline,(uint8*)fline,(480-160)<<1);
							stubzodiacblit(3,160,144,2);
							break;
						}
			  		}
			  		else fgbZ_asmfastrenderx2((UInt8*)tline,(UInt8*)fline,(UInt32)((display_pitch)<<1));
					break;
				}
				break;
			case 2:
	  		case 0:
	  			offset = ( ((display_height-144-(fgbZ_renderingmode<2?BELTBAR_SIZE:0))>>1) * display_pitch ) + ((display_width-160)>>1);
				if (offset < 0) offset = 0; else if (offset >= (display_height * display_pitch)) offset = (display_height - 1) * display_pitch + ((display_width-160)>>1);
				tline = ((UInt16*) g_pnobridge -> vram) + offset;
			    fgbZ_asmfastrender((UInt8*)tline,(UInt8*)fline,(UInt32)((display_pitch-160)<<1));
			    break;
			case 4:			
				if (device_type&BF_ZODIAC){					
					if (fgbZ_smoothmode&_ZOD_SMOOTHING_) {
						tline = ((UInt16*) fgbZ_zodvram);
						fgbZ_asmfastrender((uint8*)tline,(uint8*)fline,(480-160)<<1);
						stubzodiacblit(3,160,144,5);
						break;
					}
					else if (!fgbZ_smoothmode) {
						tline = ((UInt16*) fgbZ_zodvram);
						fgbZ_asmfastrender((uint8*)tline,(uint8*)fline,(480-160)<<1);
						stubzodiacblit(3,160,144,1);
						break;
					}
						
				}
				if (device_type&BF_HIRESPLUS)
				{
					offset = ( ((display_height-320)>>1) * display_pitch ) + ((display_width-480)>>1);
					if (offset < 0) offset = 0; else if (offset >= (display_height * display_pitch)) offset = (display_height - 1) * display_pitch + ((display_width-480)>>1);
					tline = ((UInt16*) g_pnobridge -> vram) + offset;
					if (fgbZ_smoothmode) fgbZ_filterrender((UInt8*)tline,(UInt8*)fline,(UInt32)((display_pitch)<<1),160*2,SCALE_2XSAI,160,144,480,320/*-BELTBAR_SIZE*/);
				}
				else
				{
					Sys_PrintF2("Wrong Screenmode!!!");
				}
				break;
		}
	}
	
	frames_displayed++;	
	fgbZ_frameBeginTime+=17*100/fgbZ_speedpct;	//60Hz				
	
	if ((fgbZ_frameskip==AUTO_FSKIP)&&(!fgbZ_fastforward))
	{
		int i,haswaited=0;
		
		//do avoid issue when Tick is looping
		if ((int32)(fgbZ_frameBeginTime-GetTickCount())>=(int32)(10*17))		
			fgbZ_frameBeginTime=GetTickCount()+17;				
		while (GetTickCount()<=fgbZ_frameBeginTime)  haswaited=1;
//		i=fgbZ_frameBeginTime-GetTickCount()+1;
//        if (i>0) {haswaited=1; SysTaskDelay(i);}
		
		if (haswaited||(fgbZ_frameskipped>=MAX_FRAME_SKIPPED)) 
		{
			fb.enabled=1;
			fgbZ_frameskipped=0;
		}
		else 
		{
			fb.enabled=0;
			fgbZ_frameskipped++;
		}
	}
	else
	{	
		if (fgbZ_frameskip==SKEEZIX_FSKIP) fgbZ_frameskipped=SKEEZIX_FSKIP;	
		fgbZ_frameskipped++;
		
		if (fgbZ_fastforward)
		{
			if (fgbZ_frameskipped>FGBZ_TURBOFSKIP)
			{
				fgbZ_frameskipped=0;
				fb.enabled=1;
			}
			else fb.enabled=0;
		}
		else
		{					
			if (fgbZ_frameskipped>fgbZ_frameskip)
			{
				fgbZ_frameskipped=0;
				fb.enabled=1;
				int i,haswaited;					
				
				//do avoid issue when Tick is looping				
				if ((int32)(fgbZ_frameBeginTime-GetTickCount())>=(int32)(10*17))				
					fgbZ_frameBeginTime=GetTickCount()+17;
				while (GetTickCount()<=fgbZ_frameBeginTime) haswaited=1;
//				i=fgbZ_frameBeginTime-GetTickCount()+1;
//		        if (i>0) {SysTaskDelay(i);}
	 		   			   										
			}
			else 
			{
				int i;
				fb.enabled=0;				
				if ((int32)(fgbZ_frameBeginTime-GetTickCount())>=(int32)(10*17))				
					fgbZ_frameBeginTime=GetTickCount()+17;
				if (fgbZ_enabledSound==2)
				{
					while (GetTickCount()<=fgbZ_frameBeginTime) ;
//					i=fgbZ_frameBeginTime-GetTickCount()+1;
//			        if (i>0) {SysTaskDelay(i);}
				}
			}
		}
	}
	
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

void ev_poll(void)
{
 int Key=Z_KeyGet();

 hw.pad=0;

 if(Key&KEY_UP)     hw.pad|=PAD_UP;

 if(Key&KEY_DOWN)   hw.pad|=PAD_DOWN;

 if(Key&KEY_LEFT)   hw.pad|=PAD_LEFT;

 if(Key&KEY_RIGHT)  hw.pad|=PAD_RIGHT;

 if(Key&KEY_A) 
 {
	 switch (fgbZ_autofireA)
	 {
  		case 0:hw.pad|=PAD_A; break;
  		case 1:
  	  		if ((fgbZ_framecount&31)>15) hw.pad|=PAD_A;
  	  		break;
  		case 2:
  	  		if ((fgbZ_framecount&15)>7) hw.pad|=PAD_A;
  	  		break;
  		case 3:
  	  		if ((fgbZ_framecount&7)>3) hw.pad|=PAD_A;
  	  		break;  		   
  	 }
  }  		
    
 if(Key&KEY_B)
 {
 	 switch (fgbZ_autofireB)
	 {
  		case 0:hw.pad|=PAD_B; break;
  		case 1:
  	  		if ((fgbZ_framecount&31)>15) hw.pad|=PAD_B;
  	  		break;
  		case 2:
  	  		if ((fgbZ_framecount&15)>7) hw.pad|=PAD_B;
  	  		break;
  		case 3:
  	  		if ((fgbZ_framecount&7)>3) hw.pad|=PAD_B;
  	  		break;  		   
  	 }
 }
 if(Key&KEY_SELECT) hw.pad|=PAD_SELECT;
 if(Key&KEY_START)  hw.pad|=PAD_START;

 pad_refresh();


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





static Err fgbZ_sndCallBack (void *userData,SndStreamRef stream, void *bufferZ,UInt32 frameCountZ)
{
	__asm 
	{
		stmfd  r13!,{r9,r10}
		ldr	   r9,[r0]
		ldr	   r10,[r0,#4]
	}	
	
	
	
	sound_mix(bufferZ,frameCountZ);
	__asm 
	{
		ldmfd  r13!,{r9,r10}
	}
	return errNone;
}



void pcm_init(void)
{
   __asm 
  {
  	ldr  r0, = reg_sys
  	add	 r0,r0,r10
  	str	 r9,[r0]
  	str	 r10,[r0,#4]
  } 
  /*if (fgbZ_enabledSound) */



       pcm.hz = fgbZ_sndfreq;
       pcm.stereo = 1;
       pcm.len = 512;//22050/60;
       pcm.buf = NULL;//ljz_malloc(pcm.len);
	   pcm.pos = 0;
	   
  
	   
       //memset(pcm.buf, 0, pcm.len);

/*       sndbuf.freq=PCM_M22; 
       sndbuf.format=PCM_8BIT; 
       sndbuf.samples=22050/60;
       sndbuf.userdata=NULL;
       sndbuf.callback=audio_callback;
       sndbuf.pollfreq=100;*/
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
	fgbZ_pause(1);
}


extern char *romfile;



byte *loadfile(FILE *fp, int *len)
{
	byte *d; 
	long romsize;
	char str[256];
	FILE *f;
	char gb_file[256];
	  char ext[5];
	  int	err_code;
	  unzFile zip_file = 0;    
	  unz_file_info unzinfo;
	  char *pext;
	 char *p,*pp;	
	 
	pext = (char*)strchr(romfile, '.');
	while (p = (char *)strchr(pext+1, '.'))
	{
       	pext=p;
    }
	
  	if (pext) strncpy(ext, pext, sizeof(ext));
  	strlwr(ext);
  	

  	if (StrCompare(ext, ".zip") == 0)  
  	{  	  	
	  	zip_file = unzOpen(romfile);			                              
        if (!zip_file) return NULL;                                        
        unzGoToFirstFile (zip_file);                    
        for (;;) 
        {
        	if (unzGetCurrentFileInfo(zip_file, &unzinfo, gb_file, sizeof(gb_file), NULL, NULL, NULL, NULL) != UNZ_OK) return NULL;

         	    strlwr(gb_file); 
                p = (char *)strchr(gb_file, '.');
                while (pp = (char *)strchr(p+1, '.'))
                {
                	p=pp;
                }
                if (strcmp(p, ".gb") == 0) break;
                if (strcmp(p, ".gbc") == 0) break;
                if (strcmp(p, ".sgb") == 0) break;
                if (unzGoToNextFile(zip_file) != UNZ_OK) return NULL;
         }         
         unzOpenCurrentFile (zip_file);

	 	romsize = unzinfo.uncompressed_size;	 	 	 
		/*d=(uint8 *)ljz_malloc(romsize);*/
		FtrPtrNew(CREATORID, FTR_ROM_ID, romsize , (void **)&d);
		
		/*unzReadCurrentFile(zip_file, d, romsize);         */				
		if (d)
		{
			uint8 *ptmp;
			ptmp=(uint8*)ljz_malloc(65536);
			uint32 l=romsize;
			uint32 o=0;
			while (l)
			{
				if (l>65536) 
			 	{
			 		unzReadCurrentFile(zip_file,(void*)(ptmp), 65536);
			 		DmWrite(d,o,ptmp,65536);
			 		l-=65536;
			 		o+=65536;
			 	}
			 	else		 	
			 	{
		 			unzReadCurrentFile(zip_file,(void*)(ptmp), l);
		 			DmWrite(d,o,ptmp,l);
			 		l=0;		 		
			 	}
			 }
			 ljz_free(ptmp);
		}						
	    unzCloseCurrentFile (zip_file);
	    unzClose (zip_file);  	
	}
	else
	{

	    f=fopen(romfile,"rb");		
  		if (!f) {  	
	  		//GpTextOut(NULL,&gpDraw[nflip],160,10,(char*)"File access error",0xe0);
	  		//	DrawMessage("error at opening",1);
	  		menu_message("error at opening");menu_waitPen(NULL,NULL,1);
  			return NULL;
		}  	
		fseek(f,0,SEEK_END); //bug in palmos until now...
	  	romsize=ftell(f);
	  	fseek(f,0,SEEK_SET);		
  	
	  	//d=(byte *)ljz_malloc(romsize);
	  	FtrPtrNew(CREATORID, FTR_ROM_ID, romsize , (void **)&d);
	  	if (d)
	  	{
		  	/* Read file data */
    	    uint8 *ptmp;
			ptmp=(uint8*)ljz_malloc(65536);
			int32 l,FileSize=0;
			while (1)
			{
			 	l=fread (ptmp, 1, 65536, f);
			 	if (l>=0)
			 	{		 		
				 	DmWrite(d,FileSize,ptmp,l);
				 	FileSize+=l;
				}			 	
			 	if (l<65536) break;
			}
			ljz_free(ptmp);	
		}	  		  	
  		else
	  	{  		  		
			fclose(f);
  			return NULL;
	  	}		
		fclose(f);			
		
	}
	*len=romsize;		
	return d;
}

int sram_load()
{
	if (!mbc.batt) return -1;
	/* Consider sram loaded at this point, even if file doesn't exist */
	ram.loaded = 1;
	
	char savename[255],result[256];
	byte i=0,j=255,k=255,l=255;
	char chaine[256];
	FILE *f;
  
	i=0;
	j=255;
	k=-1;
	while (CurrentROMFile[i]!=0) 
	{
		if (CurrentROMFile[i]=='.') j=i;
		if (CurrentROMFile[i]=='/') k=i;
		if (CurrentROMFile[i]=='\\') k=i;
		i++;
	  }
	if (j==255) j=i;  //pas d'extension	  
	memcpy(result,CurrentROMFile+k+1,j+1-k-1);    
	result[j+1-k-1]=0;
	strcat(result,"srm");
	sprintf(savename,"%02d:%s%s", fgbZ_curVolume, FGBZ_SAVESDIR, result);
/*
	strcpy(savename,FGBZ_SAVESDIR);
	strcat(savename,result);
	savename[0]=fgbZ_curVolume+'0';
*/
	f=fopen(savename,"rb");
	if (f)
	{
		fread((void *)&ram.sbank[0],8192*mbc.ramsize,1,f);
		fclose(f);
//		DEBUGS("sram loaded");
	}
	else
	{
		//sprintf(chaine,"Cannot open %s for loading\n",savename);   
//		DEBUGS("cannot open");
//		DEBUGS(savename);
//		DrawMessage(chaine,1);
		//menu_message(chaine);menu_waitPen(NULL,NULL,1);
	}
	
	return 0;
}

int sram_save()
{
	/* If we crash before we ever loaded sram, DO NOT SAVE! */
    if (!mbc.batt || !ram.loaded || !mbc.ramsize)  return -1;	
    
	if (!mbc.batt) return -1;
	/* Consider sram loaded at this point, even if file doesn't exist */
	ram.loaded = 1;
	
	char savename[255],result[256];
	byte i=0,j=255,k=255,l=255;
	char chaine[256];
	FILE *f;
  
	i=0;
	j=255;
	k=-1;
	while (CurrentROMFile[i]!=0) 
	{
		if (CurrentROMFile[i]=='.') j=i;
		if (CurrentROMFile[i]=='/') k=i;
		if (CurrentROMFile[i]=='\\') k=i;
		i++;
	  }
	if (j==255) j=i;  //pas d'extension	  
	memcpy(result,CurrentROMFile+k+1,j+1-k-1);    
	result[j+1-k-1]=0;
	strcat(result,"srm");
	sprintf(savename,"%02d:%s%s", fgbZ_curVolume, FGBZ_SAVESDIR, result);
/*
	strcpy(savename,FGBZ_SAVESDIR);
	strcat(savename,result);
	savename[0]=fgbZ_curVolume+'0';
*/
	f=fopen(savename,"w+b");
	if (f)
	{
		fwrite((void *)&ram.sbank[0],8192*mbc.ramsize,1,f);
		fclose(f);
//		DEBUGS("sram saved");
	}
	else
	{
		//sprintf(chaine,"Cannot open %s for saving\n",savename);   
		strcpy(chaine,"Cannot open ");
		strcat(chaine,savename);
		strcat(chaine," for saving\n");

		menu_message(chaine);menu_waitPen(NULL,NULL,1);
//		DrawMessage(chaine,1);

	}
	
	return 0;    

}

void GetStateFileName(int n,char *str)
{
  char result[256];
  byte i=0,j=255,k=255,l=255;

  
  
  i=0;
  j=255;
  k=-1;
  while (CurrentROMFile[i]!=0) 
  {
	if (CurrentROMFile[i]=='.') j=i;
	if (CurrentROMFile[i]=='/') k=i;
	if (CurrentROMFile[i]=='\\') k=i;
	i++;
  }
  if (j==255) j=i;  //pas d'extension	  
  memcpy(result,CurrentROMFile+k+1,j+1-k-1);    
  result[j+1-k-1]=0;
  if (n>=0)
  {  	
	  strcat(result,"ss");
      sprintf(str,"%02d:%s%s%d", fgbZ_curVolume, FGBZ_SAVESDIR, result);
/*
//	  sprintf(str,"%s%s%d",FGBZ_SAVESDIR,result,n);
		strcpy(str,FGBZ_SAVESDIR);
		strcat(str,result);
		int lstr=strlen(str);
		str[lstr++]=n+48;
		str[lstr]=0;
*/
  } 
  else 
  {
  	strcat(result,"srm");
	sprintf(str,"%02d:%s%s", fgbZ_curVolume, FGBZ_SAVESDIR, result);
/*
  	//sprintf(str,"%s%s",FGBZ_SAVESDIR,result);
  	strcpy(str,FGBZ_SAVESDIR);
	strcat(str,result);
*/
  }
//  str[0]=fgbZ_curVolume+'0';
}

void state_save(int n)
{	
  char savename[255];
  FILE *f;
 char chaine[256];  
  GetStateFileName(n,savename);
	
//	DEBUGS("saving state to");
//	DEBUGS(savename);
  f=fopen(savename,"w+b");
  if (f) 
  {
 	savestate(f);
	fclose(f);
  }
  else 
  {
    //sprintf(chaine,"Cannot open %s for saving\n",savename);   
    strcpy(chaine,"Cannot open ");
    strcat(chaine,savename);
    strcat(chaine," for saving\n");
//	DrawMessage(chaine,1);
	menu_message(chaine);menu_waitPen(NULL,NULL,1);
  }
}


int state_load(int n,int only_check)
{
  char savename[255];   
  FILE *f;
 char chaine[256];  
  GetStateFileName(n,savename);
   	
  f=fopen(savename,"rb");
  if (f) 
  {
  	if (only_check) 
  	{
	  	fclose(f);
  		return 1;
  	}
 	loadstate(f);
	fclose(f);
  }
  else 
  {
	if (only_check) return 0;  	
    //sprintf(chaine,"Cannot open %s for loading\n",savename);   
    strcpy(chaine,"Cannot open ");
    strcat(chaine,savename);
    strcat(chaine," for loading\n");
	//DrawMessage(chaine,1);
	menu_message(chaine);menu_waitPen(NULL,NULL,1);
  }
  
  vram_dirty();
  pal_dirty();
  sound_dirty();
  mem_updatemap();
  
  return 1;
}



int fgbZ_loadrom(void)
{
	int val;
	FtrGet(CREATORID,FTR_OPT_SOUND_ID,(unsigned long*)&val);  
	fgbZ_enabledSound=val;
	
	FtrGet(CREATORID,FTR_OPT_SOUNDFREQ_ID,(unsigned long*)&val);  
	if (val==1) fgbZ_sndfreq=22050;
	else if (val==2) fgbZ_sndfreq=11025;
	else fgbZ_sndfreq=44100;		
	
	fgbZ_speedpct=100;
	fgbZ_autofireA=0;
	fgbZ_autofireB=0;
	
	fgbZ_framecount=0;

	init_video();

	vid_init();
	
	

	pcm_init();

	char *ftrptr;
    
      {//handle categories
    	char cat[32];
	  	FtrGet(CREATORID,FTR_OPT_CATEGORY_ID ,(unsigned long*)&ftrptr);
	  	strcpy(cat,ftrptr);
  		if ( StrCaselessCompare(cat,"uNaRcHiVeD") != 0) {
  			strcat(ljp_RomPath,cat);
	  		strcat(ljp_RomPath,"/");
	  	}
	  	//FtrPtrFree(CREATORID, FTR_OPT_CATEGORY_ID);
  	}
    
    FtrGet(CREATORID,FTR_ROM_FILENAME_ID ,(unsigned long*)&ftrptr);
    
    fgbZ_curVolume = StrAToI(ftrptr);
    StrPrintF(CurrentROMFile, "%02d:%s", fgbZ_curVolume, ljp_RomPath);
    
    ftrptr += OFFSET_ROMPATH;
	strcpy(shortrom_filename, ftrptr);
    strcat(CurrentROMFile, ftrptr);
//    FtrPtrFree(CREATORID, FTR_ROM_FILENAME_ID);
    
//    char strload[128];
//    strcpy(strload,"Loading : ");
//    strcat(strload,shortrom_filename);
    
    menu_message(shortrom_filename/*strload*/);

	loader_init(CurrentROMFile);
	
    emu_running=1;

	emu_reset();
    frames_rendered=frames_displayed=0;
    framelimit_delay=0;

	fgbZ_frameBeginTime=GetTickCount();    

	emu_run();		

	fgbZ_pause(0);
	
    return 0; 
}

void fgbZ_MainLoop()
{
	emu_doframe();
	
	//process key
	int key=Z_KeyGet();
	if (key&KEY_QUICKLOAD)
	{
		if (state_load(0,1))
		{
			if (CONFIRM_LOAD) state_load(0,0);
			fgbZ_frameBeginTime=GetTickCount()+17*100/fgbZ_speedpct;
			 ClearScreen();
		}
	}
	if (key&KEY_QUICKSAVE)
	{
		if (CONFIRM_SAVE) state_save(0);
		fgbZ_frameBeginTime=GetTickCount()+17*100/fgbZ_speedpct;
		 ClearScreen();
	}
	if (key&KEY_TURBO)
	{		
		fgbZ_fastforward=1;		
	}
	else 
	if (fgbZ_fastforward)
	{
		fgbZ_fastforward=0;
		fgbZ_frameBeginTime=GetTickCount()+17*100/fgbZ_speedpct;
	}
}

void fgbZ_end()
{
//	if (!FrmAlert(FrmAlertSaveState)) state_save(0);
	sram_save();

	pcm_close();
    loader_unload();
    FUX0R_PAL();

    //Err error=FtrPtrFree(CREATORID, FTR_ROM_ID);

    
    ljz_free(fgbZ_curRenderingScreenPtr);
    
    int val;

	FtrSet(CREATORID, FTR_OPT_SMOOTHING_ID,fgbZ_smoothmode);	
	FtrSet(CREATORID, FTR_OPT_SOUND_ID,fgbZ_enabledSound);
	switch (fgbZ_sndfreq)
	{
		case 11025:val=2;break;
		case 22050:val=1;break;
		default:val=0;break;
	}
	FtrSet(CREATORID, FTR_OPT_SOUNDFREQ_ID,val);
	
	FtrSet(CREATORID, FTR_OPT_SCREENMODE_ID,fgbZ_renderingmode);
	
	FtrSet(CREATORID, FTR_OPT_FSKIP_ID,fgbZ_frameskip);	
	
	FtrSet(CREATORID,FTR_OPT_SOUNDVOLUME_ID,snd_stream_vol/16); 
	
	ljz_std_showstat();
}

void fgbZ_changefskip(void)
{
	char tmp[64];
	int i;
	

	fgbZ_frameskip++;
	if (fgbZ_frameskip>SKEEZIX_FSKIP) fgbZ_frameskip=0;
	
	
	if (fgbZ_frameskip==AUTO_FSKIP) strcpy(tmp,"Frameskip : AUTO");
	else if (fgbZ_frameskip==SKEEZIX_FSKIP) strcpy(tmp,"Frameskip : Skeezix");
	else 
	{
		char tmp2[16];
		strcpy(tmp,"Frameskip : ");
		StrIToA(tmp2,fgbZ_frameskip);
		strcat(tmp,tmp2);
	}

	menu_message(tmp);
	
	i=GetTickCount();
	while ((GetTickCount()-i)<300)
	{
	}
	fgbZ_frameBeginTime=GetTickCount()+17*100/fgbZ_speedpct;
}
void fgbZ_changerendering(void)
{

	fgbZ_renderingmode++;
	
	if (fgbZ_renderingmode>3) fgbZ_renderingmode=0;
	if (fgbZ_renderingmode==2) {
		stub_fullscreen(1);
		LJP_HANDLE_EVENT(fgbZ_pause)	
	}
	if (fgbZ_renderingmode==0) {
		stub_fullscreen(0);
		LJP_HANDLE_EVENT(fgbZ_pause)
	}
	menu_message("Changing screen mode");
	fgbZ_frameBeginTime=GetTickCount();
	while ((GetTickCount()-fgbZ_frameBeginTime)<300){
	}
	ClearScreen();
	fgbZ_frameBeginTime=GetTickCount()+17*100/fgbZ_speedpct;

}
void fgbZ_updatevpad(unsigned int val)
{
	fgbZ_padState=val;	

}

void fgbZ_pause(int pause)
{
	if (pause)
	{
		if (snd_stream) stub_SndStreamGetVolume(snd_stream,&snd_stream_vol);  
		if (fgbZ_enabledSound==2) 
		{
			stub_SndStreamStop(snd_stream);
			stub_SndStreamDelete(snd_stream);
			snd_stream=NULL;
		}
	}
	else
	{
		fgbZ_frameBeginTime=GetTickCount()+17*100/fgbZ_speedpct;
		if (fgbZ_enabledSound==2) 
		{
			stub_SndStreamCreate(&snd_stream,sndOutput,fgbZ_sndfreq,/*sndInt16Little*/sndUInt8,/*sndStereo*/sndStereo,fgbZ_sndCallBack,&reg_sys,512,1);
			stub_SndStreamStart(snd_stream);		
			stub_SndStreamSetVolume(snd_stream,snd_stream_vol);
		}
	}
}


void *sys_timer() {}
int   sys_elapsed(void) {}
void  sys_sleep(int us) {}
void  sys_checkdir(char *path, int wr){}
void  sys_initpath(char *exe){}
void  sys_sanitize(char *s){}


void Cls(int col,int page)
{
	int i;
	uint16 *p=fgbZ_curRenderingScreenPtr;
	for (i=256*240;i;i--)
		*p++=col;
}



int fgbZ_ShowMenuGFXConfig(uint8 *savescr)
{
  uint8 *scr;
  int x,y;
  int redraw;  
  int result;
  int scrmode,smoothing,fskipval,hwsmooth;
  RectangleType *item_rect;
  item_rect=(RectangleType*)ljz_malloc(24*sizeof(RectangleType));
  char tmp[16];
  

	
  scrmode=fgbZ_renderingmode;
  smoothing=fgbZ_smoothmode&_SMOOTH_MASK_;
  hwsmooth=fgbZ_smoothmode&_ZOD_SMOOTHING_;
	
  fskipval=fgbZ_frameskip;

	    
  redraw=1;         
  for (;;)
  {  
  	  if (redraw)
  	  {  	  	
		GUI_REDRAW_BG(0)		
		
		menu_button(180,20,(char*)"BACK",6,&item_rect[0],2);
	  
	  	menu_button(10,80,(char*)"FRAMESKIP",0,&item_rect[1],2);	  
	  	menu_bar(150,80,fskipval,11,&item_rect[2]);
	  	if (fskipval==AUTO_FSKIP) strcpy(tmp,"AUTO");
	  	else if (fskipval==SKEEZIX_FSKIP) strcpy(tmp,"SKZX");
	  	else StrIToA(tmp,fskipval);
	  	menu_messageRaw(260,80,tmp);
	  
	  	menu_button(10,120,(char*)"ZOOM",0,&item_rect[3],2);	  	
	  	menu_button(150,120,(char*)"1",((scrmode==0)||(scrmode==2)?2:1),&item_rect[4],0);
	  	menu_button(200,120,(char*)"2",((scrmode==1)||(scrmode==3)?2:1),&item_rect[5],0);
	  	menu_button(250,120,(char*)"WIDE",(scrmode==4?2:1),&item_rect[6],0);
	  	
	  	menu_button(10,155,(char*)"FULLSCREEN",0,&item_rect[7],2);
	  	menu_button(150,155,(char*)"OFF",(scrmode&2?1:2),&item_rect[8],0);
	  	menu_button(200,155,(char*)"ON",(scrmode&2?2:1),&item_rect[9],0);	  	  	  
	  
	  	menu_button(10,190,(char*)"SMOOTHING",0,&item_rect[10],2);
	  	menu_button(150,190,(char*)"OFF",((smoothing==0)&&(!hwsmooth)?2:1),&item_rect[11],0);	  		  	
	  	menu_button(200,190,(char*)"AdvMAME2x",(smoothing==1?2:1),&item_rect[12],4);
	  	menu_button(2,218,(char*)"LQ2X",(smoothing==2?2:1),&item_rect[13],4);	  	  	  
	  	menu_button(68,218,(char*)"HQ2X",(smoothing==3?2:1),&item_rect[14],4);
	  	menu_button(136,218,(char*)"2xSaI",(smoothing==4?2:1),&item_rect[15],4);
	  	menu_button(202,218,(char*)"Super 2xSaI",(smoothing==5?2:1),&item_rect[16],4);
	  	menu_button(2,246,(char*)"Super Eagle",(smoothing==6?2:1),&item_rect[17],4);
		menu_button(115,246,(char*)"Scale 2xSaI",(smoothing==7?2:1),&item_rect[18],4);
		menu_button(225,246,(char*)"SLs",(smoothing==8?2:1),&item_rect[23],4);
		menu_button(265,246,(char*)"HW",(hwsmooth?2:1),&item_rect[22],4);
	  	
	  	menu_button(10,280,(char*)"FLIPPING",0,&item_rect[19],2);
	  	menu_button(150,280,(char*)"OFF",(fgbZ_flipping?1:2),&item_rect[20],0);
	  	menu_button(200,280,(char*)"ON",(fgbZ_flipping?2:1),&item_rect[21],0);
	  	
	  	if (scr) ljp_WinScreenUnlock();
	
  	  }
	  redraw=menu_waitPen(&x,&y,0);	  
	  if (redraw==2) {mStopEmulation=1; break;}
	  if (redraw) x=y=-1;
	  
	  
	  //BACK	  
	  if (TEST_PEN_MENU(x,y,item_rect[0])) {result=0;break;}
	  //FSKIP
	  if (TEST_PEN_MENU(x,y,item_rect[2])) {
	  	fskipval=(x-10-item_rect[2].topLeft.x)*11/100;
	  	if (fskipval<0) fskipval=0;
	  	if (fskipval>11) fskipval=11;
	  	redraw=1;
	  }
	  //SCREEN MODES
	  //SCREEN MODES
	  if (TEST_PEN_MENU(x,y,item_rect[4])) {
		  scrmode&=~4;
	  	  scrmode&=~1;
	  	  redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[5])) {
		  scrmode&=~4;
	  	  scrmode|=1;
	  	  redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[6])) {
	  	  scrmode=4;
	  	  redraw=1;
	  }
	  //SCREEN MODES WIN/FULL
	  if (TEST_PEN_MENU(x,y,item_rect[8])) {
	  	  scrmode&=~(2|4);
	  	  redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[9])) {
	  	  if (scrmode<2) scrmode|=2;
	  	  redraw=1;
	  }
	  //SMOOTHING
	  if (TEST_PEN_MENU(x,y,item_rect[11])) {
	  	  smoothing=0;hwsmooth=0;
	  	  redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[12])) {
	  	  smoothing=1;hwsmooth=0;
	  	  redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[13])) {
	  	  smoothing=2;hwsmooth=0;
	  	  redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[14])) {
	  	  smoothing=3;hwsmooth=0;
	  	  redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[15])) {
	  	  smoothing=4;hwsmooth=0;
	  	  redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[16])) {
	  	  smoothing=5;
	  	  hwsmooth=0;
	  	  redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[17])) {
	  	  smoothing=6;
	  	  hwsmooth=0;
	  	  redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[18])) {
	  	  smoothing=7;
	  	  hwsmooth=0;
	  	  redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[23])) {
	  	  smoothing=8;
	  	  hwsmooth=0;
	  	  redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[22])) {
	  	  smoothing=0;
	  	  hwsmooth=_ZOD_SMOOTHING_;
	  	  redraw=1;
	  }
	  //FLIPPING
	  if (TEST_PEN_MENU(x,y,item_rect[20])) {
	  	  fgbZ_flipping=0;
	  	  redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[21])) {
	  	  fgbZ_flipping=1;
	  	  redraw=1;
	  }
	  if (mStopEmulation) break;
  }  
  
  fgbZ_frameskip=fskipval;

  fgbZ_renderingmode=scrmode;
  fgbZ_smoothmode=smoothing|hwsmooth;
	
  if (!mStopEmulation) menu_waitPen(NULL,NULL,2);
  
  ljz_free(item_rect);
  return result;
}



int fgbZ_ShowMenuSNDConfig(uint8 *savescr)
{
  uint8 *scr;
  int x,y;
  int redraw;  
  int result;
  int vol;
  int bassboost;
  RectangleType *item_rect;
  char tmp[16];
  item_rect=(RectangleType*)ljz_malloc(15*sizeof(RectangleType));
  
  vol=snd_stream_vol/16;	    
  redraw=1;         
  for (;;)
  {  
  	  if (redraw)
  	  {
		GUI_REDRAW_BG(0)  	
  		
  	  	  	  	  	  	
	  menu_button(180,20,(char*)"BACK",6,&item_rect[0],2);	  
	  menu_button(10,80,(char*)"SOUND",0,&item_rect[1],2);
	  menu_button(150,80,(char*)"OFF",(fgbZ_enabledSound==0?2:1),&item_rect[2],0);
	  menu_button(200,80,(char*)"SIM",(fgbZ_enabledSound==1?2:1),&item_rect[3],0);	  
	  menu_button(250,80,(char*)"ON",(fgbZ_enabledSound==2?2:1),&item_rect[4],0);	 
	  
	  menu_button(10,120,(char*)"VOLUME",0,&item_rect[5],2);	  
	  menu_bar(150,120,vol,128,&item_rect[6]);
	  sprintf(tmp,"%d%",vol*100/64);	  	
	  	menu_messageRaw(270,120,tmp);
		if (scr) ljp_WinScreenUnlock();	  
  	  }
	  redraw=menu_waitPen(&x,&y,0);	  
	  if (redraw==2) {mStopEmulation=1; break;}
	  if (redraw) x=y=-1;
	  
	  
	  //BACK	  
	  if (TEST_PEN_MENU(x,y,item_rect[0])) {result=0;break;}	  
	  //SOUND OFF
	  if (TEST_PEN_MENU(x,y,item_rect[2])) 
	  {
		  fgbZ_enabledSound=0;		  
		  redraw=1;
	  }
	  //SOUND SIM
	  if (TEST_PEN_MENU(x,y,item_rect[3])) 
	  {
	  	  fgbZ_enabledSound=1;	  	  
	  	  redraw=1;
	  }
	  //SOUND ON
	  if (TEST_PEN_MENU(x,y,item_rect[4])) 
	  {
		  fgbZ_enabledSound=2;
		  redraw=1;
	  }
	  
	  //VOLUME
	  if (TEST_PEN_MENU(x,y,item_rect[6])) 
	  {
	  	vol=(x-item_rect[6].topLeft.x-10)*128/100;
	  	if (vol<0) vol=0;
	  	if (vol>128) vol=128;
	  	redraw=1;
	  }
	  if (mStopEmulation) break;
  }  

	snd_stream_vol=vol*16;
  
  if (!mStopEmulation) menu_waitPen(NULL,NULL,2);
  
  ljz_free(item_rect);
  return result;
}


int fgbZ_ShowMenuOTHERConfig(uint8 *savescr)
{
  uint8 *scr;
  int x,y;
  int redraw;  
  int result;
  RectangleType *item_rect;
  char tmp[16];
  item_rect=(RectangleType*)ljz_malloc(15*sizeof(RectangleType));
  
  redraw=1;         
  for (;;)
  {  
  	  if (redraw)
  	  {
		GUI_REDRAW_BG(0)  	
  		
  	  	  	  	  	  	
	  menu_button(180,20,(char*)"BACK",6,&item_rect[0],2);	  
	  menu_button(10,80,(char*)"AUTO A",0,&item_rect[1],4);
	  menu_button(100,80,(char*)"OFF",(fgbZ_autofireA==0?2:1),&item_rect[2],0);
	  menu_button(150,80,(char*)"SLW",(fgbZ_autofireA==1?2:1),&item_rect[3],0);
	  menu_button(200,80,(char*)"MED",(fgbZ_autofireA==2?2:1),&item_rect[4],0);
	  menu_button(250,80,(char*)"FST",(fgbZ_autofireA==3?2:1),&item_rect[5],0);
	  
	  menu_button(10,120,(char*)"AUTO B",0,&item_rect[6],4);
	  menu_button(100,120,(char*)"OFF",(fgbZ_autofireB==0?2:1),&item_rect[7],0);
	  menu_button(150,120,(char*)"SLW",(fgbZ_autofireB==1?2:1),&item_rect[8],0);
	  menu_button(200,120,(char*)"MED",(fgbZ_autofireB==2?2:1),&item_rect[9],0);
	  menu_button(250,120,(char*)"FST",(fgbZ_autofireB==3?2:1),&item_rect[10],0);
	  
	  menu_button(10,160,(char*)"EMU SPEED",0,&item_rect[11],4);
	  menu_bar(130,160,fgbZ_speedpct,100,&item_rect[12]);
	  StrIToA(tmp,fgbZ_speedpct);
	  strcat(tmp,"%");
    	menu_messageRaw(240,160,tmp);
	  
		if (scr) ljp_WinScreenUnlock();	  
  	  }
	  redraw=menu_waitPen(&x,&y,0);	  
	  if (redraw==2) {mStopEmulation=1; break;}
	  if (redraw) x=y=-1;
	  
	  
	  //BACK	  
	  if (TEST_PEN_MENU(x,y,item_rect[0])) {result=0;break;}	  
	  //AUTOFIRE A
	  if (TEST_PEN_MENU(x,y,item_rect[2])) 
	  {
		  fgbZ_autofireA=0;		  
		  redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[3])) 
	  {
		  fgbZ_autofireA=1;
		  redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[4])) 
	  {
		  fgbZ_autofireA=2;
		  redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[5])) 
	  {
		  fgbZ_autofireA=3;
		  redraw=1;
	  }
	  //AUTOFIRE B
	  if (TEST_PEN_MENU(x,y,item_rect[7])) 
	  {
		  fgbZ_autofireB=0;		  
		  redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[8])) 
	  {
		  fgbZ_autofireB=1;
		  redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[9])) 
	  {
		  fgbZ_autofireB=2;
		  redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[10])) 
	  {
		  fgbZ_autofireB=3;
		  redraw=1;
	  }
	  //SPEED
	  if (TEST_PEN_MENU(x,y,item_rect[12])) 
	  {
	  	fgbZ_speedpct=(x-item_rect[12].topLeft.x);
	  	if (fgbZ_speedpct>100) fgbZ_speedpct=100;
	  	if (fgbZ_speedpct<10) fgbZ_speedpct=10;
	  	redraw=1;
	  }
	  if (mStopEmulation) break;
  }  
  
  if (!mStopEmulation) menu_waitPen(NULL,NULL,2);
  
  ljz_free(item_rect);
  return result;
}




int fgbZ_ShowMenuSave(uint8 *savescr)
{
  uint8 *scr;
  int x,y;
  int redraw;  
  int result;
  RectangleType *item_rect;
  FILE *savefile;
  	  int slot[4];
  item_rect=(RectangleType*)ljz_malloc(15*sizeof(RectangleType));
  memset((char*)item_rect,0,15*sizeof(RectangleType));
	    
  redraw=1;         
  for (;;)
  {  
  	  if (redraw)
  	  {
		GUI_REDRAW_BG(0)  	
  		
  	  	  	  	  	  	
	  
  	  slot[0]=slot[1]=slot[2]=slot[3]=0;
  	  for (int i=0;i<4;i++) 
  	  	if (state_load(i,1))   	  	
  	  	{
      		char str[255];		
			UInt32 long_time;
			DateTimeType date_time;			
			slot[i]=1;			
			GetStateFileName(i,str);		
			fgetdate(str,&long_time);		
			TimSecondsToDateTime(long_time,&date_time);
			
			sprintf(str,"%02d%s%02d-%02d:%02d",
			date_time.day,str_month[date_time.month-1],date_time.year%100,date_time.hour,date_time.minute);
			if (i) 
			{
				menu_messageRaw(150,80+i*30,str);
				menu_button(10,80+i*30,"X",5,&item_rect[6+i],3);
			}
			else 
			{
				menu_messageRaw(150,200,str);
				menu_button(10,200,"X",5,&item_rect[6+i],3);
			}
	  	}	  	  	
  	  	  	  	  	  	
		  menu_button(180,20,(char*)"BACK",6,&item_rect[0],2);
		  menu_button(10,50,(char*)"SAVE",0,&item_rect[1],2);	  	  	  
		  menu_button(50,110,(char*)"SLOT 1",(slot[1]?2:3),&item_rect[2],1);
		  menu_button(50,140,(char*)"SLOT 2",(slot[2]?2:3),&item_rect[3],1);
		  menu_button(50,170,(char*)"SLOT 3",(slot[3]?2:3),&item_rect[4],1);
		  menu_button(50,200,(char*)"QUICK",(slot[0]?2:3),&item_rect[5],1);
		  if (scr) ljp_WinScreenUnlock();
  	  }
	  redraw=menu_waitPen(&x,&y,1);	  
	  if (redraw==2) {mStopEmulation=1; break;}
	  if (redraw) x=y=-1;
	  
  	  if (TEST_PEN_MENU(x,y,item_rect[6])) 
	  {
	  	if (menu_confirm("Delete QUICKSLOT?"))
	  	{
		  	char str[255];		
			GetStateFileName(0,str);
	  		remove(str);
	  	}
	  	redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[7])) 
	  {
	  	if (menu_confirm("Delete SLOT 1?"))
	  	{
		  	char str[255];		
			GetStateFileName(1,str);
	  		remove(str);
	  	}
	  	redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[8])) 
	  {
	  	if (menu_confirm("Delete SLOT 2?"))
	  	{
		  	char str[255];		
			GetStateFileName(2,str);
	  		remove(str);
	  	}
	  	redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[9])) 
	  {
	  	if (menu_confirm("Delete SLOT 3?"))
	  	{
		  	char str[255];		
			GetStateFileName(3,str);
	  		remove(str);
	  	}
	  	redraw=1;
	  }
	  
	  
	  //BACK	  
	  if (TEST_PEN_MENU(x,y,item_rect[0])) {result=0;break;}
	  //SAVE 0
	  if (TEST_PEN_MENU(x,y,item_rect[2]))
	  {
		  if (!slot[1]||(SHOW_OVERWRITE))
		  {state_save(1);result=1; break;}
		  else redraw=1;
		 
	  }
	  //SAVE 1
	  if (TEST_PEN_MENU(x,y,item_rect[3]))
	  {
		  if (!slot[2]||(SHOW_OVERWRITE))
		  {state_save(2);result=1; break;}
		  else redraw=1;
		 
	  }
	  //SAVE 2
	  if (TEST_PEN_MENU(x,y,item_rect[4]))
	  {
		  if (!slot[3]||(SHOW_OVERWRITE))
		  {state_save(3);result=1; break;}
		  else redraw=1;
		 
	  }
	  //QUICKSAVE
	  if (TEST_PEN_MENU(x,y,item_rect[5]))
	  {
		  if (!slot[0]||(SHOW_OVERWRITE))
		  {state_save(0);result=1; break;}
		  else redraw=1;
		 
	  }
	  if (mStopEmulation) break;
  }  
  ljz_free(item_rect);
  return result;
}



int fgbZ_ShowMenuLoad(uint8 *savescr)
{
  uint8 *scr;
  int x,y;
  int redraw;  
  int result;
   int slot[4];
  RectangleType *item_rect;
  item_rect=(RectangleType*)ljz_malloc(15*sizeof(RectangleType));
  memset((char*)item_rect,0,15*sizeof(RectangleType));
  
  redraw=1;         
  for (;;)
  {  
  	  if (redraw)
  	  {
		GUI_REDRAW_BG(0)  	
		
  	  FILE *savefile;
  	 
  	  slot[0]=slot[1]=slot[2]=slot[3]=0;
      for (int i=0;i<4;i++) 
      	if (state_load(i,1)) 
      	{
      		char str[255];		
			UInt32 long_time;
			DateTimeType date_time;			
			slot[i]=1;			
			GetStateFileName(i,str);		
			fgetdate(str,&long_time);		
			TimSecondsToDateTime(long_time,&date_time);
			
			sprintf(str,"%02d%s%02d-%02d:%02d",
			date_time.day,str_month[date_time.month-1],date_time.year%100,date_time.hour,date_time.minute);
			if (i) 
			{
				menu_messageRaw(150,80+i*30,str);
				menu_button(10,80+i*30,"X",5,&item_rect[6+i],3);
			}
			else 
			{
				menu_messageRaw(150,200,str);
				menu_button(10,200,"X",5,&item_rect[6+i],3);
			}
		}	
      
		  menu_button(180,20,(char*)"BACK",6,&item_rect[0],2);
		  menu_button(10,50,(char*)"LOAD",0,&item_rect[1],2);	  	  	  
		  menu_button(50,110,(char*)"SLOT 1",(slot[1]?2:3),&item_rect[2],1);
		  menu_button(50,140,(char*)"SLOT 2",(slot[2]?2:3),&item_rect[3],1);
		  menu_button(50,170,(char*)"SLOT 3",(slot[3]?2:3),&item_rect[4],1);
		  menu_button(50,200,(char*)"QUICK",(slot[0]?2:3),&item_rect[5],1);
		  if (scr) ljp_WinScreenUnlock();
  	  }
	  redraw=menu_waitPen(&x,&y,1);	  
	  if (redraw==2) {mStopEmulation=1; break;}
	  if (redraw) x=y=-1;

  	  if (TEST_PEN_MENU(x,y,item_rect[6])) 
	  {
	  	if (menu_confirm("Delete QUICKSLOT?"))
	  	{
		  	char str[255];		
			GetStateFileName(0,str);
	  		remove(str);
	  	}
	  	redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[7])) 
	  {
	  	if (menu_confirm("Delete SLOT 1?"))
	  	{
		  	char str[255];		
			GetStateFileName(1,str);
	  		remove(str);
	  	}
	  	redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[8])) 
	  {
	  	if (menu_confirm("Delete SLOT 2?"))
	  	{
		  	char str[255];		
			GetStateFileName(2,str);
	  		remove(str);
	  	}
	  	redraw=1;
	  }
	  if (TEST_PEN_MENU(x,y,item_rect[9])) 
	  {
	  	if (menu_confirm("Delete SLOT 3?"))
	  	{
		  	char str[255];		
			GetStateFileName(3,str);
	  		remove(str);
	  	}
	  	redraw=1;
	  }
	  
	  
	  //BACK	  
	  if (TEST_PEN_MENU(x,y,item_rect[0])) {result=0;break;}
	  //LOAD 0
	  if (TEST_PEN_MENU(x,y,item_rect[2]))
	  {
		  if (slot[1])
		if (state_load(1,0)) {result=1;break;}
	  }
	  //LOAD 1
	  if (TEST_PEN_MENU(x,y,item_rect[3]))
	  {
		  if (slot[2])
		if (state_load(2,0)) {result=1;break;}
	  }
	  //LOAD 2
	  if (TEST_PEN_MENU(x,y,item_rect[4]))
	  {
		  if (slot[3])
		if (state_load(3,0)) {result=1;break;}	  
	  }
	
	  //QUICKLOAD
	  if (TEST_PEN_MENU(x,y,item_rect[5]))
	  {
	  	if (slot[0])
    	if (state_load(0,0)) {result=1;break;}
	  }
	  if (mStopEmulation) break;
  }  
  ljz_free(item_rect);
  return result;
}


int fgbZ_ShowMenuConfig(uint8 *savescr)
{
  uint8 *scr;
  int x,y;
  int redraw;  
  int result;
  RectangleType *item_rect;
  item_rect=(RectangleType*)ljz_malloc(15*sizeof(RectangleType));
	    
  redraw=1;         
  for (;;)
  {  
  	  if (redraw)
  	  {
		GUI_REDRAW_BG(0)  	
  		
  	  	  	  	  	  	
	  menu_button(180,20,(char*)"BACK",6,&item_rect[0],2);	  
	  menu_button(92,100,(char*)"GFX",0,&item_rect[1],2);
	  menu_button(92,150,(char*)"SOUND",0,&item_rect[2],2);
	  menu_button(92,200,(char*)"OTHER",0,&item_rect[3],2);
	  if (scr) ljp_WinScreenUnlock();
  	  }
	  redraw=menu_waitPen(&x,&y,1);	  
	  if (redraw==2) {mStopEmulation=1; break;}
	  if (redraw) x=y=-1;
	  
	  //BACK	  
	  if (TEST_PEN_MENU(x,y,item_rect[0])) {result=0;break;}	  
	  //GFX
	  if (TEST_PEN_MENU(x,y,item_rect[1])) 
	  {
	  	fgbZ_ShowMenuGFXConfig(savescr);
		redraw=1;
	  }
	  //SOUND
	  if (TEST_PEN_MENU(x,y,item_rect[2])) 
	  {
     	fgbZ_ShowMenuSNDConfig(savescr);
		redraw=1;
	  }
	  //OTHER
	  if (TEST_PEN_MENU(x,y,item_rect[3])) 
	  {
		  fgbZ_ShowMenuOTHERConfig(savescr);
		  redraw=1;		  
	  }
	  if (mStopEmulation) break;
  }  
  ljz_free(item_rect);
  return result;
}

int fgbZ_ShowSysInfos(uint8 *savescr)
{
  uint8 *scr;
  int x,y;
  int redraw;  
  int result;
  RectangleType *item_rect,my_rect;
  item_rect=(RectangleType*)ljz_malloc(15*sizeof(RectangleType));
	    
  redraw=1;         
  for (;;)
  {  
  	  if (redraw)
  	  {
		GUI_REDRAW_BG(0)				
		
		char str[64],strb[32];
		unsigned long maxP,freeP,usedP;		
		VolumeInfoType vinfo;
		VFSVolumeSize(fgbZ_curVolume,&usedP,&maxP);		
		VFSVolumeGetLabel(fgbZ_curVolume,strb,16);strb[16]=0;
		menu_button(10,70,"Volume",0,&my_rect,4);		
		sprintf(str,"%s[%d]",strb,fgbZ_curVolume);
		menu_messageRaw(my_rect.topLeft.x+my_rect.extent.x+5,63,str);				
		sprintf(str,"Used:%dM/%dM",usedP>>20,maxP>>20);
		menu_messageRaw(my_rect.topLeft.x+my_rect.extent.x+5,81,str);				
		MemHeapFreeBytes(0,&freeP,&maxP);		
		menu_button(10,100,"RAM",6,&my_rect,4);		
		sprintf(str,"RAM %ldK/%ldK",freeP/1024,maxP/1024);
		menu_messageRaw(my_rect.topLeft.x+my_rect.extent.x+5,100,str);		
		/*TwGfxInfoType gfxinfo;
		gfxinfo.size=sizeof(gfxinfo);
		TwGfxGetInfo(fgbZ_mGfxLib,&gfxinfo);		
		menu_button(10,130,"VRAM",6,&my_rect,4);		
		sprintf(str,"Free:%d Max:%d",gfxinfo.freeAcceleratorMemory,gfxinfo.totalAcceleratorMemory);
		menu_messageRaw(my_rect.topLeft.x+my_rect.extent.x+5,130,str);*/
						
    		
		menu_button(10,160,"ROM",4,&my_rect,4);
		menu_messageRaw(my_rect.topLeft.x+my_rect.extent.x+5,160,rom.name);
		
		sprintf(str,"%dKo RAM-%dKo",mbc.romsize,mbc.ramsize);
		menu_button(10,190,"Size",4,&my_rect,4);
	    menu_messageRaw(my_rect.topLeft.x+my_rect.extent.x+5,190,str);
	    
		menu_button(10,220,"Type",7,&my_rect,4);		
		sprintf(str,"%d Bt:%d Rtc:%d CGB:%d",mbc.type,mbc.batt,rtc.batt,hw.cgb);
		menu_messageRaw(my_rect.topLeft.x+my_rect.extent.x+5,220,str);
		
		menu_button(10,250,"CRC",7,&my_rect,4);		
		sprintf(str,"%08X",fgbZ_romCrc);
		menu_messageRaw(my_rect.topLeft.x+my_rect.extent.x+5,250,str);
		
		menu_button(10,280,"GoodGB",7,&my_rect,4);		
		int i=DAT_LookFor(fgbZ_romCrc);
		if (i<0) sprintf(str,"not found");
		else sprintf(str,"%s",MyDat[i].name);
		menu_messageRaw(my_rect.topLeft.x+my_rect.extent.x+5,280,str);
		
						  	
	  	menu_button(180,20,(char*)"BACK",6,&item_rect[0],2);
		
	  
	  	if (scr) ljp_WinScreenUnlock();
  	  }
	  redraw=menu_waitPen(&x,&y,1);	  
	  if (redraw==2) {mStopEmulation=1; break;}
	  if (redraw) x=y=-1;
	  
	  //BACK	  
	  if (TEST_PEN_MENU(x,y,item_rect[0])) {result=0;break;}
	  
	  if (mStopEmulation) break;
  }  
  ljz_free(item_rect);
  return result;
}


int fgbZ_ShowMenu(void)
{
  int x,y;
  int redraw;
  int result;
  int slot0;
  uint8 *scr,*savescr,*tmpscr,*tmpdst;
  RectangleType *item_rect;
  item_rect=(RectangleType*)ljz_malloc(15*sizeof(RectangleType));  
  redraw=1;

  
  fgbZ_pause(1);
    
       
  scr=/*(uint8*)(g_pnobridge -> vram);*/ljp_WinScreenLock(winLockCopy);  
  savescr=(uint8*)ljz_malloc(display_width*display_height*2);    
  if (scr&&savescr)
  {
	  tmpscr=scr;
  	for (int yy=0;yy<display_height-(fgbZ_renderingmode<2?BELTBAR_SIZE:0);yy++)
  	{
	  for (x=0;x<display_width;x++)
	  {
  		y=((uint16*)tmpscr)[x];
	  	((uint16*)tmpscr)[x]=((y>>12)<<11)|(((y>>6)&31)<<5)|((y>>1)&15);
	  }
	  tmpscr+=display_pitch*2;
	}
	tmpscr=scr;
	tmpdst=savescr;
	for (int yy=0;yy<display_height;yy++)
  	{
	    memcpy(tmpdst,tmpscr,display_width*2);
	    tmpdst+=display_width*2;
	    tmpscr+=display_pitch*2;
	}
  }
  if (scr) ljp_WinScreenUnlock();
            
              
  for (;;)
  {  
  	if (redraw)
  	{  	
	  	slot0=state_load(0,1);    
  		
  		
		GUI_REDRAW_BG(1)  		
		
		menu_button(180,20,(char*)"BACK",6,&item_rect[0],2);
		menu_button(10,100,(char*)"SAVE",0,&item_rect[1],2);
		menu_button(180,100,(char*)"QUICK SAVE",(slot0?2:1),&item_rect[2],2);
		menu_button(10,150,(char*)"LOAD",0,&item_rect[3],2);
		menu_button(180,150,(char*)"QUICK LOAD",(slot0?2:3),&item_rect[4],2);
		menu_button(10,200,(char*)"CONFIG",0,&item_rect[5],2);
		menu_button(180,250,(char*)"LAUNCHER",4,&item_rect[6],2);
		
		menu_button(180,200,(char*)"SYS INFOS",7,&item_rect[8],2);
		
		menu_button(10,250,(char*)"RESET GB",9,&item_rect[7],2);				
		if (scr) ljp_WinScreenUnlock();				
		
  	}
	  redraw=menu_waitPen(&x,&y,1);	  
	  if (redraw==2) {mStopEmulation=1; break;}
	  if (redraw) x=y=-1;
	  //HOME
	  if (TEST_PEN_MENU(x,y,item_rect[6])) {result=1;break;}
	  //BACK
	  if (TEST_PEN_MENU(x,y,item_rect[0])) {result=0;break;}
	  //CONFIG
	  if (TEST_PEN_MENU(x,y,item_rect[5])) {fgbZ_ShowMenuConfig(savescr);redraw=1;}
	  //LOAD
	  if (TEST_PEN_MENU(x,y,item_rect[3])) {if (fgbZ_ShowMenuLoad(savescr)) {result=0;break;} redraw=1;}
	  //SAVE
	  if (TEST_PEN_MENU(x,y,item_rect[1])) {if (fgbZ_ShowMenuSave(savescr)) {result=0;break;} redraw=1;}
	  //QUICK SAVE
	  if (TEST_PEN_MENU(x,y,item_rect[2]))
	  {
		  state_save(0);
		  result=0;
		  break;
	  }
	  //QUICK LOAD
	  if (TEST_PEN_MENU(x,y,item_rect[4]))
	  {
		  if (slot0)
	    if (state_load(0,0)) {result=0;break;}
	  }
	  //RESET GB
	  if (TEST_PEN_MENU(x,y,item_rect[7])) {if (menu_confirm("Reset GB ?")) {emu_reset(); result=0;break;}redraw=1;}
	  
	  //SYS INFOS
	  if (TEST_PEN_MENU(x,y,item_rect[8])) {fgbZ_ShowSysInfos(savescr);redraw=1;}

		if (mStopEmulation) break;
	}
  ljz_free(item_rect);
  
  if (savescr) ljz_free(savescr);
            
  fgbZ_pause(0);
  return result;
}  



