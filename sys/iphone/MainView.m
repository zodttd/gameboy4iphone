/*

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; version 2
 of the License.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#import <UIKit/UIKit.h>
#import <UIKit/UIPreferencesTableCell.h>
#import <UIKit/UIPreferencesTextTableCell.h>
#import <UIKit/UIAlertSheet.h>

#import "MainView.h"
#include "app_iPhone.h"

#import <errno.h>
#import <sys/types.h>
#import <dirent.h>
#import <unistd.h>

char __savefileName[512];
char __lastfileName[512];
char *__fileName;
int __mute;
int __emulation_run;
extern char __fileNameTempSave[512];

static MainView *sharedInstance = nil;

void gotoMenu()
{
	[ sharedInstance gotoMenu ];
}

@implementation MainView 
- (id)initWithFrame:(struct CGRect)rect {
    if ((self == [ super initWithFrame: rect ]) != nil) {
	
	sharedInstance = self;

        LOGDEBUG("MainView.initWithFrame()");

        mainRect = rect;
	mainRect = [ UIHardware fullScreenApplicationContentRect ];
	mainRect.origin.x = mainRect.origin.y = 0.0f;

        currentView = CUR_BROWSER;

        navBar = [ self createNavBar ];
        [ self setNavBar ];

        transitionView  = [ self createTransitionView: 48 ];
        prefTable       = [ self createPrefPane ];
        fileBrowser     = [ self createBrowser ];
        savedBrowser    = [ self createBrowser ];
        recentBrowser   = [ self createBrowser ];
        bookmarkBrowser = [ self createBrowser ];
        currentBrowserPage = CB_NORMAL;

        if (preferences.canDeleteROMs) {
            [ fileBrowser setAllowDeleteROMs: YES ];
            allowDeleteROMs = YES;
         } else {
            [ fileBrowser setAllowDeleteROMs: NO ];
            allowDeleteROMs = NO;
         }

         [ savedBrowser setSaved: YES ];
         [ savedBrowser reloadData ];

         [ recentBrowser setRecent: YES ];
         [ recentBrowser reloadData ];

         [ bookmarkBrowser setBookmarks: YES ];
         [ bookmarkBrowser reloadData ];


        [ self addSubview: navBar ];

        [ self addSubview: transitionView ];
        [ transitionView transition:1 toView:fileBrowser ];

        buttonBar = [ self createButtonBar ];
        [ self addSubview: buttonBar ];
        LOGDEBUG("MainView.initWithFrame(): Done");
    }

    return self;
}

- (void)dealloc {
    LOGDEBUG("MainView.dealloc()");
        [ prefTable release ];
        [ navBar release ];
	[ fileBrowser release ];
	[ super dealloc ];
}

- (void)alertSheet:(UIAlertSheet *)sheet buttonClicked:(int)button {
    LOGDEBUG("alertSheet:buttonClicked: %d", button);

    if (sheet == badROMSheet) {
        LOGDEBUG("alertSheet:buttonClicked(): badROMSheet");
    } else if (sheet == supportSheet) {
		if( button == 1 )
		{
			[UIApp openURL:[NSURL URLWithString:@"http://www.zodttd.com"]];
		}
		else if( button == 2 )
		{
			[UIApp openURL:[NSURL URLWithString:@"http://www.modmyifone.com/forums/?styleid=3"]];
		}
    } else if (sheet == saveStateSheet) {
        LOGDEBUG("alertSheet:buttonClicked(): saveStateSheet %d", button);
        if (button == 1) 
        {
			if( (!strcasecmp(__lastfileName + (strlen(__lastfileName)-4), ".svs")) )
			{
				if( strcasecmp(__lastfileName, __fileNameTempSave) )
				{
					unlink(__lastfileName);
				}
				rename(__fileNameTempSave, __lastfileName);
            }
            [ savedBrowser reloadData ];
        }
        else if (button == 2)
        {
			[ savedBrowser reloadData ];
        }
        else
        {
			app_DeleteTempState();
        }        
    } else if (sheet == selectROMSheet) {
        switch (button) {
            case (1):
                [ self load ];
                break;
            case (2):
            	{
            	char tempbuf[512];
            	sprintf(tempbuf, "%s", [ m_currentFile cStringUsingEncoding: NSASCIIStringEncoding ]);
                if ( (!strcasecmp(tempbuf + 
                        (strlen(tempbuf)-4), ".svs")) )
                {
                    unlink(tempbuf);
                    [ savedBrowser reloadData ];
                } else {
                    if ([ self isBookmarked: m_currentFile ] == NO) 
                    { 
                        LOGDEBUG("alertSheet.buttonClicked: calling addBookmark");
                        [ self addBookmark: m_currentFile ];
                    }
                }
                }
                break;
            case (3):
            	{
            	char tempbuf[512];
            	sprintf(tempbuf, "%s", [ m_currentFile cStringUsingEncoding: NSASCIIStringEncoding ]);
                if ( (!strcasecmp(tempbuf + 
                        (strlen(tempbuf)-4), ".svs")) )
                {
                    if ([ self isBookmarked: m_currentFile ] == NO) {

                        LOGDEBUG("alertSheet.buttonClicked: calling addBookmark (2)");

                        [ self addBookmark:m_currentFile ];
                    }
                }
                }
                break;
        }
    }   

    [ sheet dismiss ];
}

- (void)navigationBar:(UINavigationBar *)navbar buttonClicked:(int)button {
    switch (button) {

        /* Left Navigation Button */
        case 1:
            switch (currentView) {
                case CUR_PREFERENCES:
                    if ([ self savePreferences ] == YES) {
                        currentView = CUR_BROWSER;
                        [ self addSubview: buttonBar ];
                        if (currentBrowserPage == CB_NORMAL)
                            [ transitionView transition:2 toView:fileBrowser ];
                        else if (currentBrowserPage == CB_SAVED)
                            [ transitionView transition:2 toView:savedBrowser ];
                        else if (currentBrowserPage == CB_RECENT) {
                            [ recentBrowser reloadData ];
                            [ transitionView transition:2 toView:recentBrowser ];
                        } else if (currentBrowserPage == CB_BOOKMARKS) {
                            [ bookmarkBrowser reloadData ];
                            [ transitionView transition:2 toView:bookmarkBrowser ];
                        }
                    }
                    break;

                case CUR_BROWSER:
                    if (currentBrowserPage == CB_RECENT) {
                        unlink("/var/root/Library/Preferences/gameboy4iphone.history");
                        unlink("/var/mobile/Library/Preferences/gameboy4iphone.history");
                        [ recentBrowser reloadData ];
                    }
                    break;

                case CUR_EMULATOR:
                    [ self stopEmulator: YES];
                    currentView = CUR_BROWSER;
                    if (currentBrowserPage == CB_NORMAL)
                        [ transitionView transition:2 toView:fileBrowser ];
                    else if (currentBrowserPage == CB_SAVED)
                        [ transitionView transition:2 toView:savedBrowser ];
                    else if (currentBrowserPage == CB_RECENT) {
                        [ recentBrowser reloadData ];
                        [ transitionView transition:2 toView:recentBrowser ];
                    } else if (currentBrowserPage == CB_BOOKMARKS) {
                        [ bookmarkBrowser reloadData ];
                        [ transitionView transition:2 toView:bookmarkBrowser ];
                    }
                    break;
            }
            break;

        /* Right Navigation Button */
        case 0:
            switch (currentView) {
				case CUR_PREFERENCES:
			        supportSheet = [ [ UIAlertSheet alloc ] initWithFrame: 
    		        CGRectMake(0, 240, 320, 240) ];
			        [ supportSheet setTitle:@"Support ZodTTD" ];
			        [ supportSheet setBodyText:[NSString stringWithFormat:@"Thank you for using my programs for the iPhone and iPod Touch. For more information on my projects head to zodttd.com. Also be sure to visit modmyifone.com for up to date news and a large community of iPhone and iPod Touch users!"] ];
			        [ supportSheet addButtonWithTitle:@"www.zodttd.com" ];
			        [ supportSheet addButtonWithTitle:@"www.modmyifone.com" ];
			        [ supportSheet addButtonWithTitle:@"Cancel" ];
			        [ supportSheet setDelegate: self ];
			        [ supportSheet presentSheetInView: self ];
					break;
                case CUR_BROWSER:
                    currentView = CUR_PREFERENCES;
                    [ buttonBar removeFromSuperview ];
                    [ transitionView transition:1 toView:prefTable ];
                    break;

                case CUR_EMULATOR:
                    if (!__mute) {
                        __mute = 1;
			app_MuteSound();
                    } else {
                        __mute = 0;
			app_DemuteSound();
                    }
            }
            break;
    }

    [ self setNavBar ];
}

