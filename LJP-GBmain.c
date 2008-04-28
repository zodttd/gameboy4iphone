#include "palmdep.h"
#include "endianutils.h"
#include "pnobridge.h"
#include "oscompat.h"
#include "endianutils.h"
#include "ljPRsc.h"
#ifdef __PACE__  
#include "PACEInterface.h"
#else
#include "palmos5.h"
#endif


#include "PNOMain.h"


#include "palmport.h"
#include "App_Cst.h"

#include "save.h"
#include "rtc.h"

unsigned int g_translateBtn[20]=
{KEY_A,KEY_B,KEY_START,KEY_SELECT,
KEY_UP,KEY_DOWN,KEY_LEFT,KEY_RIGHT,
KEY_QUICKLOAD,KEY_QUICKSAVE,KEY_FRAMESKIP,
KEY_SCREENMODE,KEY_TURBO,KEY_PANIC,0,0,0,0,0,0};
extern unsigned int g_confinput[20];
extern unsigned int g_confinput_treo[20][3];


int fgbZ_ShowMenu(void);
void fgbZ_pause(int pause);

void ClearScreen(void);

UInt32 mStopEmulation=0;

extern UInt32 ljp_showFPS,ljp_showBeltbar,ljp_showBattMode,ljp_isLowRes,ljp_confirmOW,ljp_confirmSL;
extern UInt32 fgbZ_renderingmode;
extern UInt16 *fgbZ_zodvram;
extern UInt32 device_type;
extern char ljp_RomPath[256];

unsigned long Run()
{
	Int32 lastTick;
	UInt32 pollcpt=0;
	UInt32 keys,keyval;
    Int16 penX,penY;
    Boolean penDown;    
	//Read the key config
	UInt32 *ftrptr;
    char text[16];
    Int32 framerate = 0;
    Int32 timecpt,framecpt = 0;
	
	//Load the Rom
	mStopEmulation=fgbZ_loadrom();
	
	if (device_type & BF_ZODIAC) {
		fgbZ_zodvram=(UInt16*) stubzodiacblit ( 0,0,0,0 );
		if (!fgbZ_zodvram) menu_info("Zodiac VRAM NULL ptr!!");
	}

	ClearScreen();
	//
	//Main Loop
	
	//autoload?
	if (g_pnobridge->game_rom){
		state_load(g_pnobridge->game_rom-1,0);
	}
	
	//init rtc
	/*if (rtc)*/ {
		DateTimeType date_time;			
		UInt32 seconds,i;
		//february is always 28,....
		UInt32 month_len[12]={31,28,31,30,31,30,31,31,30,31,30,31};
		
		seconds=TimGetSeconds();
		TimSecondsToDateTime(seconds,&date_time);
		
		rtc.d=(int)date_time.day;
		
		for (i=0;i<date_time.month-1;i++) {
			rtc.d+=month_len[i];
		}
		
		rtc.h=(int)date_time.hour;
		rtc.m=(int)date_time.minute;
		rtc.s=(int)date_time.second;
		rtc.t=(int)0;
		
/*		{
			char tmp[16];
			sprintf(tmp,"%d-%d-%d-%d-%d",rtc.d,rtc.h,rtc.m,rtc.s,rtc.t);
			menu_inform(tmp);
		}*/
	}
	
	timecpt=GetTickCount();
	while (!mStopEmulation)
	{
	
   	    //Read key state
	    keys = KeyCurrentState();
	    
		keyval=0;

		//check pen
		if (!(pollcpt&3)) LJP_CHECK_MENU_ACCESS(fgbZ_ShowMenu,fgbZ_renderingmode>=2)
		
		//handle sys event
		if (!(pollcpt&31)) LJP_HANDLE_EVENT(fgbZ_pause)
		
		//avoid autoff
		//every 1024 frame => 
		if (!(pollcpt&1023)) EvtResetAutoOffTimer(); 
		
		LJP_UPDATE_KEYS
		
		fgbZ_updatevpad(keyval);
		
		if (keyval&KEY_FRAMESKIP) 
		{
			fgbZ_changefskip();
			ClearScreen();
		}
		if (keyval&KEY_SCREENMODE) 
		{
			fgbZ_changerendering();
			stub_DrawBeltBar();
			ClearScreen();
		}
		//check for panic key
		if (keyval&KEY_PANIC)
		{
			state_save(0);
			mStopEmulation=1;
			FtrSet(CREATORID, FTR_PANIC_EXIT_ID, 1);
		}
		
		pollcpt++;
		
	
		//Emulate GB
		
		fgbZ_MainLoop();
		
		framecpt++;
		if ((fgbZ_renderingmode<2) &&  ( (ljp_showFPS) || (ljp_showBeltbar) ))
		if (framecpt>=30) {
		  //FPS
		    UInt8 *vr;
			vr=(UInt8*)(g_pnobridge -> vram)/*+(g_pnobridge -> display_pitch)*2*/;		
			framecpt=0;
			framerate=30*1000/(GetTickCount()-timecpt+1);
			timecpt=GetTickCount();

			if (ljp_showBeltbar) {
				beltbar_draw(0,g_pnobridge -> display_height-BELTBAR_SIZE,g_pnobridge -> display_pitch,framerate,0,vr);
			} else if (ljp_showFPS) {
				for (int y=0;y<20;y++)
				{	
					memset(vr,0,80*2);
					vr+=(g_pnobridge -> display_pitch)*2;
				}
				
				text[0]=((framerate/100)%10)+48;
				text[1]=((framerate/10)%10)+48;
				text[2]=((framerate)%10)+48;
				text[3]=0;
				
				menu_messageRaw(5,0,text);
			}
		}
	}

	fgbZ_end();
	
	if (device_type & BF_ZODIAC) {
		stubzodiacblit ( 1,0,0,0 );
	}


	return 0;
}



/************************************/


void ClearScreen(void)
{
	UInt8 *vr;
	vr=(UInt8*)(g_pnobridge -> vram);
	for (int y=0;y<g_pnobridge -> display_height-(fgbZ_renderingmode<2?BELTBAR_SIZE:0);y++)
	{
		memset(vr,0,g_pnobridge -> display_width*2);
		vr+=(g_pnobridge -> display_pitch)*2;
	}
}
