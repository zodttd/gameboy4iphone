//-------------------- CHN sound lib

#include "gpmm.h"

/* gpsoundbuf - Implements a ring buffer for sound mixing on the GamePark 32
 *
 * Copyright (c)2002 Christian Nowak <chnowak@web.de>
 *
 * Use it as you want without any obligations or restrictions,
 * but a notification via eMail would be nice.
 *
 */

typedef struct GPSOUNDBUF
  {
    PCM_SR freq;              /* Taken from gpmm.h */
    PCM_BIT format;           /* Taken from gpmm.h */
    unsigned int samples;     /* Buffer length (in samples) */
    void * userdata;          /* Userdata which gets passed to the callback function */
    void (*callback)(         /* Callback function (just like in SDL) */
          void * userdata,    /* GPSOUNDBUF.userdata */
          unsigned char * stream,        /* Pointer to the buffer which needs to be refilled */
          int len);           /* Length of the buffer in bytes */
    unsigned int pollfreq;    /* Frequency of the timer interrupt which polls the playing position
                               * recommended value: 2*(playingfreq in Hz/GPSOUNDBUF.samples) */
    unsigned int samplesize;  /* Size of one sample (8bit mono->1, 16bit stereo->4) - don't touch this */
  } GPSOUNDBUF;


int GpSoundBufStart ( GPSOUNDBUF * );
void GpSoundBufStop ( void );


/* Global variables */
unsigned int frame;
unsigned int * soundPos;
volatile int idx_buf;
unsigned int shiftVal;
void * buffer;
GPSOUNDBUF soundBuf;






void soundtimer(void)
{
    unsigned int t = (((unsigned int)(*soundPos) - (unsigned int)buffer)>>shiftVal) >= soundBuf.samples ? 1 : 0;
    if (t!=frame)
    {
       unsigned int offs = ((frame==1) ? (soundBuf.samples<<shiftVal) : 0);
       soundBuf.callback(soundBuf.userdata,(u8*)((unsigned int)buffer+offs), soundBuf.samples<<shiftVal);
       frame = t;
    }
}









/* This routine gets called by the timer interrupt and
 * polls the current playing position within the buffer.
 */

int playing=0;

int GpSoundBufStart ( GPSOUNDBUF * sb )
  {
    frame = 0;

    if(playing) return 0;
    playing=1;

    /* Copy the structure */
    memcpy ( &soundBuf, sb, sizeof(GPSOUNDBUF) );

    /* Calculate size of a single sample in bytes
     * and a corresponding shift value
     */
    shiftVal = 0;
    switch (soundBuf.freq)
      {
        case PCM_S11:
        case PCM_S22:
        case PCM_S44:
          shiftVal++;
          break;
      }
    if (soundBuf.format == PCM_16BIT)  shiftVal++;

    soundBuf.samplesize = 1<<shiftVal;

    /* Allocate memory for the playing buffer */
    buffer = gm_malloc ( soundBuf.samplesize*soundBuf.samples*2 );
    gm_memset ( buffer, 0, soundBuf.samplesize*soundBuf.samples*2 );

    GpPcmInit(soundBuf.freq,PCM_8BIT); //22,8

    /* Set timer interrupt #0 */
#ifndef GP32_ADS
    GpTimerOptSet ( 0, soundBuf.pollfreq, 0, soundtimer);
    GpTimerSet ( 0 );

    /* Start playing */
    GpPcmPlay ( (unsigned short*)buffer, soundBuf.samples*soundBuf.samplesize*2, 1 );
    GpPcmLock ( (unsigned short*)buffer, (int*)&idx_buf, (unsigned int*)&soundPos );
#endif

#ifdef GP32_ADS
//    installVBlank( 200 );   // Install VBlank on rasterline 200
    GpTimerOptSet ( 0, soundBuf.pollfreq, 0, soundtimer );
    GpTimerSet ( 0 );

    GpPcmPlay ( (unsigned short*)buffer, soundBuf.samples*soundBuf.samplesize*2, 1 );
    GpPcmLock ( (unsigned short*)buffer, (int*)&idx_buf, (unsigned int*)&soundPos );
#endif

/*
    Setup_SND();
    smpstart=(unsigned char*)buffer;
    smpend=smpstart+soundBuf.samples*soundBuf.samplesize*2;
    smpptr=smpstart;
    playnextchunk();
*/

    return 0;
  }


void GpSoundBufStop ( void )
{
if(!playing) return;
playing=0;

#ifndef GP32_ADS
    GpPcmStop();
    GpPcmRemove ( (unsigned short*)buffer );
    GpTimerKill ( 0 );
#endif

#ifdef GP32_ADS
    GpPcmStop();
    GpPcmRemove ( (unsigned short*)buffer );

    GpTimerKill ( 0 );
//    uninstallVBlank();
#endif
   
    gm_free ( buffer );
}






GPSOUNDBUF sndbuf;


//-------------------------------------------------------------------------
