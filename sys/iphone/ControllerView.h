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

#import <Foundation/Foundation.h>
#import <GraphicsServices/GraphicsServices.h>
#import <UIKit/UIKit.h>
#import <UIKit/UIView-Geometry.h>
#import "ScreenView.h"
#import "app_iPhone.h"

@interface ControllerView : UIView
{
    UIImage *controllerImage;
    int screenOrientation;
    BOOL fixedRects;

    CGRect ButtonA;
    CGRect ButtonB;
    CGRect ButtonAB;
    CGRect Up;
    CGRect Left;
    CGRect Down;
    CGRect Right;
    CGRect UpLeft;
    CGRect DownLeft;
    CGRect UpRight;
    CGRect DownRight;
    CGRect Start;
	CGRect Select;
    CGRect LPad;
    CGRect RPad;
    CGRect Menu;
    CGRect notifyUpdateRect;
    
    int orientation;
}

- (id)initWithFrame:(CGRect)frame;
- (void)dealloc;
- (void)drawRect:(CGRect)frame;
- (BOOL)ignoresMouseEvents;
- (void)fixRects;
- (int)controllerButtonPressed:(struct __GSEvent *)event;
- (void)mouseDown:(struct __GSEvent *)event;
- (void)mouseDragged:(struct __GSEvent *)event;
- (void)mouseUp:(struct __GSEvent *)event;
- (int)controllerButtonPressedAtPoint:(CGPoint)point;
- (UIImage *)getControllerImage;
- (void)drawController;
- (void)getControllerCoords;

@end
