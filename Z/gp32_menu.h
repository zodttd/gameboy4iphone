/*

MICRO K7 XP 2.6 333MHZ                                                                     81,31
PB AMD ASUS A7N8X S-A NVIDIA F2SPP AGP8X/3DDR/5PCI/SON5.1/LAN/ATA133/USB2.0/ATX            98,00
TARJ ACEL AGP QDI GE FORCE IV  FX5200 128MB DDR 8X 128BITS TV/VIDEO IN/ DVI RET            74,94
HD UDMA 120GB SEAGATE 7200 RPM ATA 100 1AG                                                 80,84

MEMORIA DDR 512MB PC2700 333MHZ                                                            55,06

REFRIGERADOR SOCKET A SPIRE XP2.6/S-370 REGULADOR TEMPERATURA 5U213C1H3G                   11,89
REGRABADORA DVD LG -R/RW GMA-4020B OEM                                                    131,68


617 €

*/

//A¥ADIR PERCENT (como minmax pero con barra de progreso)
//a¤adir macro void* y sizeof() para usar cualquier tama¤o de variable
//a¤adir ^ char en drawmessageC

enum { BLANKLINE, MINMAX, OPTIONS, BOOLEAN, PROGRESS, SUBMENU, SUBMENUEXIT, MENUEXIT };
enum { BACK, CONTINUE };

#define MAXFIELDS   20
#define FIELDLENGTH 30
#define OPTIONS 10

typedef struct 
{
       int options, selectedline;
       char *submenu; 
	struct  {
              char *value, type, flag;
		char field[MAXFIELDS][FIELDLENGTH];
		} option[OPTIONS+1];	

} menu;

void MenuInit(menu *m)
{
 gm_memset(m,0,sizeof(menu));
 m->submenu=NULL;
}


void MenuAdd(menu *m, char type, char *var, char flag, char *str)
{
 int i=0,j;

 m->option[m->options].value=var;
 m->option[m->options].type=type;
 m->option[m->options].flag=flag;

 while(*str!='\0')
 {
 j=0;

 while(*str!='\t'&&*str!='\0')
 {
  if(j<FIELDLENGTH) m->option[m->options].field[i][j++]=*str;
  str++;
 }

 m->option[m->options].field[i][j]='\0';

 i++;

 if(*str=='\t') str++;
 }

 m->options++;

 while(*(--str)=='\n')
 {
  m->option[(m->options)-1].field[i-1][--j]='\0'; //remove \n's
  m->option[m->options++].type=BLANKLINE;      //add blank lines
 }

}


void MenuSubMenu(menu *n, char *p)
{
 n->submenu=p; 
}

