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

#include <unistd.h>

#import "FileTable.h"

@implementation FileTable

- (int)swipe:(int)type withEvent:(struct __GSEvent *)event;
{
    if (allowDelete == YES) {
        CGPoint point= GSEventGetLocationInWindow(event);
        point.y -= 40;
        CGPoint offset = _startOffset; 
        LOGDEBUG("FileTable.swipe: %d %f, %f", type, point.x, point.y);

        point.x += offset.x;
        point.y += offset.y;
        int row = [ self rowAtPoint:point ];

        [ [ self visibleCellForRow:row column:0] 
           _showDeleteOrInsertion:YES 
           withDisclosure:NO
           animated:YES 
           isDelete:YES 
           andRemoveConfirmation:YES
        ];

        return [ super swipe:type withEvent:event ];
    }
}

- (void)allowDelete:(BOOL)allow {
    allowDelete = allow;
}

- (void)setBookmarks:(BOOL)bookmarks {
    isBookmarks = bookmarks;
}

- (BOOL)getBookmarks {
    return isBookmarks;
}

@end

@implementation UIDeletableCell

- (void)removeControlWillHideRemoveConfirmation:(id)fp8
{
    [ self _showDeleteOrInsertion:NO
          withDisclosure:NO
          animated:YES
          isDelete:YES
          andRemoveConfirmation:YES
    ];
}

- (void)_willBeDeleted
{
    NSString *file = [_path stringByAppendingPathComponent: [_files objectAtIndex:[ _table _rowForTableCell:self]]];
    char *fn = (char *) [file cStringUsingEncoding: NSASCIIStringEncoding];

    LOGDEBUG("UIDeletableCell._willBeDeleted: %s", fn);
 
    if ((!strcmp(fn + (strlen(fn)-4), ".bin"))
     	|| (!strcasecmp(fn + (strlen(fn)-3), ".gb"))
     	|| (!strcasecmp(fn + (strlen(fn)-4), ".gbc"))
		)
    {
        if ([ _table getBookmarks ] == YES)
            [ self deleteBookmark: fn ];
        else
            unlink(fn);
    }

    [ _files removeObjectAtIndex: [ _table _rowForTableCell: self ] ];
}

- (void)deleteBookmark: (const char *)file {
    FILE *in, *out;
    in = fopen_home("Library/Preferences/gameboy4iphone.bookmarks", "r");
    out = fopen("/tmp/gameboy4iphone.bookmarks", "w");
    if (out) {
        char *s, *t, *u;
        t = strdup(file);
        s = strtok(t, "/");
        while(s) {
            u = s;
            s = strtok(NULL, "/");
        }
        LOGDEBUG("deleteBookmark: deleting '%s'", u);

        if (in) {
            char buff[1024];
            while((fgets(buff, sizeof(buff), in))!=NULL) {
                if (strncmp(buff, u, strlen(u))) {
                    fprintf(out, "%s", buff);
                }
            }
            fclose(in);
        }
        fclose(out);
        free(t);
        rename("/tmp/gameboy4iphone.bookmarks", "/var/mobile/Library/Preferences/gameboy4iphone.bookmarks");
        rename("/tmp/gameboy4iphone.bookmarks", "/var/root/Library/Preferences/gameboy4iphone.bookmarks");
    }
}


- (void)setTable:(FileTable *)table {
    _table = table;
}

- (void)setFiles:(NSMutableArray *)files {
    _files = files;
}

- (NSString *)path {
        return [[_path retain] autorelease];
}

- (void)setPath: (NSString *)path {
        [_path release];
        _path = [path copy];
}

@end

