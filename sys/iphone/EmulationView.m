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

#include <pthread.h>

#import "EmulationView.h"
#import "ControllerView.h"
#import "ScreenView.h"

static EmulationView *sharedInstance = nil;

@implementation EmulationView
- (id)initWithFrame:(CGRect)frame {
    if ((self == [super initWithFrame:frame])!=nil) {

		sharedInstance = self;

        LOGDEBUG("EmulationView.initWithFrame(): Initializing");
	
        rect = frame;

	    if(preferences.landscape)
	    {
			controllerView = [[ControllerView alloc] initWithFrame: CGRectMake(0, 0, 320, 480)];
		    [ controllerView setAlpha: 0.25 ];
		}
		else
		{
			controllerView = [[ControllerView alloc] initWithFrame: CGRectMake(0, 240, 320, 240)];
		    [ controllerView setAlpha: 1.00 ];		
		}
		
        int width, height, xOffset, yOffset;

        /* Landscape Resolution */
	    if(preferences.landscape)
	    {
            width = 320;
            height = 480;
            xOffset = 0;
            yOffset = 0;
		}
		else
		{
            width = 320;
            height = 240;
            xOffset = 0;
            yOffset = 0;
		}

        LOGDEBUG("EmulationView.initWithFrame(): Creating screem view");

        screenView = [ [ ScreenView alloc] 
            initWithFrame: CGRectMake(xOffset, yOffset, width, height) ];

        LOGDEBUG("EmulationView.initWithFrame(): Adding subviews");
        [ self addSubview: screenView ];
	    if(preferences.landscape)
	    {
			[ self addSubview: controllerView ];
        }
        else
        {
			[ self addSubview: controllerView ];        
        }
        LOGDEBUG("EmulationView.initWithFrame(): Done");
    }
    return self;
}

- (void)dealloc {
    LOGDEBUG("EmuView.dealloc()");
    [ controllerView release ];
    [ screenView release ];
    [ super dealloc ];
}

- (int)loadROM: (NSString *)path {
    char fileName[strlen([path cStringUsingEncoding: NSASCIIStringEncoding])+1];
    strcpy(fileName, [path cStringUsingEncoding: NSASCIIStringEncoding]);
    //strcpy(fileName + (strlen(fileName)-3), "gba");
    int err;

    LOGDEBUG("EmulationView.loadROM: Loading %s", fileName);

    err = app_LoadROM(fileName);
    /*if (err  == EINVAL) {
        strcpy(fileName + (strlen(fileName)-3), "GBA");
        err = (app_LoadROM(fileName) == EINVAL);
    }
	*/
    return err;
}

- (void)drawRect:(CGRect)drect {
/*
        CGContextRef ctx = UICurrentContext();
        float black[4] = {0, 0, 0, 1};
        CGContextSetFillColor(ctx, black);
        CGContextFillRect(ctx, CGRectMake(drect.origin.x, drect.origin.y, drect.size.width, drect.size.height));
*/
}

- (void)drawController {
/*
  [ controllerView drawController ];
*/
}

- (void)updateControllerSkin {
#if 0
	//if( sharedInstance != nil )
	{
	    int width, height, xOffset, yOffset;

	    /* Landscape Resolution */
	    if(preferences.landscape)
	    {
            width = 320;
            height = 480;
            xOffset = 0;
            yOffset = 0;
		}
		else
		{
            width = 320;
            height = 240;
            xOffset = 0;
            yOffset = 0;
		}

		[ screenView removeFromSuperview ];
		[ controllerView release ];
		[ screenView release ];
		screenView = [ [ ScreenView alloc] 
		    initWithFrame: CGRectMake(xOffset, yOffset, width, height) ];
		[ self addSubview: screenView ];

		//[ controllerView removeFromSuperview ];
	    if(preferences.landscape)
	    {
			controllerView = [[ControllerView alloc] initWithFrame: CGRectMake(0, 0, 320, 480)];
		    [ controllerView setAlpha: 0.25 ];
		    [ screenView addSubview: controllerView ];
		}
		else
		{
			controllerView = [[ControllerView alloc] initWithFrame: CGRectMake(0, 240, 320, 240)];
		    [ controllerView setAlpha: 1.00 ];
		    [ self addSubview: controllerView ];
		}
	}
#endif
}

@end