- (void)fileBrowser: (FileBrowser *)browser fileSelected:(NSString *)file {
    m_currentFile = [ file copy ];
    BOOL bookmarked = [ self isBookmarked: file ];

    selectROMSheet = [ [ UIAlertSheet alloc ] initWithFrame:
        CGRectMake(0, 240, 320, 240) ];
    [ selectROMSheet setTitle:[ file lastPathComponent ] ];
    [ selectROMSheet setBodyText:@"Please select an action:" ];

	{
	char tempbuf[512];
	sprintf(tempbuf, "%s", [ file cStringUsingEncoding: NSASCIIStringEncoding ]);
    if ( (!strcasecmp(tempbuf + 
            (strlen(tempbuf)-4), ".svs")) )
    {
        [ selectROMSheet addButtonWithTitle:@"Restore Saved Game" ];
        [ selectROMSheet addButtonWithTitle:@"Delete Saved Game" ];
    } else {
        [ selectROMSheet addButtonWithTitle:@"Start New Game" ];
    }
	}

    if (bookmarked == NO)
        [ selectROMSheet addButtonWithTitle:@"Bookmark" ];

    [ selectROMSheet addButtonWithTitle:@"Cancel" ];
    [ selectROMSheet setDelegate: self ];
    [ selectROMSheet presentSheetInView: self ];
}

