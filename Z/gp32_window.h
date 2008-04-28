signed short Sin[256];
         int sinPos=0;


void Init_Sinus()
{
  int c;

  for(c=0;c<256;c++)
   Sin[c]=(signed short)(128.0f* (sin( (((double)c)/256.0f) *2.0f*3.1415926535f) ));
}


struct { int cur,min,max,x,y; } progress[10]; int progressnum;

void DrawPercentageBar(unsigned int x, unsigned  int y, unsigned  int current, unsigned  int min, unsigned  int max)
{
int j;

   for(j=1-2;j<=102+1;j++) Picture[(240-y+2-1)+(x+j)*240]=Picture[(240-y-3-1)+(x+j)*240]=255;
   for(j=1+0;j<1+(current*100/(max-min));j++) Picture[(240-y-1)+(x+j)*240]=Picture[(240-y-1-1)+(x+j)*240]=255;
   Picture[(240-y+1-1)+(x-2+1)*240]=Picture[(240-y-1)+(x-2+1)*240]=
   Picture[(240-y-1-1)+(x-2+1)*240]=Picture[(240-y-2-1)+(x-2+1)*240]=255;
   Picture[(240-y+1-1)+(x+102+1)*240]=Picture[(240-y-1)+(x+102+1)*240]=
   Picture[(240-y-1-1)+(x+102+1)*240]=Picture[(240-y-2-1)+(x+102+1)*240]=255;

   for(j=-2;j<=102;j++) Picture[(240-y+2)+(x+j)*240]=Picture[(240-y-3)+(x+j)*240]=254;
   for(j=0;j<(current)*100/(max-min);j++) Picture[(240-y)+(x+j)*240]=Picture[(240-y-1)+(x+j)*240]=254;
   Picture[(240-y+1)+(x-2)*240]=Picture[(240-y)+(x-2)*240]=
   Picture[(240-y-1)+(x-2)*240]=Picture[(240-y-2)+(x-2)*240]=254;
   Picture[(240-y+1)+(x+102)*240]=Picture[(240-y)+(x+102)*240]=
   Picture[(240-y-1)+(x+102)*240]=Picture[(240-y-2)+(x+102)*240]=254;
}

void DrawProgressBar(unsigned int x, unsigned  int y, unsigned int percent)
{
 DrawPercentageBar(x,y,percent,1,100);
}



#include "gp32_windups.h"

void FlipScreen(int vsync, unsigned char clearscreen)
{
  volatile long *lcdcon1 = (long *)0x14a00000;
  volatile long *lcdaddr1 = (long *)0x14a00014, *lcdaddr2 = (long *)0x14a00018, *PictureL, i;

  if(vsync||clearscreen)
   {
    //if(!MSXVersion) while(239!=(((*lcdcon1)>>18)&0x1ff));
    //if(!MSXVersion) while(319!=(((*lcdcon1)>>18)&0x1ff));
    //while(1!=(((*lcdcon1)>>18)&0x1ff));

    while(1!=(((*lcdcon1)>>18)&0x1ff));
   }

  if(!gpDrawSurfaceIndex)    {  *lcdaddr1 += 240*320/2; *lcdaddr2 += 320*240/2; } //0x1e3600; }
  else                       {  *lcdaddr1 -= 240*320/2; *lcdaddr2 -= 320*240/2; }

  Picture     = (unsigned char *)(0x0C7B4000+(320*240*gpDrawSurfaceIndex));

  if(clearscreen) 
  {
  register long v=(clearscreen<<24)|(clearscreen<<16)|(clearscreen<<8)|(clearscreen);
  PictureL = (long *)(0x0C7B4000+(320*240*(gpDrawSurfaceIndex)));

  for(i=320*240/4;i>0;i--) *(PictureL++)=v;
  }

  gpDrawSurfaceIndex ^= 1;

  PictureAlt  = (unsigned char *)(0x0C7B4000+(320*240*gpDrawSurfaceIndex));
}



int StringSinus=0,WindowSinus=0;
int StringSinusEnabled=1,WindowSinusEnabled=1;

