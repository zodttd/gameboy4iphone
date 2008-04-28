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

#import "ControllerView.h"
#import "MainView.h"

#define INP_BUTTON_UP				(0)
#define INP_BUTTON_DOWN				(1)
#define INP_BUTTON_LEFT				(2)
#define INP_BUTTON_RIGHT			(3)
#define INP_BUTTON_HARDB			(4)
#define INP_BUTTON_HARDA			(5)
#define INP_BUTTON_HARDAB			(6)
#define INP_BUTTON_START			(7)
#define INP_BUTTON_SELECT			(8)


#define BIT_U			(1<<INP_BUTTON_UP)
#define BIT_D			(1<<INP_BUTTON_DOWN)
#define BIT_L 			(1<<INP_BUTTON_LEFT)
#define BIT_R		 	(1<<INP_BUTTON_RIGHT)
#define BIT_B			(1<<INP_BUTTON_HARDB)
#define BIT_A			(1<<INP_BUTTON_HARDA)
#define BIT_AB			(1<<INP_BUTTON_HARDAB)
#define BIT_ST			(1<<INP_BUTTON_START)
#define BIT_SEL			(1<<INP_BUTTON_SELECT)

#define BIT_LPAD1		(1<<29)
#define BIT_RPAD1		(1<<30)
#define BIT_MENU		(1<<31)

extern unsigned long cPad1;
extern float __audioVolume;

@implementation ControllerView
- (id)initWithFrame:(CGRect)frame {
  if ((self == [super initWithFrame:frame])!=nil) {
    LOGDEBUG("ControllerView.initWithFrame");
    [ self setTapDelegate: self ];

    [ self getControllerCoords ];
    controllerImage = [ self getControllerImage ];
  }

  return self;
}

- (void)dealloc {
    LOGDEBUG("Controller.dealloc()");
    //[ controllerImage release ];
    [ super dealloc ];
}

- (void)drawRect:(CGRect)rect {
    CGRect controllerRect;

    LOGDEBUG("ControllerView.drawRect()");


	    controllerRect.origin.x = 0;
	    controllerRect.size.width  = 320;

		if(preferences.landscape)
	    {
		    controllerRect.origin.y = 0;
			controllerRect.size.height = 480;
		}
		else
		{
			controllerRect.origin.y = 0;
			controllerRect.size.height = 240;
		}
	    [ controllerImage draw1PartImageInRect: controllerRect ];
#if 0
    if (IS_DEBUG) {
        CGContextRef ctx = UICurrentContext();
        float red[4]   = {1, 0, 0, 1};
        float green[4] = {0, 1, 0, 1};
        float blue[4]  = {0, 0, 1, 1};

        CGContextSetFillColor(ctx, red);
        CGContextFillRect(ctx, LPad);
        CGContextFillRect(ctx, RPad);
        CGContextFillRect(ctx, Menu);

        CGContextFillRect(ctx, ButtonA);
        CGContextFillRect(ctx, ButtonB);
        CGContextFillRect(ctx, ButtonC);
        CGContextFillRect(ctx, ButtonX);
        CGContextFillRect(ctx, ButtonY);
        CGContextFillRect(ctx, ButtonZ);

        CGContextSetFillColor(ctx, blue);
        CGContextFillRect(ctx, Up);
        CGContextFillRect(ctx, Down);
        CGContextFillRect(ctx, Left);
        CGContextFillRect(ctx, Right);
        CGContextFillRect(ctx, UpLeft);
        CGContextFillRect(ctx, DownLeft);
        CGContextFillRect(ctx, UpRight);
        CGContextFillRect(ctx, DownRight);

        CGContextSetFillColor(ctx, green);
        CGContextFillRect(ctx, Start);
    }
#endif

	if(preferences.landscape)
	{
		[ self fixRects ];
	}
}

- (BOOL)ignoresMouseEvents {

    return NO;
}

- (BOOL)canHandleGestures {

   return YES;
}