- (BOOL)isBookmarked: (NSString *)file {
    char cFileName[256];
    char buff[1024];
    FILE *in;
    BOOL isBookmarked = NO;
    char *s, *t, *u;

    strlcpy(cFileName,
         [ file cStringUsingEncoding: NSASCIIStringEncoding ],
         sizeof(cFileName));

    t = strdup(cFileName);
    s = strtok(t, "/");
    while(s) {
        u = s;
        s = strtok(NULL, "/");
    }

    LOGDEBUG("isBookmarked: checking %s", u);

    in = fopen_home("Library/Preferences/gameboy4iphone.bookmarks", "r");
    if (in) { 
        while((fgets(buff, sizeof(buff), in)!=NULL)) {
            if (!strncmp(buff, u, strlen(u)))
                isBookmarked = YES;
        }
        fclose(in);
    }
    return isBookmarked;
}

- (void)addBookmark: (NSString *)file {
    char cFileName[256];
    FILE *out;

    strlcpy(cFileName,
         [ file cStringUsingEncoding: NSASCIIStringEncoding ],
         sizeof(cFileName));

    LOGDEBUG("Adding bookmark: %s", cFileName);

    out = fopen_home("Library/Preferences/gameboy4iphone.bookmarks", "a");
    if (out) {
        char *s, *t, *u;
        t = strdup(cFileName);
        s = strtok(t, "/");
        while(s) {
            u = s;
            s = strtok(NULL, "/");
        }
        fprintf(out, "%s\n", u);
        fclose(out);
        free(t);
    }
    [ bookmarkBrowser reloadData ];
}

- (void)load {
    int err;
    NSString *file = [ m_currentFile copy ];
    char cFileName[256];

    strlcpy(cFileName, 
         [ file cStringUsingEncoding: NSASCIIStringEncoding ],
         sizeof(cFileName));

    LOGDEBUG("MainView.fileBrowser.fileSelected('%s')", cFileName);

    [ UIHardware _setStatusBarHeight: 0.0f ];
    [ UIApp setStatusBarMode: 2 duration: 0 ];

    mainRect = [ UIHardware fullScreenApplicationContentRect ];
    mainRect.origin.x = mainRect.origin.y = 0.0f;
    [ parentWindow setFrame: [ UIHardware fullScreenApplicationContentRect ] ];
    [ self setFrame: mainRect ];
    [ emuView removeFromSuperview ];
    [ emuView release ];
    emuView = [ self createEmulationView ];

    [ transitionView removeFromSuperview ];
    [ transitionView release ];
    transitionView  = [ self createTransitionView: 0 ];
    [ self addSubview: transitionView ];

    err = [ emuView loadROM: file ];
    if (!err) {
        FILE *in, *out;
        __fileName = strdup(cFileName);
        sprintf(__lastfileName, "%s", __fileName);
	[ prefTable release ];
        prefTable    = [ self createPrefPane ];

        /* Prepend to most recent log */
        in = fopen_home("Library/Preferences/gameboy4iphone.history", "r");
        out = fopen("/tmp/gameboy4iphone.history", "w");
        if (out) {
            char *s, *t, *u;
            t = strdup(cFileName);
            s = strtok(t, "/");
            while(s) {
                u = s;
                s = strtok(NULL, "/");
            }
            fprintf(out, "%s\n", u);
            if (in) {
                char buff[1024];
                int total = 1;
                while(total != 25 && (fgets(buff, sizeof(buff), in))!=NULL) {
                    if (strncmp(buff, u, strlen(u))) {
                        fprintf(out, "%s", buff);
                        total++;
                    }
                }
                fclose(in);
            }
            fclose(out);
            rename("/tmp/gameboy4iphone.history", "/var/mobile/Library/Preferences/gameboy4iphone.history");
            rename("/tmp/gameboy4iphone.history", "/var/root/Library/Preferences/gameboy4iphone.history");
            free(t);
        }

        currentView = CUR_EMULATOR;
        [ transitionView transition:1 toView:emuView ];
        [ self startEmulator ];
    } else {
        badROMSheet = [ [ UIAlertSheet alloc ] initWithFrame: 
            CGRectMake(0, 240, 320, 240) ];
        [ badROMSheet setTitle:@"Unable to load ROM Image" ];
        [ badROMSheet setBodyText:[NSString stringWithFormat:@"Unable to load ROM image %@. It may not be a valid ROM image, or the resources may not be available to load it.", file] ];
        [ badROMSheet addButtonWithTitle:@"OK" ];
        [ badROMSheet setDelegate: self ];
        [ badROMSheet presentSheetInView: self ];
    }
}

