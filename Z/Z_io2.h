unsigned long MyGameSize; 
unsigned char *MyGame; 
unsigned long MyGameNo, MyGameDatNo;


void LoadPacked(int skip, int gz_size, int gz_crc)
{
}

void fs_add(char type,unsigned short entry,char *file,unsigned long offset)
{
}

int fs_loadgame(char *dir, char *name, unsigned long *CRC32, int entry, int force_type) 
{
	FILE *f;
	f=fopen("/test.gb","rb");		
	if (!f) {  	
  
//  		DrawMessage("error at opening",1);
  		return 1;
  	}  	
	fseek(f,-1,SEEK_END); //bug in palmos until now...
  	MyGameSize=ftell(f)+1;
  	fseek(f,0,SEEK_SET);
		
	//sprintf(strfsize,"Loaded, size=%d...",romsize);
	//DrawMessage(strfsize,1);	
  	
  	(void *)&MyGame[0]=(uint8 *)malloc(MyGameSize);
  	if (!(void *)&MyGame[0])
  	{  		  		
		fclose(f);
  		return(2);
  	}


	if (fread((void *)&MyGame[0],MyGameSize,1,f)!=1/*romsize*/)	
	{
		fclose(f);
		free((void *)&MyGame[0]);
		return 3;
	}
	fclose(f);			
	

//		  for(CRC=i=0;i<MyGameSize;i++) GetCRC(MyGame[i]);

//		  *CRC32=CRC;
	*CRC32=0;

 return 1;		
}

//ok
void fs_scanfile(char *dir, char *name)
{
}





//ok
void fs_write_ini(char *dir, char *name)
{
}


//ok
void fs_scandir(char *dir, char *name)
{
}



void fs(char *title, char *dir, char *name, unsigned char *dest)
{
}





