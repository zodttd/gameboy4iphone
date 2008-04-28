


#ifndef __DEFS_H__
#define __DEFS_H__


#define IS_LITTLE_ENDIAN

//#include <stdio.h>
//#include <math.h>
//#include <stdlib.h>

//#define INLINE static __inline

#undef byte
#undef word

#ifdef IS_LITTLE_ENDIAN
#define LO 0
#define HI 1
#else
#define LO 1
#define HI 0
#endif

//ifndef GP32
typedef unsigned char byte;
//endif

typedef unsigned char un8;
typedef unsigned short un16;
typedef unsigned int un32;

typedef signed char n8;
typedef signed short n16;
typedef signed int n32;

//ifndef GP32
typedef un16 word;
//endif
typedef word addr;





#endif