- (void)fixRects {

    if (fixedRects == YES)
        return;

    MainView *mainView = [ [ [ self superview] superview ] superview ];
    UpLeft    	= [ self convertRect: UpLeft toView: mainView ];
    DownLeft  	= [ self convertRect: DownLeft toView: mainView ];
    UpRight   	= [ self convertRect: UpRight toView: mainView ];
    DownRight  	= [ self convertRect: DownRight toView: mainView ];
    Up     = [ self convertRect: Up toView: mainView ];
    Down   = [ self convertRect: Down toView: mainView ];
    Left   = [ self convertRect: Left toView: mainView ];
    Right  = [ self convertRect: Right toView: mainView ];
    Start  = [ self convertRect: Start toView: mainView ];
	Select  = [ self convertRect: Select toView: mainView ];
	

    ButtonA    	= [ self convertRect: ButtonA toView: mainView ];
    ButtonB  	= [ self convertRect: ButtonB toView: mainView ];
    ButtonAB   	= [ self convertRect: ButtonAB toView: mainView ];

    LPad   = [ self convertRect: LPad toView: mainView ];
    RPad   = [ self convertRect: RPad toView: mainView ];

    Menu   = [ self convertRect: Menu toView: mainView ];

    fixedRects = YES;
}

- (int)controllerButtonPressed:(GSEvent *)event {
    CGPoint point = GSEventGetLocationInWindow(event);
 
    return [ self controllerButtonPressedAtPoint: point ];
}

- (int)controllerButtonPressedAtPoint:(CGPoint)point {
    int button = 0;

    if (CGRectContainsPoint(ButtonAB, point)) {
            button = BIT_AB;
    }
    else if (CGRectContainsPoint(ButtonB, point)) {
            button = BIT_B;
    }
    else if (CGRectContainsPoint(ButtonA, point)) {
            button = BIT_A;
    }
    else if (CGRectContainsPoint(Start, point)) {
            button = BIT_ST;
    }
    else if (CGRectContainsPoint(Select, point)) {
            button = BIT_SEL;
    }
    else if (CGRectContainsPoint(Up, point)) {
            button = BIT_U;
    }
    else if (CGRectContainsPoint(Down, point)) {
            button = BIT_D;
    }
    else if (CGRectContainsPoint(Left, point)) {
            button = BIT_L;
    }
    else if (CGRectContainsPoint(Right, point)) {
            button = BIT_R;
    }
    else if (CGRectContainsPoint(UpLeft, point)) {
            button = BIT_U | BIT_L;
    }
    else if (CGRectContainsPoint(DownLeft, point)) {
            button = BIT_D | BIT_L;
    }
    else if (CGRectContainsPoint(UpRight, point)) {
            button = BIT_U | BIT_R;
    }
    else if (CGRectContainsPoint(DownRight, point)) {
            button = BIT_D | BIT_R;
    }
    else if (CGRectContainsPoint(LPad, point)) {
            button = BIT_LPAD1;
    }
    else if (CGRectContainsPoint(RPad, point)) {
            button = BIT_RPAD1;
    }
    else if (CGRectContainsPoint(Menu, point)) {
			button = BIT_MENU;
    }

    return button;
}

- (void)mouseDown:(GSEvent *)event {
    CGPoint point = GSEventGetLocationInWindow(event);
    int button = [self controllerButtonPressedAtPoint: point];

	if (button == BIT_MENU)
	{
		gotoMenu();
	}
	else if( button == BIT_LPAD1 )
	{
		if( __audioVolume > 0.0 ) __audioVolume -= 0.1;
		cPad1 = button;
	}
	else if( button == BIT_RPAD1 )
	{
		if( __audioVolume < 1.0 ) __audioVolume += 0.1;
		cPad1 = button;
	}
    else if (button != cPad1) 
        cPad1 = button;

    LOGDEBUG("ControllerView.mouseDown(%d) cPad1:%d", button, cPad1);
}

- (void)mouseDragged:(GSEvent *)event {
    CGPoint point = GSEventGetLocationInWindow(event);
    int button;

    button = [ self controllerButtonPressedAtPoint: point ];

	if (button == BIT_MENU)
	{
		gotoMenu();
		return;
	}
	else if( button == BIT_LPAD1 )
	{
		if( __audioVolume > 0.0 ) __audioVolume -= 0.1;
		cPad1 = button;
	}
	else if( button == BIT_RPAD1 )
	{
		if( __audioVolume < 1.0 ) __audioVolume += 0.1;
		cPad1 = button;
	}
    else if ( ! button 
         || cPad1 == button
         || button == BIT_ST
		 || button == BIT_SEL )
    {
        return;
    }

    cPad1 = button;
    LOGDEBUG("ControllerView.mouseDragged(%d) cPad1:%d", button, cPad1);
}

- (void)mouseUp:(GSEvent *)event {

    cPad1 = 0;

    LOGDEBUG("ControllerView.mouseUp() cPad1:%d", cPad1);
}

- (void)gestureStarted:(struct __GSEvent *)event {
    [ self gestureChanged: event ];
}

