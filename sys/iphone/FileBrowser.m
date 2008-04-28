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

#import "FileBrowser.h"

@implementation FileBrowser 
- (id)initWithFrame:(struct CGRect)frame{
	if ((self == [super initWithFrame: frame]) != nil) {
		UITableColumn *col = [[UITableColumn alloc]
			initWithTitle: @"FileName"
			identifier:@"filename"
			width: frame.size.width
		];

                _saved = NO;
                _recent = NO;
                _bookmarks = NO;
		_table = [[FileTable alloc] initWithFrame: CGRectMake(0, 0, frame.size.width, frame.size.height)];
		[ _table addTableColumn: col ];
		[ _table setSeparatorStyle: 1 ];
		[ _table setDelegate: self ];
		[ _table setDataSource: self ];
                if (_allowDeleteROMs == YES)
                    [ _table allowDelete: YES ];
                else
                    [ _table allowDelete: NO ];
                
		_extensions = [[NSMutableArray alloc] init];
		_files = [[NSMutableArray alloc] init];

		_delegate = nil;

		[self addSubview: _table];
	}

	return self;
}

- (void)dealloc {
    LOGDEBUG("FileBrowser.dealloc()");
	[_path release];
LOGDEBUG("FileBrowser.dealloc1()");
	[_files release];
LOGDEBUG("FileBrowser.dealloc2()");
	[_extensions release];
LOGDEBUG("FileBrowser.dealloc3()");
	[_table release];
LOGDEBUG("FileBrowser.dealloc4()");
	_delegate = nil;
	[super dealloc];
}

- (NSString *)path {
	return [[_path retain] autorelease];
}

- (void)setPath: (NSString *)path {
	[_path release];
	_path = [path copy];

	[self reloadData];
}

- (void)addExtension: (NSString *)extension {
	if (![_extensions containsObject:[extension lowercaseString]]) {
		[_extensions addObject: [extension lowercaseString]];
	}
}

- (void)setExtensions: (NSArray *)extensions {
	[_extensions setArray: extensions];
}

- (void)reloadData {
	NSFileManager *fileManager = [NSFileManager defaultManager];
	if ([fileManager fileExistsAtPath: _path] == NO) {
		return;
	}

	[ _files removeAllObjects ];

        if (_recent == NO && _bookmarks == NO) {
            NSString *file;
            NSDirectoryEnumerator *dirEnum = [[NSFileManager defaultManager] enumeratorAtPath: _path];
            while ((file = [dirEnum nextObject])) {
                char *fn = (char *) [file cStringUsingEncoding: NSASCIIStringEncoding];
                if (_saved) {
                        if (!strcasecmp(fn + (strlen(fn)-4), ".svs")) 
                            [_files addObject: file];
                } else {
                        if (!strcasecmp(fn + (strlen(fn)-4), ".bin"))
                            [_files addObject: file];
                        if (!strcasecmp(fn + (strlen(fn)-3), ".gb"))
                            [_files addObject: file];
                        if (!strcasecmp(fn + (strlen(fn)-4), ".gbc"))
                            [_files addObject: file];
                }
            }

        } else {
            FILE *file;
            if (_recent == YES) {
                file = fopen_home("Library/Preferences/gameboy4iphone.history", "r");
            } else {
                file = fopen_home("Library/Preferences/gameboy4iphone.bookmarks", "r");
            }
            if (file) {
                char buff[1024];
                while((fgets(buff, sizeof(buff), file))!=NULL) {
                    buff[strlen(buff)-1] = 0;
                    NSString *string = [ [ NSString alloc ] initWithCString: buff ];
                    [ _files addObject: string ];
                    [ string release ];
                }
                fclose(file);
            } 
        }


        if (_recent == NO) {
            NSArray *sorted = [ _files sortedArrayUsingSelector:@selector(localizedCaseInsensitiveCompare:) ];
            [ _files release ];
            _files = [ [ NSMutableArray alloc] initWithArray: sorted ];
        }

	[_table reloadData];
}

- (void)setDelegate:(id)delegate {
	_delegate = delegate;
}

- (int)numberOfRowsInTable:(UITable *)table {
	return [_files count];
}

- (UIDeletableCell *)table:(UITable *)table cellForRow:(int)row column:(UITableColumn *)col {
	UIDeletableCell *cell = [[UIDeletableCell alloc] init];

        [ cell setTable: _table ];
        [ cell setFiles: _files ];
        [ cell setPath: _path ];
        [ cell setTitle: [[_files objectAtIndex: row] stringByDeletingPathExtension ]];

	return [ cell autorelease ];
}

- (void)tableRowSelected:(NSNotification *)notification {
	if( [ _delegate respondsToSelector:@selector( fileBrowser:fileSelected: ) ] )
		[ _delegate fileBrowser:self fileSelected:[ self selectedFile ] ];
}

- (NSString *)selectedFile {
	if ([_table selectedRow] == -1)
		return nil;

	return [_path stringByAppendingPathComponent: [_files objectAtIndex: [_table selectedRow]]];
}

- (void)setSaved:(BOOL)saved {
    _saved = saved;
    if (_allowDeleteROMs == NO)
        [ _table allowDelete: saved ];
}

- (void)setRecent:(BOOL)recent {
    _recent = recent;
    [ _table allowDelete: NO ];
}

- (void)setBookmarks:(BOOL)bookmarks {
    _bookmarks = bookmarks;
    if (bookmarks == YES) {
        [ _table setBookmarks: YES ];
        [ _table allowDelete: YES ];
    }
}

- (BOOL)getSaved {
    return _saved;
}

- (void)setAllowDeleteROMs: (BOOL)allow {
    _allowDeleteROMs = allow;
    [ _table allowDelete: allow ];
}

- (void)fileBrowser: (FileBrowser *)browser fileSelected:(NSString *)file {

}

- (void)scrollToTop {
    [ _table scrollRowToVisible: 0 ];
}

@end