- (void)startEmulator {
    LOGDEBUG("MainView.startEmulator()");

    __emulation_run = 1;

    [ UIApp addStatusBarImageNamed: @"NES" removeOnAbnormalExit: YES ];
    pthread_create(&emulation_tid, NULL, app_Thread_Start, NULL);
    LOGDEBUG("MainView.startEmulator(): Done");

    [ navBar removeFromSuperview ];
    [ buttonBar removeFromSuperview ];
}

- (void)stopEmulator:(BOOL)promptForSave {

    LOGDEBUG("MainView.stopEmulator()");
    if (currentView != CUR_EMULATOR)
        return;

    if(__emulation_run != 0)
	{
		app_Halt();

		LOGDEBUG("MainView.stopEmulator(): calling pthread_join()");
		pthread_join(emulation_tid, NULL);
		LOGDEBUG("MainView.stopEmulator(): pthread_join() returned");
	}

    [ UIApp removeStatusBarImageNamed: @"NES" ];

    LOGDEBUG("MainView.stopEmulator(): saving SRAM");

	if (promptForSave == YES) 
	{
			/*if (preferences.autoSave) {
				[ savedBrowser reloadData ];
			} 
			else*/
			{
				saveStateSheet = [ [ UIAlertSheet alloc ] initWithFrame:
					CGRectMake(0, 240, 320, 240) ];
				[ saveStateSheet setTitle:@"Do you want to save this game?" ];
				[ saveStateSheet setBodyText:@"Do you want to create a new save state or overwrite the currently loaded save?" ];
				[ saveStateSheet addButtonWithTitle:@"Yes Overwrite Current" ];
				[ saveStateSheet addButtonWithTitle:@"Yes" ];
				[ saveStateSheet addButtonWithTitle:@"No" ];
				[ saveStateSheet setDelegate: self ];
				[ saveStateSheet presentSheetInView: self ];
			}
    }
    else
    {
		app_DeleteTempState();
    }

    LOGDEBUG("MainView.stopEmulator(): Done");
}

- (void)suspendEmulator {
    if (currentView != CUR_EMULATOR)
        return;
    //Main_Halt();
    currentView = CUR_EMULATOR_SUSPEND;
    LOGDEBUG("MainView.suspendEmulator(): calling pthread_join()");
    //pthread_join(emulation_tid, NULL);
    LOGDEBUG("MainView.suspendEmulator(): pthread_join() returned");
}

- (void)resumeEmulator {
    if (currentView != CUR_EMULATOR_SUSPEND)
        return;

    currentView = CUR_EMULATOR;

	//Main_Resume();
    //__emulation_run = 1;
    //pthread_create(&emulation_tid, NULL, Main_Thread_Start, NULL);
}