int MenuDisplay(menu *m)
{
  int i, j, opt=m->options, Key=GpKeyGet(), ReleaseButton=1;
  char page[1000],pagetemp[1000];

  if(m->submenu!=NULL)
  {
   Key=MenuDisplay((menu *)m->submenu);
   if(Key==GPC_VK_FB) m->submenu=NULL;
   return Key;
  }

  page[0]=pagetemp[0]='\0';

  if(Key&GPC_VK_UP)   if(m->selectedline>0) m->selectedline--;
  if(Key&GPC_VK_DOWN) if(m->selectedline<opt-1) m->selectedline++;

  if((Key&GPC_VK_FR)&&(Key&GPC_VK_FL)) Key|=GPC_VK_FB; //exit root
  
  for(i=0;i<opt;i++)
  {
   char line[255];
   line[0]='\0';

       switch(m->option[i].type)
	{
       case BLANKLINE:  if(i==m->selectedline)
                        {
                         if(Key&GPC_VK_DOWN) m->selectedline++;
                         if(Key&GPC_VK_UP) if(i>0) { j=i; while(m->option[j--].type==BLANKLINE) m->selectedline--; }
                        }
                         break;

	case MINMAX:
                     if(i==m->selectedline)
			{
                         if(Key&GPC_VK_LEFT)  {ReleaseButton=0;if((*m->option[i].value)>atoi(m->option[i].field[3])) (*m->option[i].value)--;}
                         if(Key&GPC_VK_RIGHT) {ReleaseButton=0;if((*m->option[i].value)<atoi(m->option[i].field[4])) (*m->option[i].value)++;}
                         if(Key&GPC_VK_FA)    {while(GpKeyGet());if(m->option[i].flag==BACK) Key=GPC_VK_FB; }
			}

                        if((*m->option[i].value)==atoi(m->option[i].field[3]))
                                gm_sprintf(line,"%s ^%s^..%03d..%s",m->option[i].field[0],m->option[i].field[1],(*m->option[i].value),m->option[i].field[2]);
                        else
                        if((*m->option[i].value)==atoi(m->option[i].field[4]))
                                gm_sprintf(line,"%s %s..%03d..^%s^",m->option[i].field[0],m->option[i].field[1],(*m->option[i].value),m->option[i].field[2]);
                        else
                                gm_sprintf(line,"%s %s..^%03d^..%s",m->option[i].field[0],m->option[i].field[1],(*m->option[i].value),m->option[i].field[2]);

			break;

       case PROGRESS:
                     if(i==m->selectedline)
			{
                         if(Key&GPC_VK_LEFT)  {ReleaseButton=0;if((*m->option[i].value)>atoi(m->option[i].field[3])) (*m->option[i].value)--;}
                         if(Key&GPC_VK_RIGHT) {ReleaseButton=0;if((*m->option[i].value)<atoi(m->option[i].field[4])) (*m->option[i].value)++;}
                         if(Key&GPC_VK_FA)    {while(GpKeyGet()); if(m->option[i].flag==BACK) Key=GPC_VK_FB; }
			}

                      gm_sprintf(line,"%s %s #(%08x,%08x,%08x)# %s",m->option[i].field[0],m->option[i].field[1],(int)(*m->option[i].value), atoi(m->option[i].field[3]), atoi(m->option[i].field[4]),m->option[i].field[2]);

			break;

       case SUBMENU:
                     if(i==m->selectedline)
                                if(Key&GPC_VK_FA) {while(GpKeyGet()); MenuSubMenu(m,m->option[i].value); }

                        gm_sprintf(line,"%s",m->option[i].field[0]);
                     break;

       case MENUEXIT:
                     if(i==m->selectedline)
                                if(Key&GPC_VK_FA) { while(GpKeyGet()); Key=GPC_VK_FB; }

                        gm_sprintf(line,"%s",m->option[i].field[0]);
                     break;

       case SUBMENUEXIT:
                     if(i==m->selectedline)
                                if(Key&GPC_VK_FA) { while(GpKeyGet()); MenuSubMenu((menu *)m->option[i].value, NULL); /*Key=GPC_VK_FB;*/ }

                        gm_sprintf(line,"%s",m->option[i].field[0]);
                     break;

       case BOOLEAN:
                     if(i==m->selectedline)
                                if(Key&GPC_VK_FA) {while(GpKeyGet()); (*m->option[i].value)^=1; if(m->option[i].flag==BACK) Key=GPC_VK_FB; }

                        gm_sprintf(line,"%s",(*m->option[i].value)?m->option[i].field[1]:m->option[i].field[0]);
                     break;

	case OPTIONS:
                     if(i==m->selectedline)
			{
                         if(Key&GPC_VK_LEFT)  {while(GpKeyGet());if((*m->option[i].value)>0) (*m->option[i].value)--;}
                         if(Key&GPC_VK_RIGHT) {while(GpKeyGet());if(m->option[i].field[(*m->option[i].value)+1+1][0]!='\0') (*m->option[i].value)++;}
                         if(Key&GPC_VK_FA)    {while(GpKeyGet());if(m->option[i].flag==BACK) Key=GPC_VK_FB; }
                     }

                        j=0;
                        gm_sprintf(line,"%s  ",m->option[i].field[j++]);
                        while(m->option[i].field[j][0]!='\0')
                        {
                         char linej[255];
                         linej[0]='\0';
                         gm_sprintf(linej,"%s%s%s%s",line,j-1==(*m->option[i].value)?"^ ":" ",m->option[i].field[j],j-1==(*m->option[i].value)?" ^":" ");
                         gm_sprintf(line,linej);
                         j++;
                        }

			break;

	}	

   gm_sprintf(pagetemp,"%s%c %s \n",page,i==m->selectedline?'>':' ',line);
//   gm_sprintf(pagetemp,"%s%c %s%c\n",page,i==m->selectedline?'^':' ',line,i==m->selectedline?'^':' ');
   gm_sprintf(page,pagetemp);
  }

 page[gm_lstrlen(page)-1]='\0';

 DrawMessageC(page);
 FlipScreen(1,0);

 if(ReleaseButton) if(Key) while(GpKeyGet());

// if(submenu!=NULL) { while(MenuDisplay(submenu)!=GPC_VK_FB); Key=0; }

 return Key;
}


