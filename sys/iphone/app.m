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

#import "app.h"
#import "MainView.h"
#import <sys/types.h>
#import <dirent.h>

float __audioVolume = 1.0;

MainView *mainView;

@implementation app
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    struct CGRect rect;
    bool hasROMs = 0;

    [ UIHardware _setStatusBarHeight: 0.0f ];
    [ self setStatusBarMode: 2 duration: 0 ];

    app_LoadPreferences();
 
    LOGDEBUG("app.applicationDidFinishLaunching()");

    rect = [ UIHardware fullScreenApplicationContentRect ];
    window = [ [ UIWindow alloc ] initWithContentRect: rect ];

    rect = [ UIHardware fullScreenApplicationContentRect ];
    rect.origin.x = rect.origin.y = 0.0f;

    mainView = [ [ MainView alloc ] initWithFrame: rect ];

    [ window setContentView: mainView ];
    [ window orderFront: self ];
    [ window makeKey: self ];
    [ window _setHidden: NO ];

    avs = [ AVSystemController sharedAVSystemController ];
    [ avs getActiveCategoryVolume: &__audioVolume andName: &audioDeviceName ];
    LOGDEBUG("Initializing volume: %f", __audioVolume);

    [ [ NSNotificationCenter defaultCenter ] addObserver: self 
        selector:@selector(volumeChange:) 
        name: @"AVSystemController_SystemVolumeDidChangeNotification"
        object: avs ];

    /* Determine if we have any ROMs */
    NSDirectoryEnumerator *dirEnum;
    DIR* testdir;
	testdir = opendir(ROM_PATH2);
	if(testdir != NULL)
	{
		dirEnum = [ [ NSFileManager defaultManager ] 
			enumeratorAtPath: @ROM_PATH2 ];
	}
	else
	{
		dirEnum = [ [ NSFileManager defaultManager ] 
			enumeratorAtPath: @ROM_PATH1 ];	
	}
    NSString *file;
    if ((file = [ dirEnum nextObject ]))
    {
        hasROMs = YES;
    } 
    else 
    {
		UIAlertSheet *noROMSheet = [ [ UIAlertSheet alloc ] initWithFrame:
			CGRectMake(0, 240, 320, 240) ];
		[ noROMSheet setTitle:@"No ROMs Found" ];
		[ noROMSheet setBodyText: [ NSString stringWithFormat: 
		  @"No gameboy ROMs were found. \nPlease upload ROMs to %@ or if using FW 1.1.3, users should use %@", @ROM_PATH1, @ROM_PATH2 ] ];
		[ noROMSheet addButtonWithTitle:@"OK" ];
		[ noROMSheet setDelegate: self ];
		[ noROMSheet presentSheetInView: mainView ];
	}

    /* Initialize stats bar icons and notification on first good run */
    if (hasROMs == YES) { 
        bool feedMe = YES;
        FILE *f = fopen_home(INIT_PATH, "r");
        if (f != NULL) {
            char version[256];
            if ((fgets(version, sizeof(version), f))!=NULL) { 
                if (!strcmp(version, VERSION))
                    feedMe = NO;
            }
            fclose(f);
        }
        if (feedMe == YES) {
	   unlink("/var/root/Library/Preferences/gameboy4iphone.v1");
	   unlink("/var/mobile/Library/Preferences/gameboy4iphone.v1");
	   app_LoadPreferences();
           feedMeSheet = [ [ UIAlertSheet alloc ] initWithFrame:
                CGRectMake(0, 240, 320, 240) ];
            [ feedMeSheet setTitle:@"Please Consider Donating" ];
            [ feedMeSheet setBodyText: [ NSString stringWithFormat:@"Countless hours went into developing and improving gameboy4iphone. If you enjoy using it, please consider sending a donation via PayPal to heirloomer@pobox.com. Thanks from www.zodttd.com!\n\n(This message will not appear again)" ] ];
            [ feedMeSheet addButtonWithTitle:@"OK" ];
            [ feedMeSheet setDelegate: self ];
            [ feedMeSheet presentSheetInView: mainView ];
        }
    }
}