- (void)setNavBar {
    switch (currentView) {

        case (CUR_PREFERENCES):
            [ navItem setTitle: @"Settings" ];
            [ navBar showButtonsWithLeftTitle:@"Back"
                 rightTitle:@"Support" leftBack: YES
            ];
            break;

        case (CUR_BROWSER):
            if (currentBrowserPage != CB_RECENT) {
                [navBar showButtonsWithLeftTitle:nil
                         rightTitle:@"Settings" leftBack: NO
                ];
            } else {
                [navBar showButtonsWithLeftTitle:@"Clear"
                         rightTitle:@"Settings" leftBack: NO
                ];
            }

            switch (currentBrowserPage) {
                case (CB_NORMAL):
                    [ navItem setTitle: @"All Games" ];
                    break;
                case (CB_SAVED):
                     [ navItem setTitle: @"Saved Games" ];
                     break;
                case (CB_RECENT):
                      [ navItem setTitle: @"Most Recent" ];
                      break;
                case (CB_BOOKMARKS):
                      [ navItem setTitle: @"Bookmarks" ];
                      break;
            }

            break;

        case (CUR_EMULATOR):
            [ navItem setTitle: @"" ];
            if (!__mute) {
                [navBar showLeftButton:@"ROM List" withStyle: 2
                         rightButton:@"Mute" withStyle: 0 ];
            } else {
                [navBar showLeftButton:@"ROM List" withStyle:2
                         rightButton:@"Mute" withStyle: 1 ];
            }
            break;
    }
}

- (FileBrowser *)createBrowser {
    float offset = 48.0 * 2; /* nav bar + button bar */

    LOGDEBUG("MainView.createBrowser(): Initializing");
    FileBrowser *browser = [ [ FileBrowser alloc ] initWithFrame:
        CGRectMake(0, 0, mainRect.size.width, mainRect.size.height - offset)
    ];

    [ browser setSaved: NO ];

	/* Determine which ROM path */
	DIR* testdir;
	testdir = opendir(ROM_PATH2);
	if(testdir != NULL)
	{     
		[ browser setPath:@ROM_PATH2 ];	
    } 
    else 
    {
		[ browser setPath:@ROM_PATH1 ];
	}
    [ browser setDelegate: self ];
    [ browser setAllowDeleteROMs: allowDeleteROMs ];

    LOGDEBUG("MainView.createBrowser(): Done");
    return browser;
}

- (EmulationView *)createEmulationView {
    EmulationView *emu = [ [ EmulationView alloc ]
        initWithFrame:
            CGRectMake(0, 0, mainRect.size.width, mainRect.size.height)
    ];

    return emu;
}

- (UINavigationBar *)createNavBar {
    UINavigationBar *nav = [ [ UINavigationBar alloc ] initWithFrame:
        CGRectMake(0, 0, mainRect.size.width, 48.0f)
    ];

    [ nav setDelegate: self ];
    [ nav enableAnimation ];

    navItem = [[UINavigationItem alloc] initWithTitle:@""];
    [ nav pushNavigationItem: navItem ];

    return nav;
}

- (UITransitionView *)createTransitionView:(int)offset {
    UITransitionView *transition = [ [ UITransitionView alloc ] 
        initWithFrame:
            CGRectMake(mainRect.origin.x, mainRect.origin.y + offset, mainRect.size.width,
                       mainRect.size.height - offset)
    ];
    return transition;
}

- (BOOL)isBrowsing {

    if (currentView == CUR_EMULATOR)
        return NO;
    return YES;
}

- (BOOL)savePreferences {
    BOOL ret = YES;
    LOGDEBUG("savePreferences: currentView %d", currentView);

    if (currentView != CUR_PREFERENCES)
        return YES;

    preferences.frameSkip = [ frameControl selectedSegment ];

    preferences.selectedSkin = [ skinControl selectedSegment ];

#ifdef GUI_DEBUG
    IS_DEBUG = [ debugControl value ];
    if (IS_DEBUG != preferences.debug) {
        EmulationView *_newEmuView = [ self createEmulationView ];
        [emuView release];
        emuView = _newEmuView;
    } 
    preferences.debug = IS_DEBUG;
#else
    preferences.debug = 0;
    IS_DEBUG = 0;
#endif

    preferences.canDeleteROMs   = [ delromsControl value ];
    if (preferences.canDeleteROMs) {
        [ fileBrowser setAllowDeleteROMs: YES ];
        allowDeleteROMs = YES;
     } else {
        [ fileBrowser setAllowDeleteROMs: NO ];
        allowDeleteROMs = NO;
     }

    preferences.transparency        = [ transparencyControl value ];
    preferences.landscape       = [ landscapeControl value ];
    preferences.muted           = [ mutedControl value ];
    preferences.scaled          = [ scaledControl value ];

    app_SavePreferences();

    return ret;
}

