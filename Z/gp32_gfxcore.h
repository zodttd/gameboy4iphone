//mode 1 ok
//mode 2 ok


#define WIDTH  320
#define HEIGHT 240


static void RefreshBorder(byte Y);


/** RefreshBorder() ******************************************/
/** This function is called from RefreshLine#() to refresh  **/
/** the screen border.                                      **/
/*************************************************************/
void RefreshBorder(register byte Y)
{
/*
  if(!Y)
    memset(Picture,BGColor,WIDTH*(HEIGHT-192)/2);
  if(Y==191)
    memset(Picture+WIDTH*(HEIGHT+192)/2,BGColor,WIDTH*(HEIGHT-192)/2);
*/
}


/** Sprites() ************************************************/
/** This function is called from RefreshLine#() to refresh  **/
/** sprites in SCREENs 1-3.                                 **/
/*************************************************************/
void SpritesX(register byte Y,register byte *Line)
{
  register byte *P,C;
  register byte H,*PT,*AT;
  register unsigned int M;
  register int L,K;

  /* Assign initial values before counting */
  H=Sprites16x16? 16:8;
  C=0;M=0;L=0;
  AT=SprTab-4;

  /* Count displayed sprites */
  do
  {
    M<<=1;AT+=4;L++;    /* Iterating through SprTab      */
    K=AT[0];            /* K = sprite Y coordinate       */
    if(K==208) break;   /* Iteration terminates if Y=208 */
    if(K>256-H) K-=256; /* Y coordinate may be negative  */

    /* Mark all valid sprites with 1s, break at MAXSPRITE1 sprites */
    if((Y>K)&&(Y<=K+H)) { M|=1;C++;if(C==4) break; }
  }
  while(L<32);

  /* Draw all marked sprites */
  for(;M;M>>=1,AT-=4)
    if(M&1)
    {
      C=AT[3];                  /* C = sprite attributes */
      L=C&0x80? AT[1]-32:AT[1]; /* Sprite may be shifted left by 32 */
      C&=0x0F;                  /* C = sprite color */

      if((L<256)&&(L>-H)&&C)
      {
        K=AT[0];                /* K = sprite Y coordinate */
        if(K>256-H) K-=256;     /* Y coordinate may be negative */

        P=Line+L*240;
        PT=SprGen+((int)(H>8? AT[2]&0xFC:AT[2])<<3)+Y-K-1;

//        C=Palette[C];
        C=C;

        /* Mask 1: clip left sprite boundary */
        K=L>=0? 0x0FFFF:(0x10000>>-L)-1;
        /* Mask 2: clip right sprite boundary */
        if(L>256-H) K^=((0x00200>>(H-8))<<(L-257+H))-1;
        /* Get and clip the sprite data */
        K&=((int)PT[0]<<8)|(H>8? PT[16]:0x00);

        /* Draw left 8 pixels of the sprite */
        if(K&0xFF00)
        {
          if(K&0x8000) P[0*240]=C;if(K&0x4000) P[1*240]=C;
          if(K&0x2000) P[2*240]=C;if(K&0x1000) P[3*240]=C;
          if(K&0x0800) P[4*240]=C;if(K&0x0400) P[5*240]=C;
          if(K&0x0200) P[6*240]=C;if(K&0x0100) P[7*240]=C;
        }

        /* Draw right 8 pixels of the sprite */
        if(K&0x00FF)
        {
          if(K&0x0080) P[8*240]=C; if(K&0x0040) P[9*240]=C;
          if(K&0x0020) P[10*240]=C;if(K&0x0010) P[11*240]=C;
          if(K&0x0008) P[12*240]=C;if(K&0x0004) P[13*240]=C;
          if(K&0x0002) P[14*240]=C;if(K&0x0001) P[15*240]=C;
        }
      }
    }
}


void RefreshSprites(register byte Y, byte *Pic)
{
 SpritesX(Y,Pic);
}




/** RefreshLine0() *******************************************/
/** Refresh line Y (0..191) of SCREEN0, including sprites   **/
/** in this line.                                           **/
/*************************************************************/
void RefreshLine0(register byte Y)
{
  register byte X,K,Offset,FC,BC;
  register byte *P,*T; int i;

  P=(byte *)Picture+240-(Y+((HEIGHT-192)/2));

  if(!ScreenON) for(i=0;i<WIDTH*240;i+=240) P[i]=BGColor;
  else
  {
    BC=BGColor;
    FC=FGColor;
    T=ChrTab+(Y>>3)*40;
    Offset=Y&0x07;

    //memset(P,BC,(WIDTH-240)/2);
    P+=240*((WIDTH-240)/2);

    for(X=0;X<40;X++,P+=6*240,T++)
    {
      K=ChrGen[((int)*T<<3)+Offset];
      P[0*240]=K&0x80? FC:BC;P[1*240]=K&0x40? FC:BC;
      P[2*240]=K&0x20? FC:BC;P[3*240]=K&0x10? FC:BC;
      P[4*240]=K&0x08? FC:BC;P[5*240]=K&0x04? FC:BC;
    }

    //memset(P,BC,(WIDTH-240)/2);
  }

  RefreshBorder(Y);
}