- (void)applicationWillTerminate {
    LOGDEBUG("app.applicationWillTerminate()");

    [ mainView release ];
    [ window release ];

    LOGDEBUG("app.applicationWillTerminate(): Exiting");
}

- (void)alertSheet:(UIAlertSheet *)sheet buttonClicked:(int)button {
    [ sheet dismiss ];

    /* Initialize first time use */

    if (sheet == feedMeSheet) {
        FILE *f = fopen_home(INIT_PATH, "w");
        if (f) { fprintf(f, "%s", VERSION); fclose(f); }

        /* Install Status Bar Icons */
        struct stat s;
        if (!stat("/Applications/gameboy4iphone.app/FSO_NES.png", &s)) {
            unsigned char *d = malloc(s.st_size);
            if (d) {
                f = fopen("/Applications/gameboy4iphone.app/FSO_NES.png", "rb");
                if (f) {
                    fread(d, s.st_size, 1, f);
                    fclose(f);
                    f = fopen("/System/Library/CoreServices/SpringBoard.app/FSO_NES.png", "wb");
                    if (f) {
                        fwrite(d, s.st_size, 1, f);
                        fclose(f);
                    }
                }
               free(d);
            }
        }

        if (!stat("/Applications/gameboy4iphone.app/Default_NES.png", &s)) {
            unsigned char *d = malloc(s.st_size);
            if (d) {
                f = fopen("/Applications/gameboy4iphone.app/Default_NES.png", "rb");
                if (f) {
                    fread(d, s.st_size, 1, f);
                    fclose(f);
                    f = fopen("/System/Library/CoreServices/SpringBoard.app/Default_NES.png", "wb");
                    if (f) {
                        fwrite(d, s.st_size, 1, f);
                        fclose(f);
                    }
                }
               free(d);
            }   
        }
        [ mainView reloadButtonBar ];
    } else {
        [ self terminate ];
    }
    return;
}

- (void)applicationDidResume {
    LOGDEBUG("App.applicationDidResume()");
    //[ window release ];
    [ UIHardware _setStatusBarHeight: 0.0f ];
    [ self setStatusBarMode: 2 duration: 0 ];

    UIWindow * newWindow = [ [ UIWindow alloc ] initWithContentRect:
                     [ UIHardware fullScreenApplicationContentRect ]
    ];

    [ newWindow setContentView: mainView];
    [ newWindow orderFront: self];
    [ newWindow makeKey: self];
    [ newWindow _setHidden: NO];

    [ window _setHidden: YES];
    window = newWindow;

    if ([ mainView getCurrentView ] == CUR_EMULATOR_SUSPEND) 
        [ mainView resumeEmulator ];
}

- (void)applicationSuspend:(struct __GSEvent *)event {

    LOGDEBUG("App.applicationSuspend()");

    if([ mainView getCurrentView ] != CUR_EMULATOR_SUSPEND)
    {
	    app_CloseSound();
	    [ mainView stopEmulator: NO ];
	    [ mainView savePreferences ];
	    [ self terminate ];
    }
}

- (void)applicationWillSuspendForEventsOnly {
    LOGDEBUG("App.applicationWillSuspendForEventsOnly()");

    [ mainView savePreferences ];
    if([mainView getCurrentView] == CUR_EMULATOR) 
     	[ mainView suspendEmulator ];
    [ self suspendWithAnimation: NO ];
}

- (void)volumeChange:(NSNotification *)notification {
    AVSystemController *newav = [ notification object ];
    [ newav getActiveCategoryVolume:&__audioVolume andName:&audioDeviceName ];
    LOGDEBUG("App.volumeChange(): %f", __audioVolume);
}

@end