- (UIButtonBar *)createButtonBar {
    UIButtonBar *bar;
    bar = [ [ UIButtonBar alloc ] 
       initInView: self
       withFrame: CGRectMake(0.0f, 431.0f, 320.0f, 49.0f)
       withItemList: [ self buttonBarItems ] ];
    [bar setDelegate:self];
    [bar setBarStyle:1];
    [bar setButtonBarTrackingMode: 2];

    int buttons[5] = { 1, 2, 3, 4, 5 };
    [bar registerButtonGroup:0 withButtons:buttons withCount: 5];
    [bar showButtonGroup: 0 withDuration: 0.0f];
    int tag;

    for(tag = 1; tag < 5; tag++) {
        [ [ bar viewWithTag:tag ] 
            setFrame:CGRectMake(2.0f + ((tag - 1) * 80.0f), 1.0f, 80.0f, 48.0f)
        ];
    }
    [ bar showSelectionForButton: 1];

    return bar;
}

- (NSArray *)buttonBarItems {
    return [ NSArray arrayWithObjects:
        [ NSDictionary dictionaryWithObjectsAndKeys:
          @"buttonBarItemTapped:", kUIButtonBarButtonAction,
          @"TopRated.png", kUIButtonBarButtonInfo,
          @"TopRated.png", kUIButtonBarButtonSelectedInfo,
          [ NSNumber numberWithInt: 1], kUIButtonBarButtonTag,
            self, kUIButtonBarButtonTarget,
          @"All Games", kUIButtonBarButtonTitle,
          @"0", kUIButtonBarButtonType,
          nil 
        ],

        [ NSDictionary dictionaryWithObjectsAndKeys:
          @"buttonBarItemTapped:", kUIButtonBarButtonAction,
          @"History.png", kUIButtonBarButtonInfo,
          @"History.png", kUIButtonBarButtonSelectedInfo,
          [ NSNumber numberWithInt: 2], kUIButtonBarButtonTag,
            self, kUIButtonBarButtonTarget,
          @"Saved Games", kUIButtonBarButtonTitle,
          @"0", kUIButtonBarButtonType,
          nil 
        ], 

        [ NSDictionary dictionaryWithObjectsAndKeys:
          @"buttonBarItemTapped:", kUIButtonBarButtonAction,
          @"Bookmarks.png", kUIButtonBarButtonInfo,
          @"Bookmarks.png", kUIButtonBarButtonSelectedInfo,
          [ NSNumber numberWithInt: 3], kUIButtonBarButtonTag,
            self, kUIButtonBarButtonTarget,
          @"Bookmarks", kUIButtonBarButtonTitle,
          @"0", kUIButtonBarButtonType,
          nil
        ],

        [ NSDictionary dictionaryWithObjectsAndKeys:
          @"buttonBarItemTapped:", kUIButtonBarButtonAction,
          @"MostRecent.png", kUIButtonBarButtonInfo,
          @"MostRecent.png", kUIButtonBarButtonSelectedInfo,
          [ NSNumber numberWithInt: 4], kUIButtonBarButtonTag,
            self, kUIButtonBarButtonTarget,
          @"Most Recent", kUIButtonBarButtonTitle,
          @"0", kUIButtonBarButtonType,
          nil
        ], 

        nil
    ];
}

- (void)buttonBarItemTapped:(id) sender {
    int button = [ sender tag ];
    switch (button) {
        case 1:
            [ transitionView transition:0 toView:fileBrowser ];
            currentBrowserPage = CB_NORMAL;
            break;
        case 2:
             [ transitionView transition:0 toView:savedBrowser ];
             currentBrowserPage = CB_SAVED;
             break;
        case 3:
             [ bookmarkBrowser reloadData ];
             [ transitionView transition:0 toView:bookmarkBrowser ];
             currentBrowserPage = CB_BOOKMARKS;
             break;
        case 4:
             [ recentBrowser reloadData ];
             [ transitionView transition:0 toView:recentBrowser ];
             currentBrowserPage = CB_RECENT;
             break;
    }
    [ self setNavBar ];
}

