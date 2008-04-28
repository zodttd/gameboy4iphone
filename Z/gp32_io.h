#include "gp32_crc.h"


void DrawAssistant(void (*user)(void *userdatai),void *userdata)
{
  if(user!=NULL) user(userdata);
}









#define MAXFS 11

#define MAX_COUNT_FILE  1024                                      //maximum number of file entries in one folder
#define	MAX_PATH_LEN	255					//maximum length of full path name
ERR_CODE        err;
char		g_path_curr[MAX_PATH_LEN + 1];	//current path (in full path format)
GPDIRENTRY	g_list_file[MAX_COUNT_FILE];	//list of files in current path
unsigned long   g_cnt_file = 0;                                 //number of files in current path


int mydir[MAX_COUNT_FILE];

int RetrieveDir()
{
 int i,j,k,root,mydirtemp[MAX_COUNT_FILE],err;
 char file_name[255];

 err = GpDirEnumList(g_path_curr, 0, MAX_COUNT_FILE, (GPDIRENTRY*)&g_list_file, &g_cnt_file);

 if(gm_compare(g_path_curr,"gp:\\")==0) root=0; else root=2;

 for(i=0;i<g_cnt_file;i++) mydirtemp[i]=i; 

 for(i=root;i<g_cnt_file-1;i++)
  for(j=i+1;j<g_cnt_file;j++)
   if(strcmp(g_list_file[mydirtemp[i]].name,g_list_file[mydirtemp[j]].name)>0) {k=mydirtemp[i];mydirtemp[i]=mydirtemp[j];mydirtemp[j]=k;}


 j=0;
 for(i=root?1:0;i<g_cnt_file;i++)
   {
   GPFILEATTR attr1;
   gp_str_func.sprintf(file_name, "%s%s", g_path_curr, g_list_file[mydirtemp[i]].name);
   GpFileAttr(file_name, &attr1);
   if(attr1.attr&0x10) mydir[j++]=mydirtemp[i];
   }

 for(i=root?1:0;i<g_cnt_file;i++)
   {
   GPFILEATTR attr1;
   gp_str_func.sprintf(file_name, "%s%s", g_path_curr, g_list_file[mydirtemp[i]].name);
   GpFileAttr(file_name, &attr1);
   if(!(attr1.attr&0x10)) mydir[j++]=mydirtemp[i];
   }

 g_cnt_file--;
                     //now file list sorted by dir, by name, [.] is hidden, it take cares about root dir
}




void LoadPacked(int skip, int gz_size, int gz_crc)
{
    char mychar[255];
    z_stream d_stream; /* decompression stream */
    void *temp; int i;

    temp=gm_malloc(gz_size+256);  //extra bytes

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

    for(CRC=i=0;i<MyGameSize;i++) GetCRC(MyGame[i]);
}


