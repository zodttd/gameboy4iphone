/*

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; version 2
 of the License.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef APP_IPHONE_H
#define APP_IPHONE_H

#import <CoreSurface/CoreSurface.h>
#import <AudioToolbox/AudioQueue.h>

typedef unsigned char byte;

struct app_Preferences {
    byte frameSkip;
    byte debug;
    byte canDeleteROMs;
    byte transparency;
    byte landscape;
    byte allowSuspend;
    byte scaled;
    byte muted;
    byte selectedSkin;
};

void setDefaultPreferences();
int app_SavePreferences();
int app_LoadPreferences();

/* STUBs to emulator core */

void *app_Thread_Start(void *args);
void *app_Thread_Resume(void *args);
void app_Halt(void);
void app_Resume(void);
int app_LoadROM(const char *fileName);
void app_DeleteTempState(void);
void app_SetSvsFile(char* filename);
int app_OpenSound(int samples_per_sync, int sample_rate);
void app_CloseSound(void);
void app_StopSound();
void app_StartSound();
unsigned long joystick_read();
FILE* fopen_home(char* filename, char* fileop);

extern byte IS_DEBUG;
extern byte IS_CHANGING_ORIENTATION;
extern unsigned short  *BaseAddress;
extern int __screenOrientation;
extern struct app_Preferences preferences;
extern unsigned short *videobuffer;
extern int   __emulation_run;
extern char appworkdir[512];

/* Audio Resources */
#define AUDIO_BUFFERS 24
#define AUDIO_PRECACHE 25
#define WAVE_BUFFER_SIZE 735
#define WAVE_BUFFER_BANKS 25

typedef struct AQCallbackStruct {
    AudioQueueRef queue;
    UInt32 frameCount;
    AudioQueueBufferRef mBuffers[AUDIO_BUFFERS];
    AudioStreamBasicDescription mDataFormat;
} AQCallbackStruct;

#ifndef GUI_DEBUG
#define LOGDEBUG(...) while(0){}
#else
void LOGDEBUG(const char *text, ...);
#endif

#endif