- (void)gestureEnded:(struct __GSEvent *)event {
    int button = [self controllerButtonPressed: event];

	if (button == BIT_MENU)
	{
		gotoMenu();
	}
	else if( button == BIT_LPAD1 )
	{
		if( __audioVolume > 0.0 ) __audioVolume -= 0.1;
		cPad1 = button;
	}
	else if( button == BIT_RPAD1 )
	{
		if( __audioVolume < 1.0 ) __audioVolume += 0.1;
		cPad1 = button;		
	}
	else cPad1 = button;

    LOGDEBUG("ControllerView.gestureEnded(%d) cPad1:%d", button, cPad1);
}

- (void)gestureChanged:(struct __GSEvent *)event {
    int i, button, button1, button2, changed = 0;
    CGPoint left  = GSEventGetInnerMostPathPosition(event);
    CGPoint right = GSEventGetOuterMostPathPosition(event);

    button1 = [ self controllerButtonPressedAtPoint: left ];
    button2 = [ self controllerButtonPressedAtPoint: right ];
#if 0
    if ( (cPad1 & button1) && ((cPad1 & 0x7F) == button2) )
      return;
#endif

	cPad1 = 0;

	if (button == BIT_MENU)
	{
		gotoMenu();
	}
	else if( button == BIT_LPAD1 )
	{
		if( __audioVolume > 0.0 ) __audioVolume -= 0.1;
		cPad1 |= button1 | button2;
		changed = 1;		
	}
	else if( button == BIT_RPAD1 )
	{
		if( __audioVolume < 1.0 ) __audioVolume += 0.1;
		cPad1 |= button1 | button2;
		changed = 1;		
	}
	else if( button1 || button2 ) 
	{
		cPad1 |= button1 | button2;
		changed = 1;
	}


    if (changed) {
        LOGDEBUG("ControllerView.gestureChanged() b1: %d b2: %d "
            "cPad1: %d", button1, button2, cPad1);
    }

}

- (UIImage *)getControllerImage {
    NSString *controllerFilename;
    UIImage *image;

    if(preferences.landscape)
    {
	    controllerFilename = [ NSString stringWithFormat:@"controller_fs%d.png", preferences.selectedSkin ];
	}
	else
	{
	    controllerFilename = [ NSString stringWithFormat:@"controller_hs%d.png", preferences.selectedSkin ];
	}

    LOGDEBUG("ControllerView.getControllerImage(): Loading controller image %s",
        [ controllerFilename cStringUsingEncoding: NSASCIIStringEncoding ]);
    image = [ UIImage applicationImageNamed: controllerFilename ];
    return image;
};

- (void)getControllerCoords {
	char string[256];
	char cFileName[256];
	FILE *fp;
	NSString *file;
	if(preferences.landscape)
	{
		file = [ NSString stringWithFormat:@"controller_fs%d.txt", preferences.selectedSkin ];
	}
	else
	{
		file = [ NSString stringWithFormat:@"controller_hs%d.txt", preferences.selectedSkin ];	
	}
	strlcpy(cFileName,
		 [ file cStringUsingEncoding: NSASCIIStringEncoding ],
		 sizeof(cFileName));

	fp = fopen(cFileName, "r");
	if (fp) 
	{
		int i = 0;
        while(fgets(string, 256, fp) != NULL && i < 16) 
        {
			char* result = strtok(string, ",");
			int coords[4];
			int i2 = 1;
			while( result != NULL && i2 < 5 )
			{
				coords[i2 - 1] = atoi(result);
				result = strtok(NULL, ",");
				i2++;
			}
			
			switch(i)
			{
			case 0:    DownLeft   	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 1:    Down   		= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 2:    DownRight    = CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 3:    Left  		= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 4:    Right  		= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 5:    UpLeft     	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 6:    Up     		= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 7:    UpRight  	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 8:    Select		= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 9:	   Start	   	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 10:   ButtonA     	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 11:   ButtonB     	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 12:   ButtonAB    	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 13:   LPad			= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 14:   RPad			= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 15:   Menu			= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			}
           	i++;
        }
        fclose(fp);
		fixedRects = false;
    }
}

/*
- (void)drawController {
	CGRect controllerRect;

	    controllerRect.origin.x = 0;
	    controllerRect.origin.y = 0;
	    controllerRect.size.width  = 320;
	    controllerRect.size.height = 480;

	    [ controllerImage draw1PartImageInRect: controllerRect ];

}
*/
@end
