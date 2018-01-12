//
//  XXRemoteXWindows.m
//  osx2x
//
// Copyright (c) Michael Dales 2002, 2003, Jon Bardin 2014
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// Neither the name of Michael Dales nor the names of its contributors may be
// used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT S HALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS I NTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//


#import "XXRemoteXWindows.h"
#import "g1bson.h"

//#import "XXController.h"
//#import "keymap.h"

// This is used when waiting for a copy event from the X side, both as a way of knowing
// who last asked for the X Selection and as a semaphor to stop multiple requests
//static XXRemoteXWindows* lastToCopy = nil;
//
//void XXCopyCallback(char* data)
//{
//   [lastToCopy copyCallBack: data];
//}


@implementation XXRemoteXWindows


-initWithHostName: (NSString*)hostname {
	return [self initWithHostName:hostname cursor_id:1];
}


-initWithHostName:(NSString*)hostname cursor_id:(int)c_id {
  
  if (self = [super init]) {
  
    int fd;
    printf("cursor is %d\n", c_id);
    cursor_no  = c_id;

    //display = XOpenDisplay((char*)[hostname cStringUsingEncoding:NSUTF8StringEncoding]);
    display = XOpenDisplay(NULL);
      
    if (display == NULL)
    {
      // Failed to open the connection, so fail to create the object
      NSLog(@"Failed to open connection to the X Server");
      return nil;
    }

    {
      screen = DefaultScreenOfDisplay(display);
      rootWindow = RootWindowOfScreen(screen);
      proofWindow = g1bson_search(display, rootWindow);
      
      {
        srand((int)time(NULL));
          
        int i;
        for(i = 0; i < 20; i++){
          name[i] = '0' + rand() % 10;
        }
        name[19] = '\0';
        g1bson_add_mpx_for_window (display, name, // name must be uniq
                                   &xid_master_kbd,
                                   &xid_slave_kbd,
                                   &xid_master_ptr,
                                   &xid_slave_ptr);
      }
    }
    
    // Get the file descriptor of the X connection, and then set up
    // a handler to watch for activity on it.
    fd = XConnectionNumber(display);

    // If we get this far then the socket has been created, so we wrap it up
    sock = [[NSFileHandle alloc] initWithFileDescriptor: fd
                                         closeOnDealloc: NO];

    // Register ourselves for notifications
    [[NSNotificationCenter defaultCenter] addObserver: self
                                             selector: @selector(readNotify:)
                                                 name: NSFileHandleDataAvailableNotification
                                               object: sock];
    
    [sock waitForDataInBackgroundAndNotify];
  }

  return self;
}

/*------------------------------------------------------------------------------
 *
 */
- (void)readNotify: (NSNotification*)aNotification
{
  
  NSLog(@"Events in the queue: %d", XQLength(display));
  /*
    if (XXEventHandler(display) == -1)
    {
        NSException* myException = [NSException exceptionWithName: @"ConnectionLost"
                                                           reason: @"Failed To Process Event"
                                                         userInfo: nil];
        [myException raise];
        
    }

    // Tell the handler to notify us on data arriving
   */
  [sock waitForDataInBackgroundAndNotify];
}


/*------------------------------------------------------------------------------
 * displaySize - Returns size of remote display
 */
- (NSRect)displaySize
{
  NSRect r;

  Screen *s = XDefaultScreenOfDisplay(display);


  r.origin.x = 0.0;
  r.origin.y = 0.0;


  r.size.width = s->width;
  r.size.height = s->height;
  
  return r;
}


/*------------------------------------------------------------------------------
 *
 */
- (void)moveCursorRelative: (NSPoint)position
{
  NSLog(@"moveCursorRelative");
  /*
    if (XXSendRelativeMotionEvent(display, (int)position.x, (int)position.y, cursor_no) == -1)
    {
        NSException* myException = [NSException exceptionWithName: @"ConnectionLost"
                                                           reason: @"Failed To Move Pointer"
                                                         userInfo: nil];
        [myException raise];
    }
   */
}


/*------------------------------------------------------------------------------
 *
 */
- (void)moveCursorAbsolute: (NSPoint)position
{
  NSLog(@"moveCursorAbsolute %f %f", position.x, position.y);
  
  g1bson_fake_mouse(display, xid_master_ptr, position.x, position.y);

  /*
    if (XXSendAbsoluteMouseLocation(display, (int)position.x, (int)position.y, cursor_no) == -1)
    {
        NSException* myException = [NSException exceptionWithName: @"ConnectionLost"
                                                           reason: @"Failed To Move Pointer"
                                                         userInfo: nil];
        [myException raise];
    }
   */
}


