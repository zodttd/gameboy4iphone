
//diferencias respecto a fNES, la cabecera de archivo, el (GG)/(SMS), y el sort

unsigned long MyGameSize; unsigned char *MyGame; unsigned long MyGameNo, MyGameDatNo;

#define MAXFS 11

#define MAX_COUNT_FILE  10000                                      //maximum number of file entries in one folder
#define MAX_PATH_LEN	255					//maximum length of full path name
ERR_CODE        err;
char		g_path_curr[MAX_PATH_LEN + 1];	//current path (in full path format)
GPDIRENTRY	g_list_file[MAX_COUNT_FILE];	//list of files in current path
unsigned long g_cnt_file = 0;                                 //number of files in current path




F_HANDLE g__file;

void LoadPacked(int skip, int gz_size, int gz_crc)
{
    z_stream d_stream; /* decompression stream */
    void *temp; int i; 

    /*
    if(gz_size>64*1024)
    {
    unsigned long tt;

    temp=(void *)gm_malloc(64*1024+256);  //extra bytes

    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in   = (Bytef *)temp; //skip
    d_stream.avail_in  = (uLongf)64*1024; //-skip-8 //solo con MyGameSize quedaria ok
    d_stream.next_out  = (Bytef *)&MyGame[0];
    d_stream.avail_out = (uLongf)gz_size;

    i=inflateInit2(&d_stream, -MAX_WBITS);

    GpFileSeek(g__file,FROM_BEGIN,skip,(long *)&tt);

    do
    {
//     char s[100]; gm_sprintf(s,"%d (%d) (%d %d)",64*1024*j,ST,d_stream.avail_in,d_stream.avail_out); DrawMessageC(s);FlipScreen(1,0);DrawMessageC(s);FlipScreen(1,0); DelayA();

     GpFileRead(g__file,temp,64*1024,&tt); 

     d_stream.next_in   = (Bytef *)temp; 
     d_stream.avail_in  = (uLongf)64*1024;
    }
    while((ST=inflate(&d_stream, Z_SYNC_FLUSH))!=Z_STREAM_END);

    i=inflateEnd(&d_stream);

    MyGameSize=gz_size; //d_stream.total_out;

    gm_free(temp);
    }
    */

    {
    unsigned long tt; int lsize=((int)(gm_availablesize()/65536))*65536;

    lsize=8192; //patch for low mem (<512k), used by fGB

    temp=(void *)gm_malloc(lsize+256);  //extra bytes

    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in   = (Bytef *)temp; //skip
    d_stream.avail_in  = (uLongf)lsize; //-skip-8 //solo con MyGameSize quedaria ok
    d_stream.next_out  = (Bytef *)&MyGame[0];
    d_stream.avail_out = (uLongf)gz_size;

    i=inflateInit2(&d_stream, -MAX_WBITS);

    GpFileSeek(g__file,FROM_BEGIN,skip,(long *)&tt);

    do
    {
     GpFileRead(g__file,temp,lsize,&tt); 

     d_stream.next_in   = (Bytef *)temp; 
     d_stream.avail_in  = (uLongf)lsize;
    }
    while(inflate(&d_stream, Z_SYNC_FLUSH)!=Z_STREAM_END);

    i=inflateEnd(&d_stream);

    MyGameSize=gz_size; //d_stream.total_out;

    gm_free(temp);
    }
    
    /*
    else

    {
    temp=(void *)gm_malloc(gz_size+256);  //extra bytes

    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in   = (Bytef *)&MyGame[skip]; //skip
    d_stream.avail_in  = (uLongf)MyGameSize-skip; //-skip-8 //solo con MyGameSize quedaria ok
    d_stream.next_out  = (Bytef *)temp;
    d_stream.avail_out = (uLongf)gz_size;

    i=inflateInit2(&d_stream, -MAX_WBITS);

    i=inflate(&d_stream, Z_FINISH);

    i=inflateEnd(&d_stream);

    gm_memcpy(MyGame,temp,MyGameSize=d_stream.total_out); 
    gm_free(temp);
    }
    */

    for(CRC=i=0;i<MyGameSize;i++) GetCRC(MyGame[i]);
}

//ok
enum
{
UNK,
UNP,
GZ,
ZIP
};

typedef struct {
                char type;
                unsigned short entry; 
                char file[8+3+1+1];
                unsigned long offset;
               } ini_game; 

struct {
        int version_ini;
        int num_games_in_ini;
   ini_game game[MAX_COUNT_FILE]; //1000 roms max
        } ini;

