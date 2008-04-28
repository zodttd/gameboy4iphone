#define BLACK_COLOR 255




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


#include "zlib.h"
#include "unzip.h"


//#include <stdlib.h>
//#include <math.h>


#define  HOST_LITTLE_ENDIAN

#define  ZERO_LENGTH 0

/* quell stupid compiler warnings */
#ifndef UNUSED
#define  UNUSED(x)   ((x) = (x))
#endif

typedef  signed char    int8;
typedef  signed short   int16;
typedef  signed long     int32;
typedef  unsigned char  uint8;
typedef  unsigned short uint16;
typedef  unsigned int   uint32;

#ifndef __cplusplus
/*typedef enum
{
   false = 0,
   true = 1
} bool;*/
#define bool uint8 //bool
#ifndef  NULL
#define  NULL     ((void *) 0)
#endif
#endif /* !__cplusplus */

#define boolean bool

//#include <memguard.h>
//#include <log.h>


#define  ASSERT(expr)
#define  ASSERT_MSG(msg)

typedef unsigned char Uint8;
typedef unsigned short Uint16;
typedef unsigned int Uint32;
typedef signed char Sint8;
typedef signed short Sint16;
typedef signed int Sint32;

//typedef Uint8 u8;

#undef byte
#undef word
typedef uint16 word;
typedef uint8 byte;
typedef signed char offset;

/*#ifndef boolean
typedef unsigned char boolean;
#endif*/
#ifndef  BOOL
typedef int BOOL;
#endif


#define FALSE 0
#define TRUE 1
#define false 0
#define true 1




#define DelayA(); 
#define DelayB(); 


#define ONE_SECOND 1000

#define FGBZ_TURBOFSKIP 15

#define FGBZ_ROMSDIR "0:/PALM/Programs/LJP/GB/Roms/" //"gp:\\snes\\ffmq11.zip"
#define FGBZ_SAVESDIR "0:/PALM/Programs/LJP/GB/Saves/" //"gp:\\snes\\ffmq11.zip"
#define AUTO_FSKIP 10
#define SKEEZIX_FSKIP 11

#define MAX_FRAME_SKIPPED 10

#define KEY_UP    		0x00000001
#define KEY_DOWN  		0x00000002
#define KEY_LEFT  		0x00000004
#define KEY_RIGHT 		0x00000008
#define KEY_SEL   		0x00000010
#define KEY_A     		0x00000020
#define KEY_B	  		0x00000040
#define KEY_START 		0x00000080
#define KEY_SELECT		0x00000100
#define KEY_QUICKLOAD   0x00000200
#define KEY_QUICKSAVE   0x00000400
#define KEY_TURBO  		0x00000800
#define KEY_SCREENMODE	0x00001000
#define KEY_FRAMESKIP	0x00002000

#define version_emu=11003;







