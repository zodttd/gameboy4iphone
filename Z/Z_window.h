/*void DrawTile(int x, int y, int width, int height, unsigned char *sprite)
{
  int w;

  for(w=0;w<width;w++)  // f   y   w   x h   f
   memcpy(&Picture[((240-y)-height)+(x+w)*240],sprite+w*height,height);
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
*/