void fs_add(char type,unsigned short entry,char *file,unsigned long offset)
{
 ini.game[ini.num_games_in_ini].type=type;
 ini.game[ini.num_games_in_ini].entry=entry;
 ini.game[ini.num_games_in_ini].offset=offset;
 gm_sprintf(ini.game[ini.num_games_in_ini].file,"%s",file);
 ini.game[ini.num_games_in_ini].file[12]='\0'; //force string

 ini.num_games_in_ini++;
}

int fs_loadgame(char *dir, char *name, unsigned long *CRC32, int entry, int force_type) 
{
 char filename[255]; F_HANDLE file; int i;
 int test=1;

 if(force_type==UNK)
 {
 test=0;
 force_type=ini.game[entry].type;
 gm_sprintf(filename,"%s\\%s",dir,ini.game[entry].file);
 }
 else gm_sprintf(filename,"%s\\%s",dir,name);  

 switch(force_type)
 {
  case GZ:  
		{
		  int skip; uLongf gz_crc,gz_size; 

  //              GpFileGetSize(filename, &MyGameSize);

		  GpFileOpen(filename, OPEN_R, &file);
               /* if(!test) GpFileRead(file, (void *)&MyGame[0], MyGameSize, &MyGameSize);
                     else*/ GpFileRead(file, (void *)&MyGame[0], 1024, &MyGameSize);
		  GpFileClose(file);

		  GpFileGetSize(filename, &MyGameSize);

		  //gzip
		  //ID1,ID2,CM,FLG,MTIME-MTIME-MTIME-MTIME,XFL,OS
		  skip=10;          //1000
		  if(MyGame[3]&4)  { skip+=4; skip+=(MyGame[12]<<8)|MyGame[13]; } //FLG.FEXTRA (4 o 2?)
                  if(MyGame[3]&8)  { /*int l=0;*/ while(MyGame[skip++]) ; /*real_filename[l++]=MyGame[skip-1]; real_filename[l]='\0';*/ } //FLG.FNAME
		  if(MyGame[3]&16) { while(MyGame[skip++]); } //FLG.FCOMMENT
		  if(MyGame[3]&2)  { skip+=2; }                  //FLG.FHCRC

		  gz_size=(MyGame[MyGameSize-1]<<24)|(MyGame[MyGameSize-2]<<16)|(MyGame[MyGameSize-3]<<8)|(MyGame[MyGameSize-4]);
		  gz_crc =(MyGame[MyGameSize-5]<<24)|(MyGame[MyGameSize-6]<<16)|(MyGame[MyGameSize-7]<<8)|(MyGame[MyGameSize-8]);

                 if(!test) { GpFileOpen(filename, OPEN_R, &g__file);
                             LoadPacked(skip,gz_size,gz_crc);
                             GpFileClose(g__file);
                           }

		  *CRC32=gz_crc;

  	  	  return 1;
		}
		

  case UNP: 
		{
		  GpFileGetSize(filename, &MyGameSize);

		  GpFileOpen(filename, OPEN_R, &file);
		  GpFileRead(file, (void *)&MyGame[0], MyGameSize, &MyGameSize);
		  GpFileClose(file);

		  for(CRC=i=0;i<MyGameSize;i++) GetCRC(MyGame[i]);

		  *CRC32=CRC;

  	 	  return 1;
		}
 }

 if(force_type>=ZIP)
 {
//  case ZIP:  //unpack_zip(dir,ini.game[n].file,offset); return 1;
//             return 0;

  {
   int skip; uLongf gz_crc,gz_size; 

//   GpFileGetSize(filename, &MyGameSize);

   GpFileOpen(filename, OPEN_R, &file);

/*   if(!test) GpFileRead(file, (void *)&MyGame[0], MyGameSize, &MyGameSize);
        else*/ GpFileRead(file, (void *)&MyGame[0], 1024, &MyGameSize);

   GpFileClose(file);

   GpFileGetSize(filename, &MyGameSize); //nuevo!

   skip=30;
   skip+=(MyGame[27]<<8)|MyGame[26]; //filename length
   skip+=(MyGame[29]<<8)|MyGame[28]; //extra fields length

   //gm_memcpy(real_filename,&MyMSXGame[30],(MyMSXGame[27]<<8)|MyMSXGame[26]);

   gz_size=(MyGame[25]<<24)|(MyGame[24]<<16)|(MyGame[23]<<8)|(MyGame[22]);
   gz_crc =(MyGame[17]<<24)|(MyGame[16]<<16)|(MyGame[15]<<8)|(MyGame[14]);

   if(!test) { GpFileOpen(filename, OPEN_R, &g__file);
               LoadPacked(skip,gz_size,gz_crc);
               GpFileClose(g__file);
             }

   *CRC32=gz_crc;

   return 1;
  }

 }

 return 0;
}