void LoadGame(char *file_name, int selected)
{
 F_HANDLE myfile; int chosen;
 char real_filename[255];

 GpFileGetSize(file_name, &MyGameSize);

 {
 int i,j=MyGameSize,bar,bar2,mycrc;

 GpFileOpen(file_name, OPEN_R, &myfile);

 CRC=0;
 gm_memset(MyGame, 0, 1024*1024); //limpiamos pq GpFileRead parece que deja bytes ultimos sueltos a veces

// GpFileRead(myfile, (void *)&MyGame, MyGameSize, &MyGameSize);
// for(i=0;i<MyGameSize;i++) GetCRC(MyGame[i]);

 for(i=0;i<100;i++)
 {
  GpFileRead(myfile, (void *)&MyGame[i*(j/100)], j/100, &MyGameSize);
  for(mycrc=0;mycrc<j/100;mycrc++) GetCRC(MyGame[i*(j/100)+mycrc]);

//  DrawAssistant(&Assistant3,(void *)&i);
 }

 if(j-(100*(j/100))>0)
 {
  GpFileRead(myfile, (void *)&MyGame[100*(j/100)], j-(100*(j/100)), &MyGameSize);
  for(mycrc=0;mycrc<j-(100*(j/100));mycrc++) GetCRC(MyGame[(100*(j/100))+mycrc]);
 }

 GpFileClose(myfile);
 }

 GpFileGetSize(file_name, &MyGameSize);

  if(MyGame[0]==0x1f&&MyGame[1]==0x8b) //gzip
  {
   int skip; uLongf gz_crc,gz_size; 

   //gzip
   //ID1,ID2,CM,FLG,MTIME-MTIME-MTIME-MTIME,XFL,OS
   skip=10;          //1000
   if(MyGame[3]&4)  { skip+=4; skip+=(MyGame[12]<<8)|MyGame[13]; } //FLG.FEXTRA (4 o 2?)
   if(MyGame[3]&8)  { int l=0; while(MyGame[skip++]) real_filename[l++]=MyGame[skip-1]; real_filename[l]='\0'; } //FLG.FNAME
   if(MyGame[3]&16) { while(MyGame[skip++]); } //FLG.FCOMMENT
   if(MyGame[3]&2)  { skip+=2; }                  //FLG.FHCRC

// update size, wrong?
// semiworking:
// gz_size=(MyGame[MyGameSize-3]<<24)|(MyGame[MyGameSize-2]<<16)|(MyGame[MyGameSize-1]<<8)|(MyGame[MyGameSize-0]);
// working:
   gz_size=(MyGame[MyGameSize-1]<<24)|(MyGame[MyGameSize-2]<<16)|(MyGame[MyGameSize-3]<<8)|(MyGame[MyGameSize-4]);


   gz_crc =(MyGame[MyGameSize-5]<<24)|(MyGame[MyGameSize-6]<<16)|(MyGame[MyGameSize-7]<<8)|(MyGame[MyGameSize-8]);

   LoadPacked(skip,gz_size,gz_crc);
  }
  else 
  if(MyGame[0]=='P'&&MyGame[1]=='K') //zip
  {
   int skip; uLongf gz_crc,gz_size; 

   skip=30;
   skip+=(MyGame[27]<<8)|MyGame[26]; //filename length
   skip+=(MyGame[29]<<8)|MyGame[28]; //extra fields length

   gm_memcpy(real_filename,&MyGame[30],(MyGame[27]<<8)|MyGame[26]);

   gz_size=(MyGame[25]<<24)|(MyGame[24]<<16)|(MyGame[23]<<8)|(MyGame[22]);
   gz_crc =(MyGame[17]<<24)|(MyGame[16]<<16)|(MyGame[15]<<8)|(MyGame[14]);

   LoadPacked(skip,gz_size,gz_crc);
  }
  else gm_strcpy(real_filename,g_list_file[mydir[selected]].name);

/*
 {
 int i=0;
 while(CRC!=MyDat[chosen=i].crc&&i<DATROMS) i++;
 MyDat[chosen].name[39]='\0';
 }
 */

   /*
 {
 gp_str_func.sprintf(g_string, "%s\nPress A to play!\n%d bytes\n%s",CRC==MyDat[chosen].crc?MyDat[chosen].name:real_filename,MyGameSize,"ROM File");

 while((!(GpKeyGet()&GPC_VK_FA))&&(!(GpKeyGet()&GPC_VK_FB))) ;
 }
 */
}