- (UIPreferencesTable *)createPrefPane {
    float offset = 0.0;

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    float transparentComponents[4] = {0, 0, 0, 0};
    float grayComponents[4] = {0.85, 0.85, 0.85, 1};

    CGColorSpaceRef colorShadow = CGColorSpaceCreateDeviceRGB();

    UIPreferencesTable *pref = [[UIPreferencesTable alloc] initWithFrame:
      CGRectMake(0, 0, mainRect.size.width, mainRect.size.height - offset)];

    [ pref setDataSource: self ];
    [ pref setDelegate: self ];

    CGSize size;
    size.height = 1;
    size.width = 1;

    int i, j;
    for(i=0;i<10;i++)
        for(j=0;j<10;j++) 
            cells[i][j] = NULL;

    frameControl = [[UISegmentedControl alloc]
        initWithFrame:CGRectMake(170.0f, 5.0f, 120.0f, 55.0f)];
    [ frameControl insertSegment:0 withTitle:@"0" animated: NO ];
    [ frameControl insertSegment:1 withTitle:@"1" animated: NO ];
    [ frameControl insertSegment:2 withTitle:@"2" animated: NO ];
    [ frameControl insertSegment:3 withTitle:@"3" animated: NO ];
    [ frameControl insertSegment:4 withTitle:@"4" animated: NO ];
    [ frameControl insertSegment:5 withTitle:@"A" animated: NO ];
    [ frameControl selectSegment: preferences.frameSkip ];

    skinControl = [[UISegmentedControl alloc]
        initWithFrame:CGRectMake(170.0f, 5.0f, 120.0f, 55.0f)];
    [ skinControl insertSegment:0 withTitle:@"0" animated: NO ];
    [ skinControl insertSegment:1 withTitle:@"1" animated: NO ];
    [ skinControl insertSegment:2 withTitle:@"2" animated: NO ];
    [ skinControl insertSegment:3 withTitle:@"3" animated: NO ];
    [ skinControl insertSegment:4 withTitle:@"4" animated: NO ];
    [ skinControl insertSegment:5 withTitle:@"5" animated: NO ];
    [ skinControl selectSegment: preferences.selectedSkin ];

    NSString *verString = [ [NSString alloc] initWithCString: VERSION ]; 
    versionString = [ [ NSString alloc ] initWithFormat: @"Version %@", verString ];
    [ verString release ];

    /* Current Game Title */
    {
        char *x, *o;
        if (!__fileName) {
            x = "(No Game Selected)";
        } else {
           char *y;
           x = strdup(__fileName);
           o = x;
           while((x = strchr(x, '/'))) {
               y = x+1;
               x = y;
           }
           x = y;
          x[strlen(x)-4] = 0;
       }
       currentGameTitle = [[NSString alloc] initWithCString: x];
       if (__fileName)
           free(o);
    }

    [ pref reloadData ];
    return pref;
}

- (int)numberOfGroupsInPreferencesTable:(UIPreferencesTable *)aTable {
         return 2;
}

 - (int)preferencesTable:(UIPreferencesTable *)aTable 
    numberOfRowsInGroup:(int)group 
{
    switch (group) { 
        case(0):
            return 2;
            break;
        case(1):
#ifdef GUI_DEBUG
            return 5;
#else
            return 4;
#endif
            break;
    }
}

- (UIPreferencesTableCell *)preferencesTable:(UIPreferencesTable *)aTable 
    cellForGroup:(int)group 
{
         if (groupcell[group] != NULL)
             return groupcell[group];

         groupcell[group] = [[UIPreferencesTableCell alloc] init];

         if (group == 0) {
             [ groupcell[group] setTitle: @"Game Options" ];
         } else if (group == 1) {
             [ groupcell[group] setTitle: @"Advanced Options" ];
         }
         return groupcell[group];
} 

- (float)preferencesTable:(UIPreferencesTable *)aTable 
    heightForRow:(int)row 
    inGroup:(int)group 
    withProposedHeight:(float)proposed 
{

    if (row == -1) {
        return 40;
    }

    if (group == 1) {
        switch (row) {
            case 0:
                return 55;
            case 2:
		return 55;
        }
    }

    return proposed;
}

- (BOOL)preferencesTable:(UIPreferencesTable *)aTable 
    isLabelGroup:(int)group 
{
    return NO;
}