//ok
void fs_scanfile(char *dir, char *name)
{
 char filename[255]; char head[]="abcdef"; F_HANDLE file; unsigned long crc=0; int i;

 MyGameSize=0;

 gm_sprintf(filename,"%s\\%s",dir,name);

 if(GpFileOpen(filename, OPEN_R, &file)!=SM_OK) return;
 GpFileRead(file, (void *)&head[0], 4, &crc); //crc unused
 GpFileClose(file);
 if(crc==0) return;

 if(head[0]=='P'&&head[1]=='K') //zip
 {
  /*
  # = 0
  eoz = 1

  while(!eoz)
  {
  unpack #

  get_crc_zip
  if(crc_in_list) fs_add(ZIP,entry_in_list,file,0);
   else fs_add(ZIP,file en zip,file,0);

  #++
  if(not exist #) eoz=0;
  }
  */

   fs_loadgame(dir,name,&crc,0,ZIP);
 
   i=DAT_LookFor(crc);

   if(i!=-1) fs_add(ZIP,i,name,0);
    else fs_add(ZIP,65535,name,0); //deberia almacenar "real_filename"
 }
 else
 if(head[0]==0x1f&&head[1]==0x8b) //gzip
 {
   fs_loadgame(dir,name,&crc,0,GZ);
 
   i=DAT_LookFor(crc);

   if(i!=-1) fs_add(GZ,i,name,0);
    else fs_add(GZ,65535,name,0); //deberia almacenar "real_filename"
  }
 else
 {
//  if(head[0]=='N'&&head[1]=='E'&&head[2]=='S'&&head[3]=='\x1a') //.NES
  {
   fs_loadgame(dir,name,&crc,0,UNP);

   i=DAT_LookFor(crc);

   if(i!=-1) fs_add(UNP,i,name,0);
    else fs_add(UNP,65535,name,0); //deberia almacenar "name sin ."
  }
 }

}





//ok
void fs_write_ini(char *dir, char *name)
{
 char filename[255]; F_HANDLE file; unsigned long size;

 gm_sprintf(filename,"%s\\%s",dir,name);

 if(GpFileCreate(filename, ALWAYS_CREATE, &file)!=SM_OK) return;

 GpFileClose(file);

 if(GpFileOpen(filename, OPEN_W, &file)!=SM_OK) return;

 //sort
 {
 int i,j;
 for(i=0;i<ini.num_games_in_ini-1;i++)
  for(j=i+1;j<ini.num_games_in_ini;j++)
  {
   char str1[255], str2[255]; ini_game k;

   if(ini.game[i].entry==65535) gm_sprintf(str1,ini.game[i].file);
    else    gm_sprintf(str1,"%s %s",ini.game[i].entry>GBC_ROMS?"(GB)":"(GBC)",MyDat[ini.game[i].entry].name);
          //gm_sprintf(str1,"%s",MyDat[ini.game[i].entry].name);
   if(ini.game[j].entry==65535) gm_sprintf(str2,ini.game[j].file);
    else    gm_sprintf(str2,"%s %s",ini.game[j].entry>GBC_ROMS?"(GB)":"(GBC)",MyDat[ini.game[j].entry].name);
          //gm_sprintf(str2,"%s",MyDat[ini.game[j].entry].name);

   if(strcmp(str1,str2)>0)
   {
    gm_memcpy(&k,&ini.game[i],sizeof(ini_game));
    gm_memcpy(&ini.game[i],&ini.game[j],sizeof(ini_game));
    gm_memcpy(&ini.game[j],&k,sizeof(ini_game));
   }
  }
 }

 ini.version_ini=version_emu;
 size=sizeof(int)*2+sizeof(ini.game[0])*ini.num_games_in_ini;
 GpFileWrite(file, (void *)&size, sizeof(unsigned long));
 GpFileWrite(file, (void *)&ini, size);
 GpFileClose(file);
}

//ok
int fs_read_ini(char *dir, char *name)
{
 char filename[255]; F_HANDLE file; unsigned long size,temp;

 gm_memset(&ini,0,sizeof(ini));

 gm_sprintf(filename,"%s\\%s",dir,name);

 if(GpFileOpen(filename, OPEN_R, &file)!=SM_OK) return 0;

 GpFileRead(file, (void *)&size, sizeof(unsigned long), &temp);
 GpFileRead(file, (void *)&ini, size, &temp);
 GpFileClose(file);

 while(ini.game[ini.num_games_in_ini].type!=UNK) ini.num_games_in_ini++;

 return 1;
}