int BrowseDir(void)
{
 static int h=0,i=0,mydirpos=0,selected,root; static int needsrepainting=1;
 int condition=1,max;
 char file_name[255];
 static char mypage[MAX_COUNT_FILE*14];

 if(gm_compare(g_path_curr,"gp:\\")==0) root=0; else root=2;

 //h=i=0;mydirpos=0;
 //condition=1;

 if(needsrepainting)
 {
 needsrepainting=0;

 if(g_cnt_file>MAXFS) max=MAXFS; else max=g_cnt_file;
 selected=-1;

 mypage[0]='\0';

 if((err != SM_OK) && (err != ERR_EOF))
 {
 gp_str_func.sprintf(g_string, "IO ERROR");
 DrawMessageB(g_string);   
 DelayA();
 }
 else for(h=0;h<max;h++)
	{
          GPFILEATTR attr;
          char file_backup[MAX_COUNT_FILE*14], file_display[256];

                //if(g_list_file[h+i].name[0]=='.') continue;

                gp_str_func.sprintf(file_name, "%s%s", g_path_curr, g_list_file[mydir[h+i]].name);

                err = GpFileAttr(file_name, &attr);
                if(err==SM_OK)
                 {
                  if(attr.attr & 0x10) gp_str_func.sprintf(file_display,"%s [%12s] %s",i+h==mydirpos?"> ":"  ",g_list_file[mydir[h+i]].name,i+h==mydirpos?" <":"  ");
                    else gp_str_func.sprintf(file_display,"%s %14s %s",i+h==mydirpos?"> ":"  ",g_list_file[mydir[h+i]].name,i+h==mydirpos?" <":"  ");
                 }

                gp_str_func.sprintf(file_backup,"%s%s%s",mypage,file_display,h==max-1?"":"\n"); 
                gp_str_func.sprintf(mypage,file_backup); 

                //if(i+h>=root) //skip [.] and [..]
                if(i+h==mydirpos&&(GpKeyGet()&GPC_VK_FA))
                {
                 while(GpKeyGet()&GPC_VK_FA) ;

                 selected=h+i;
                }
	}

  }


  {

  if(GpKeyGet()&GPC_VK_FA) selected=mydirpos;


  if(selected!=-1)
  {    
    GPFILEATTR attr;

    gp_str_func.sprintf(file_name, "%s%s", g_path_curr, g_list_file[mydir[selected]].name);

    needsrepainting=1;

    if(GpFileAttr(file_name, &attr)==SM_OK)
     if((condition=attr.attr&0x10)!=0)
     {
        gp_str_func.sprintf(g_path_curr, "%s\\", file_name);
        RetrieveDir();
        while(GpKeyGet());
        selected=h=i=mydirpos=0;
        return 1;
     }

    LoadGame(file_name, selected);
    return 0;
  }

  DrawAssistant(&Assistant,(void *)mypage);
  
  if(condition) 
  {
   int ib=i,mb=mydirpos;

   //i=page >=mydirpos
   
   if(GpKeyGet()&GPC_VK_UP) { if(i==mydirpos&&i>0) i--; if(mydirpos>0) mydirpos--; }
   if(GpKeyGet()&GPC_VK_DOWN) {  if(mydirpos<g_cnt_file-1) mydirpos++; if(i+max-1<mydirpos) i=mydirpos-max+1; } 
   if(GpKeyGet()&GPC_VK_FB) { gp_str_func.sprintf(g_path_curr, "gp:\\"); RetrieveDir(); selected=h=i=mydirpos=0; needsrepainting=1; return 1; }

   if(GpKeyGet()&GPC_VK_FL)
   {
    if(GpKeyGet()&GPC_VK_FR) GP32_System_Reset();
    if(i>MAXFS) i-=MAXFS; else i=0;
    if(mydirpos>MAXFS) mydirpos-=MAXFS; else mydirpos=0;
   } 

   if(GpKeyGet()&GPC_VK_FR)
   {
    if(GpKeyGet()&GPC_VK_FL) GP32_System_Reset();

    if(g_cnt_file<MAXFS) {i=0;mydirpos=max-1;} else
    {
    //if(i+11<g_cnt_file-11) i+=11; else i=g_cnt_file-11; 
    //if(mydirpos<g_cnt_file-11) mydirpos+=11; else mydirpos=g_cnt_file-1;
    i+=MAXFS;
    mydirpos+=MAXFS;
    if(i>g_cnt_file-MAXFS-1) i=g_cnt_file-MAXFS;
    if(mydirpos>g_cnt_file-1) mydirpos=g_cnt_file-1;
    }

   } 

    if(i!=ib||mb!=mydirpos) needsrepainting=1; else needsrepainting=0;
  }
 }


return condition;
}
