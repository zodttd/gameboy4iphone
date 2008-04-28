//GoodGBX  - 1.020.0.dat
#ifndef h_Z_good_h
#define h_Z_good_h

#define DATROMS (2995+3501)
#define GBC_ROMS 2995

typedef struct { int crc; int size; char name[255]; } t_goodgbx_entry;

#define _GOODGBX_DATFILE_ "0:/PALM/Programs/LJP/goodgbx.zip"

extern t_goodgbx_entry *MyDat;

int DAT_LookFor(int CRC32); 

#endif