//ok
void fs_scandir(char *dir, char *name)
{
 int i;
 char temp[255];

 gm_sprintf(g_path_curr,"%s\\",dir);

// GpFatInit();

 if(!fs_read_ini(dir,name))
 {
  if(ini.version_ini!=version_emu)
  {
  if(ini.version_ini==0)
   gm_sprintf(temp,"Welcome!\nPress A for a new scanning");
  else
   gm_sprintf(temp,"Another emulator version detected\nPress A to force a new scanning");

  DrawMessageC(temp);
  FlipScreen(1,0);
  DrawMessageC(temp);
  FlipScreen(1,0);

  while(GpKeyGet()&GPC_VK_FA); DelayA();
  }

  DrawMessageC("Reading dir, please wait");
  FlipScreen(1,0);
  DrawMessageC("Reading dir, please wait");
  FlipScreen(1,0);

  GpDirEnumList(g_path_curr, 0, MAX_COUNT_FILE, (GPDIRENTRY*)&g_list_file, &g_cnt_file);

  for(i=0;i<g_cnt_file;i++)
  {
   gm_sprintf(temp," Scanning file %04d/%04d (%03d%%) \n",i+1,g_cnt_file,(i+1)*100/g_cnt_file/*,g_list_file[i].name*/);
   DrawMessageC(temp);

   DrawPercentageBar(110, 130, i, 0, g_cnt_file-1);

   FlipScreen(1,0);

   DrawMessageC(temp);

   DrawPercentageBar(110, 130, i, 0, g_cnt_file-1);

   FlipScreen(1,0);

   fs_scanfile(dir,g_list_file[i].name);
  }

  if(ini.num_games_in_ini) fs_write_ini(dir,name); 
 }

 /*
 if(ini.version_ini!=version_emu)
 {
  if(ini.version_ini==0)
  gm_sprintf(temp,"This is the first time you install this emu\nPress A to force a new scanning");
   else
  gm_sprintf(temp,"Another emulator version detected\nPress A to force a new scanning");

  DrawMessageC(temp);
  FlipScreen(1,0);
  DrawMessageC(temp);
  FlipScreen(1,0);

  while(GpKeyGet()&GPC_VK_FA); DelayA();

  DrawMessageC("Reading dir, please wait");
  FlipScreen(1,0);
  DrawMessageC("Reading dir, please wait");
  FlipScreen(1,0);

  GpDirEnumList(g_path_curr, 0, MAX_COUNT_FILE, (GPDIRENTRY*)&g_list_file, &g_cnt_file);

  gm_memset(&ini,0,sizeof(ini));

  for(i=0;i<g_cnt_file;i++)
  {
//   gm_sprintf(temp,"Scanning file %04d/%04d (%03d%%) \n\n%s",i+1,g_cnt_file,(i+1)*100/g_cnt_file,g_list_file[i].name);
   gm_sprintf(temp," Scanning file %04d/%04d (%03d%%) \n",i+1,g_cnt_file,(i+1)*100/g_cnt_file);
   DrawMessageC(temp);

   DrawPercentageBar(110, 130, i, 0, g_cnt_file-1);

   FlipScreen(1,0);

   DrawMessageC(temp);

   DrawPercentageBar(110, 130, i, 0, g_cnt_file-1);

   FlipScreen(1,0);

   fs_scanfile(dir,g_list_file[i].name);
  }

  if(ini.num_games_in_ini) fs_write_ini(dir,name);
 }
  */


/*
 if(g_cnt_file<4) // exclude empty dir, 4 items: . .. SAV SNAPS
 {
  gm_sprintf(temp,"%s\\%s",dir,name);
  GpFileRemove(temp);

  gm_sprintf(temp,"No roms found in %s    ",dir);
  DrawMessageC(temp);
  FlipScreen(1,0);
  while(1);
 }
 */
}