void DrawStringMini(char *c, int x, int y, char ch)
{
 unsigned int loop,loop2,bit;
 int xbackup=x;
 int sinusbackup=StringSinus;

 if(GP32_Video==16) return;

 while(*c)
 switch(*c)
 {
 case '\n': y+=16/2;x=xbackup;c++;break;
 default:

 y+=Sin[StringSinus]/8;

 switch(GP32_Video)
 {
 case 8:  for(loop=y;loop<y+16/2;loop++)
           for(loop2=x,bit=0x80;loop2<x+8/2;loop2++,bit/=4)
            if(gfx_font[(*c)*16+(loop-y)*2]&bit) Picture[((240-loop)+loop2*240)]=ch;
           break;

 default: for(loop=y;loop<y+16;loop++) //15!
           for(loop2=x,bit=0x80;loop2<x+8;loop2++,bit/=2)
            if(gfx_font[(*c)*16+loop-y]&bit) { Picture[((240-loop)+loop2*240)*2]=0xFF; Picture[((240-loop)+loop2*240)*2+1]=0xFF; }
           break;

 }
          x+=8/2;c++;

 y-=Sin[StringSinus]/8;

 if(StringSinusEnabled) StringSinus=(StringSinus+8)%256;
 }

 if(StringSinusEnabled) StringSinus=(sinusbackup+1)%256;
}


void DrawString(char *c, int x, int y, char ch)
{
 unsigned int loop,loop2,bit;
 int xbackup=x;
 int sinusbackup=StringSinus;

 if(GP32_Video==16) return;

 while(*c)
 switch(*c)
 {
 case '\n': y+=16;x=xbackup;c++;break;
 default:

 y+=Sin[StringSinus]/8;

 switch(GP32_Video)
 {
 case 8:  for(loop=y;loop<y+16;loop++)
           for(loop2=x,bit=0x80;loop2<x+8;loop2++,bit/=2)
            if(gfx_font[(*c)*16+loop-y]&bit) Picture[(240-loop)+loop2*240]=ch;
           break;

 default: for(loop=y;loop<y+16;loop++) //15!
           for(loop2=x,bit=0x80;loop2<x+8;loop2++,bit/=2)
            if(gfx_font[(*c)*16+loop-y]&bit) { Picture[((240-loop)+loop2*240)*2]=0xFF; Picture[((240-loop)+loop2*240)*2+1]=0xFF; }
           break;

 }
          x+=8;c++;

 y-=Sin[StringSinus]/8;

 if(StringSinusEnabled) StringSinus=(StringSinus+8)%256;
 }

 if(StringSinusEnabled) StringSinus=(sinusbackup+1)%256;
}

typedef struct {
                     int x, y;
                     int w, h;
                     char *gfx;
                     int unused;
                     int frames;
                }
                Sprite;

void DrawBox(int x, int y, int w, int h, char c)
{
int loop;

if(GP32_Video==16||GP32_Video==15) return;

if(WindowSinusEnabled)
{
 x+=Sin[WindowSinus%256]/32;
 w+=Sin[WindowSinus%256]/32;

 for(loop=0;loop<w;loop++)
 {
  y+=Sin[(WindowSinus+loop)%256]/32;
  gm_memset(&Picture[((240-y)-h)+(x+loop)*240], c, h);
  y-=Sin[(WindowSinus+loop)%256]/32;
 }
 WindowSinus=(WindowSinus+1)%256;
}
else
 switch(GP32_Video)
 {
 case 8:   for(loop=0;loop<w;loop++) gm_memset(&Picture[((240-y)-h)+(x+loop)*240], c, h);
           break;

 case 16:  for(loop=0;loop<w;loop++) gm_memset(&Picture[(((240-y)-h)+(x+loop)*240)*2], 0x80, h*2);
           break;
 }
}

void DrawRect(int x, int y, int w, int h, char c)
{
if(x+w>320) w=320-x;   if(y+h>240) h=240-y;
if(x<0)  { w+=x;x=0; } if(y<0)  { h+=y;y=0; }
DrawBox(x,y,w,h,c);
}

void DrawWindow2(int x, int y, int w, int h)
{
DrawRect(x+5,y+5,w,h,255);
DrawRect(x-1,y-1,w+1,h+1,255);
DrawRect(x,y,w-1,h-1,253);
}

void DrawMessage2(char *s, int x, int y, int w, int h)
{
DrawWindow2(x,y,w,h);
DrawString(s,x+8+1,y+4,255);
DrawString(s,x+8+0,y+4+1,255);
DrawString(s,x+8+1,y+4+1,255);
DrawString(s,x+8,y+4,254);
DrawString(s,x+8+1,y+4,254);
//DrawString(s,((x+8)-(w/2))/8,((y+16)-(h/2))/8);
}

void DrawMessage3(char *s, int x, int y, int w, int h)
{
DrawWindow2(x,y,w,h);
DrawStringMini(s,x+4+1,y+4+1,255);
DrawStringMini(s,x+4,y+4,254);
//DrawString(s,((x+8)-(w/2))/8,((y+16)-(h/2))/8);
}