/*------------------------------------------------------------------------------
 *
 */
- (void)sendKeyPress: (int)keycode
         inDirection: (enum XXDIRECTION)direction
{
  //NSLog(@"sendKeyPress");
  
  int dir;

  dir = (direction == XD_DOWN) ? TRUE : FALSE;
  
  //NSLog(@"%d\n", keycode);
  
  g1bson_fake_keystroke(display, screen, proofWindow, 'X', xid_master_kbd, xid_slave_kbd, xid_master_ptr, xid_slave_ptr);

  
  /*
  if (keymap[keycode] == -1)
      return;

  if ((keycode >= 0) && (keycode < 128))
  {
      if (XXSendKeyEvent(display, keymap[keycode], dir, cursor_no) == -1)
      {
          NSException* myException = [NSException exceptionWithName: @"ConnectionLost"
                                                             reason: @"Failed To Send Key"
                                                           userInfo: nil];
          [myException raise];
      }
  }
  */
  
}


/*------------------------------------------------------------------------------
 *
 */
- (void)sendMousePress: (int)button
           inDirection: (enum XXDIRECTION)direction
{
  NSLog(@"sendMousePress");

  /*
    int dir;

    dir = (direction == XD_DOWN) ? TRUE : FALSE;
    
    if (XXSendMouseButtonEvent(display, button, dir, cursor_no) == -1)
    {
        NSException* myException = [NSException exceptionWithName: @"ConnectionLost"
                                                           reason: @"Failed To Send Button"
                                                         userInfo: nil];
        [myException raise];
    }
   */
}


/*------------------------------------------------------------------------------
 *
 */
- (void)pasteToRemote: (char*)data
{
  /*
	NSUserDefaults* defaults;
	BOOL click;
	
	//NSLog(@"pasteToRemote %s", data);
	
    if (XXSetSelectionOwner(display, data) == -1)
    {
        NSException* myException = [NSException exceptionWithName: @"ConnectionLost"
                                                           reason: @"Failed To Paste"
                                                         userInfo: nil];
        [myException raise];
    }
	
	// Are we meant to automatically send a middle click here?
	defaults = [NSUserDefaults standardUserDefaults];
	click = [defaults boolForKey: OXAutoMiddleClickKey];
        
        NSLog(@"click = %d\n", click);
	
	if (click)
	{
            NSLog(@"Hmmmm\n");
		[self sendMousePress: 2
				 inDirection: XD_DOWN];
		[self sendMousePress: 2
				 inDirection: XD_UP];
	}
   */
}


/*------------------------------------------------------------------------------
 * copyCallBack - this is a C string (for now)
 */
- (void)copyCallBack: (char*)data
{
  /*
    NSData* d;
	
    lastToCopy = nil;

    d = [NSData dataWithBytes: data
                       length: strlen(data) + 1];

    // Create the dictionary
    NSDictionary* copyDict = [NSDictionary dictionaryWithObject: d
                                                         forKey: kCopyData];

    // Send the Procimity Notification
    [[NSNotificationCenter defaultCenter] postNotificationName: kCopyNotification
                                                        object: NSApp
                                                      userInfo: copyDict];
   */
}


/*------------------------------------------------------------------------------
 *
 */
- (void)copyFromRemote
{
  /*
	NSLog(@"copyFromRemote");
	
    if (lastToCopy != nil)
    {
		NSLog(@"lastToCopy not clear");
        // Some other X Server object is awaiting data
        return;
    }
        
    if (XXRequestSelection(display) == -1)
    {
        NSException* myException = [NSException exceptionWithName: @"ConnectionLost"
                                                           reason: @"Failed To Paste"
                                                         userInfo: nil];
        [myException raise];
    }

    lastToCopy = self;
   */
}


/*------------------------------------------------------------------------------
 * disconnect - ideally we'd not let the user do this, as it breaks the central
 *              invarient that display is valid. But you can't tell when the
 *              distructor is going to get called, so we need th ensure the
 *              connection is torn down.
 */
- (void)disconnect
{
    [[NSNotificationCenter defaultCenter] removeObserver: self];
  /*
    XXDisconnectDisplay(display);
    display = NULL;

    [sock release];
    sock = nil;
   */
}


@end