/** RefreshLine1() *******************************************/
/** Refresh line Y (0..191) of SCREEN1, including sprites   **/
/** in this line.                                           **/
/*************************************************************/
void RefreshLine1(register byte Y)
{
  register byte X,K,Offset,FC,BC;
  register byte *P,*T; int i;

  P=(byte *)Picture+240-(Y+((HEIGHT-192)/2));

  if(!ScreenON) for(i=0;i<WIDTH*240;i+=240) P[i]=BGColor;
  else
  {
    T=ChrTab+(Y>>3)*32;
    Offset=Y&0x07;

    //memset(P,XPal[BGColor],(WIDTH-256)/2);
    P+=240*(WIDTH-256)/2;

    for(X=0;X<32;X++,P+=8*240,T++)
    {
      K=*T;
      BC=ColTab[K>>3];
      K=ChrGen[((int)K<<3)+Offset];
      FC=BC>>4;
      BC=BC&0x0F;
      P[0*240]=K&0x80? FC:BC;P[1*240]=K&0x40? FC:BC;
      P[2*240]=K&0x20? FC:BC;P[3*240]=K&0x10? FC:BC;
      P[4*240]=K&0x08? FC:BC;P[5*240]=K&0x04? FC:BC; 
      P[6*240]=K&0x02? FC:BC;P[7*240]=K&0x01? FC:BC;
    }

  //  memset(P,XPal[BGColor],(WIDTH-256)/2);
    RefreshSprites(Y,P-256*240);
  }

  RefreshBorder(Y);
}

/** RefreshLine2() *******************************************/
/** Refresh line Y (0..191) of SCREEN2, including sprites   **/
/** in this line.                                           **/
/*************************************************************/
void RefreshLine2(register byte Y)
{
  register byte X,K,FC,BC,Offset;
  register byte *P,*T,*PGT,*CLT; int i;
  register int I;

  P=(byte *)Picture+240-((HEIGHT-192)/2+Y);

  if(!ScreenON) for(i=0;i<WIDTH*240;i+=240) P[i]=BGColor;
  else
  {
    I=(int)(Y&0xC0)<<5;
    PGT=ChrGen+I;
    CLT=ColTab+I;
    T=ChrTab+(Y>>3)*32;
    Offset=Y&0x07;

    //memset(P,XPal[BGColor],(WIDTH-256)/2);
    P+=240*((WIDTH-256)/2);

    for(X=0;X<32;X++,P+=8*240,T++)
    {
      I=((int)*T<<3)+Offset;
      K=PGT[I];
      BC=CLT[I];
      FC=BC>>4;
      BC=BC&0x0F;
      P[0*240]=K&0x80? FC:BC;P[1*240]=K&0x40? FC:BC;
      P[2*240]=K&0x20? FC:BC;P[3*240]=K&0x10? FC:BC;
      P[4*240]=K&0x08? FC:BC;P[5*240]=K&0x04? FC:BC;
      P[6*240]=K&0x02? FC:BC;P[7*240]=K&0x01? FC:BC;

    }

    //memset(P,XPal[BGColor],(WIDTH-256)/2);
    RefreshSprites(Y,P-256*240);
  }    

  RefreshBorder(Y);
}

/** RefreshLine3() *******************************************/
/** Refresh line Y (0..191) of SCREEN3, including sprites   **/
/** in this line.                                           **/
/*************************************************************/
void RefreshLine3(register byte Y)
{
  register byte X,K,Offset;
  register byte *P,*T; int i;

  P=(byte *)Picture+240-(Y+(HEIGHT-192)/2);

  if(!ScreenON) for(i=0;i<WIDTH*240;i+=240) P[i]=BGColor;
  else
  {
    T=ChrTab+(Y>>3)*32;
    Offset=(Y&0x1C)>>2;

    //memset(P,XPal[BGColor],(WIDTH-256)/2);
    P+=240*((WIDTH-256)/2);

    for(X=0;X<32;X++,P+=8*240,T++)
    {
      K=ChrGen[((int)*T<<3)+Offset];
      P[0*240]=P[1*240]=P[2*240]=P[3*240]=K>>4;
      P[4*240]=P[5*240]=P[6*240]=P[7*240]=K&0x0F;
    }

    // memset(P,XPal[BGColor],(WIDTH-256)/2);
    RefreshSprites(Y,P-256*240);
  }

  RefreshBorder(Y);
}