void DrawMessageB(char *s)
{
 int c=0,h=0,n=1,loop=0;

 while(s[loop]!='\0') if(s[loop++]=='\n') {n++; if(c>h) h=c; c=0;} else c++;

 if(h==0) h=c; else if(c>h) h=c;

 c=h*8;
 n*=16;

 DrawMessage2(s,(320-c)/2,(240-n)/2,c+8+8,n+16/2);
}

#ifdef GP32_ADS
extern int sscanf(const char * /*s*/, const char * /*format*/, ...);
#endif

void DrawMessageC(char *s)
{
 int c=0,h=0,n=1,loop=0;

 progressnum=0;

 while(s[loop]!='\0')
 {
  switch(s[loop++])
  {
  case '\n': n++; if(c>h) h=c; c=0;
             break;

  case '^':  //ctrl code
             break;

  case '#':  {

             progressnum++;

             #ifndef GP32_ADS
             s[loop+9]=s[loop+18]=s[loop+27]='\0';
             progress[progressnum].cur=strtol(&s[loop+1],NULL,16);
             progress[progressnum].min=strtol(&s[loop+10],NULL,16);
             progress[progressnum].max=strtol(&s[loop+19],NULL,16);
             s[loop+9]=s[loop+18]=s[loop+27]=',';
             #endif

             #ifdef GP32_ADS
             { int i,j,val,pow;

             #define Hex2Int(x) \
             for(val=0,pow=1,j=7;j>=0;j--,pow*=16) \
             { i=s[loop+x+j]; if(i>='a') i=i-'a'+10; else if(i>='A') i=i-'A'+10; else i=i-'0';  \
              val+=i*pow;  }

             Hex2Int(1) ;progress[progressnum].cur=val;
             Hex2Int(10);progress[progressnum].min=val;
             Hex2Int(19);progress[progressnum].max=val;
             }
             #undef Hex2Int
             #endif

//           s[loop+27]='\0';
//           sscanf(&s[loop+1],"%08x%*c%08x%*c%08x",&progress[progressnum].cur,&progress[progressnum].min,&progress[progressnum].max);
//           s[loop+27]=')';

             loop+=29; // skip #(%08x,%08x,%08x,#

             c+=22; //adjust line width with spaces

             }
             break;

  default:   c++;
  }
 }

 if(h==0) h=c; else if(c>h) h=c;

 c=h*5;
 n*=11;

 n+=11; //extra
 c+=5;  //extra

// DrawMessage3(s,(320-c)/2,(240-n)/2,c+(8+8)/2,n+(16/2)/2);
 DrawWindow2((320-c)/2,(240-n)/2,c+(8+8)/2,n+(16/2)/2);
 textOut(s,5+(320-c)/2,5+(240-n)/2,255,0);
 textOut(s,4+(320-c)/2,4+(240-n)/2,254,0);

 while(progressnum) { DrawPercentageBar(progress[progressnum].x,progress[progressnum].y,progress[progressnum].cur,progress[progressnum].min,progress[progressnum].max); progressnum--; }
}

void DrawTile(int x, int y, int width, int height, unsigned char *sprite)
{
  int w;

  for(w=0;w<width;w++)  // f   y   w   x h   f
   gm_memcpy(&Picture[((240-y)-height)+(x+w)*240],sprite+w*height,height);
}

void DrawSprite(int x, int y, int w, int h, unsigned char *sprite, char transparency)
{
  int loop,loop2;

  if(x+w>320) w=320-x;   if(y+h>240) h=240-y;
  if(x<0)  { w+=x;x=0; } if(y<0)  { h+=y;y=0; }

  for(loop=0;loop<w;loop++)  // f   y   w   x h   f
  {
   char *d=(char *)&Picture[((240-y)-h)+(x+loop)*240];
   char *s=(char *)sprite+loop*h;

   loop2=h;
   while(loop2--) { if(*s!=transparency) *d=*s; d++;s++; }
  }
}

void DrawSpriteNoClipX(int x, int y, int w, int h, unsigned char *sprite, char transparency)
{
  int loop,loop2;

  if(x<0)  { w+=x;x=0; } if(y<0)  { h+=y;y=0; }

  for(loop=0;loop<w;loop++)  // f   y   w   x h   f
  {
   char *d=(char *)&Picture[((240-y)-h)+((x+loop)%320)*240];
   char *s=(char *)sprite+loop*h;

   loop2=h;
   while(loop2--) { if(*s!=transparency) *d=*s; d++;s++; }
  }
}