void fs(char *title, char *dir, char *name, unsigned char *dest)
{
 int choose=-1, needsrepainting=1, firsttime=1;
 char page[4000],pagetemp[4000];

 MyGame=(unsigned char*)dest;//gm_zi_malloc(1024*1024);

 fs_scandir(dir,name);

 GpClockSpeedChange(80000000 , 0x48012, 0);

 while(choose==-1)
 {
  static int pos=0, pause=0, touch=0; int max, key; //maxperscreen

  if(ini.num_games_in_ini>16) max=16; else max=ini.num_games_in_ini;

  pagetemp[0]=page[0]='\0';

  if(max>0) 
  {
   int i; 

   for(i=pos;i<pos+max;i++)
   {
    char datname[255];

    if(i>=ini.num_games_in_ini)
    {
     gm_sprintf(pagetemp,"%s%c",page,i==pos+max-1?' ':'\n');
     gm_sprintf(page,pagetemp);
    }
    else
    {
    if(ini.game[i].entry==65535)
     gm_sprintf(datname,"%s (CRC unknown)",ini.game[i].file);
    else
//       gm_sprintf(datname,"%s",MyDat[ini.game[i].entry].name); 
       gm_sprintf(datname,"(%s) %s",ini.game[i].entry>GBC_ROMS?"GB":"GBC",MyDat[ini.game[i].entry].name); 

    { //removes goodstuff [] (), but keeps version names
     int h=gm_lstrlen(datname), last_c, last_o;
     do
      { if(datname[h]==')') last_c=h; if(datname[h]=='(') last_o=h; 
        if(datname[h]==']') last_c=h; if(datname[h]=='[') last_o=h; 
      } while((--h)>4); //while(datname[--h]);

// ok: this remove () but region and version
      if(datname[last_c-1]>='0'||datname[last_c-1]<='9') datname[last_c+1]='\0';
       else datname[last_o-1]='\0';

// ok: this does remove any () and region but version
//      if(datname[last_c-1]>='0'&&datname[last_c-1]<='9') datname[last_c+1]='\0';
//       else datname[last_o-1]='\0';
    }

    datname[50]='\0'; //cut name

    gm_sprintf(pagetemp,"%s%04d. %c %-50s%c",page,i+1,i==pos?'>':' ',datname,i==pos+max-1?' ':'\n');
    gm_sprintf(page,pagetemp);
    }

   }

  gm_sprintf(pagetemp," %s\n\n%s",title,page);
  gm_sprintf(page,pagetemp);

  if(firsttime)
  {
   FlipScreen(1,0);
   DrawMessageC(page);
   FlipScreen(1,0);
   DrawMessageC(page);
   FlipScreen(1,0);

   firsttime=0;
  }
  else
  if(needsrepainting)
  {
  DrawMessageC(page);
  FlipScreen(1,0);
  needsrepainting=0;
  }

  key=GpKeyGet();

  if(key&GPC_VK_FA)   { choose=pos; needsrepainting=1; }
   else
  if(key&GPC_VK_UP)   { if(pause) pause--; else { if(pos!=0) pos--; needsrepainting=1; if(!touch) { pause=45; touch=1; } } }
   else
  if(key&GPC_VK_DOWN) { if(pause) pause--; else { if(pos!=ini.num_games_in_ini-1) pos++; needsrepainting=1; if(!touch) { pause=45; touch=1; } } }
   else
  if(key&GPC_VK_FL)   { pos-=16; if(pos<0) pos=0; needsrepainting=1; }
   else
  if(key&GPC_VK_FR)   { pos+=16; if(pos>ini.num_games_in_ini-1) pos=ini.num_games_in_ini-1; needsrepainting=1; }
   else pause=touch=0;

  if((key&GPC_VK_FL)&&(key&GPC_VK_FR)||(key&GPC_VK_START)||(key&GPC_VK_SELECT))
   {
    while(GpKeyGet());

    DrawMessageC("Press START to restart GP32\nPress SELECT to rescan dir\nPress A to continue...");
    FlipScreen(1,0);
    DrawMessageC("Press START to restart GP32\nPress SELECT to rescan dir\nPress A to continue...");
    FlipScreen(1,0);

    while(1)
    {
     key=GpKeyGet();

     if(key&GPC_VK_FA)     { while(GpKeyGet()); break; }
     if(key&GPC_VK_START)  GP32_System_Reset();
     if(key&GPC_VK_SELECT)
     {
      char temp[255];
      gm_sprintf(temp,"%s\\%s",dir,name);
      GpFileRemove(temp);
       
      pos=0;

      DrawMessageC(page);
      FlipScreen(1,0);
      DrawMessageC(page);
      FlipScreen(1,0);

      fs_scandir(dir,name);

      break;
     }
    }

    needsrepainting=1;
   }

  }
 }

/*
 DrawMessageC("Loading...");
 FlipScreen(1,0);
 DrawMessageC("Loading...");
 FlipScreen(1,0);
*/

 {
  unsigned long crc;
  fs_loadgame(dir,"",&crc,choose,UNK);
  gm_sprintf(szRomName,"%08x.tmp",crc);
 }

 MyGameNo=choose;
 MyGameDatNo=ini.game[choose].entry;

// gm_memcpy(dest,MyGame,MyGameSize);

// gm_free(MyGame);

 GpClockSpeedChange(132000000, 0x3a011, 3); //spiv
}





