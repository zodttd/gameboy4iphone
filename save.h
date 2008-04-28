#ifndef __SAVE_H__
#define __SAVE_H__




#ifdef IS_LITTLE_ENDIAN
#define LIL(x) (x)
#else
#define LIL(x) ((x<<24)|((x&0xff00)<<8)|((x>>8)&0xff00)|(x>>24))
#endif

#define I1(s, p) { 1, s, p }
#define I2(s, p) { 2, s, p }
#define I4(s, p) { 4, s, p }
#define R(r) I1(#r, &R_##r)
#define NOSAVE { -1, "\0\0\0\0", 0 }
#define END { 0, "\0\0\0\0", 0 }

struct svar
{
	int len;
	char key[4];
	void *ptr;
};

extern int ver;
extern int sramblock, iramblock, vramblock;
extern int hramofs, hiofs, palofs, oamofs, wavofs;

extern struct svar svars[] ;


#endif