- (UIPreferencesTableCell *)preferencesTable:(UIPreferencesTable *)aTable 
    cellForRow:(int)row 
    inGroup:(int)group 
{
    if (cells[row][group] != NULL)
        return cells[row][group];

    UIPreferencesTableCell *cell;

    cell = [[UIPreferencesTableCell alloc] init];
#ifdef GUI_DEBUG
    if (group == 1 && row == 4) 
#else
    if (group == 1 && row == 3)
#endif
        [ cell setEnabled: NO ];
    else
        [ cell setEnabled: YES ];

    switch (group) {
        case (0):
          switch (row) {
             case (0):
                [ cell setTitle:@"Landscape View" ];
				landscapeControl = [ [ UISwitchControl alloc ]
					initWithFrame:CGRectMake(170.0f, 5.0f, 120.0f, 30.0f)];
                [ landscapeControl setValue: preferences.landscape ];
                [ cell addSubview:landscapeControl ];
                break;
            case (1):
                [ cell setTitle:@"Mute Sound" ];
                mutedControl = [ [ UISwitchControl alloc ]
                initWithFrame:CGRectMake(170.0f, 5.0f, 120.0f, 30.0f)];
                [ mutedControl setValue: preferences.muted ];
                [ cell addSubview: mutedControl ];
                break;
        }
        break;

      case (1):
        switch (row) {
            case (0):
                [ cell setTitle:@"Can Delete ROMs" ];
                delromsControl = [[UISwitchControl alloc]
                initWithFrame:CGRectMake(170.0f, 5.0f, 120.0f, 30.0f)];
                [ delromsControl setAlternateColors: YES ];
                [ delromsControl setValue: preferences.canDeleteROMs ];
                [ cell  addSubview:delromsControl ];
                break;
            case (1):
                [ cell setTitle:@"Selected Skin" ];
                [ skinControl selectSegment: preferences.selectedSkin ];
                [ cell addSubview:skinControl ];
                break;
            case (2):
                [ cell setTitle:@"Enable Scaling" ];
		scaledControl = [ [ UISwitchControl alloc ]
                initWithFrame:CGRectMake(170.0f, 5.0f, 120.0f, 30.0f)];
                [ scaledControl setValue: preferences.scaled ];
                [ cell addSubview:scaledControl ];
                break;
#ifdef GUI_DEBUG
            case (3):
                [ cell setTitle:@"Debug Mode" ];
                debugControl = [[UISwitchControl alloc]
                    initWithFrame:CGRectMake(170.0f, 5.0f, 120.0f, 30.0f)];
                [ debugControl setValue: preferences.debug ];
                [ debugControl setAlternateColors: YES ];
                [ cell addSubview:debugControl ];
                break;
            case (4):
                [ cell setValue:versionString ];
                break;
#else
            case (3):
                [ cell setValue:versionString ];
                break;
#endif
               
        }
        break;
    }

    cells[row][group] = cell;
    return cells[row][group];
}

- (int)getCurrentView {

    return currentView;
}

- (void)reloadBrowser {

    LOGDEBUG("MainView.reloadBrowser()");
    if (currentBrowserPage == CB_NORMAL) 
        [ fileBrowser scrollToTop ];
    else
        [ savedBrowser scrollToTop ];

    [ fileBrowser reloadData ]; 
    [ savedBrowser reloadData ];
}

- (void)reloadButtonBar {
    [ buttonBar removeFromSuperview ];
    [ buttonBar release ];
    buttonBar = [ self createButtonBar ];
}

- (void)gotoMenu {
    LOGDEBUG("MainView.gotoMenu()");
    [ self stopEmulator: YES];
    currentView = CUR_BROWSER;

    LOGDEBUG("MainView.gotoMenu() transition");
    [ transitionView removeFromSuperview ];
    [ transitionView release ];
    transitionView = [ self createTransitionView: 48 ];
    [ self addSubview: transitionView ];

    LOGDEBUG("MainView.gotoMenu() transition end");

    [ self addSubview: buttonBar ];

    [ self addSubview: navBar ];
    [ self setNavBar ];

    LOGDEBUG("MainView.gotoMenu() set navbar");

    if (currentBrowserPage == CB_NORMAL)
        [ transitionView transition:1 toView:fileBrowser ];
    else if (currentBrowserPage == CB_SAVED)
        [ transitionView transition:1 toView:savedBrowser ];
    else if (currentBrowserPage == CB_RECENT) {
        [ recentBrowser reloadData ];
        [ transitionView transition:1 toView:recentBrowser ];
    } else if (currentBrowserPage == CB_BOOKMARKS) {
        [ bookmarkBrowser reloadData ];
        [ transitionView transition:1 toView:bookmarkBrowser ];
    }

    LOGDEBUG("MainView.gotoMenu() end");
}

@end
