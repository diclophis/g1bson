//
//  GBAppDelegate.m
//  g1bson
//
//  Created by Jon Bardin on 2/2/14.
//
//

#import "GBAppDelegate.h"
#import "XXArrowView.h"

@implementation GBAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  // Insert code here to initialize your application
  
  conn = [[XXConnection alloc] initWithHost: [NSString stringWithUTF8String:getenv("DISPLAY")]
                                     withPosition: XXWindowNorth
                                     withUsername: @""
                                     withPassword: @""
                                         withType: XXConnectionX11
                                       withCursor: 1
                                    forController: self];
  
  //[conn setArrowVisible:YES];
  NSLog(@"connection: %@ NOT", conn);
  
  //[self setWindow:[connection arrowWin]];
  //self.window = [[connection getArrowView] window];
  
  
  //[[connection arrowWin] showWindow:self];
  
  //[self.window setContentView:[connection getArrowView]];
  //[self.window.contentView addSubview:[connection getArrowView]];
  //[self.window addChildWindow:[connection getArrowView] ordered:NSWindowAbove];
  //[self.window addChildWindow:[connection controlWin] ordered:NSWindowAbove];

}

//-(IBAction)didClickButton:(id)sender {
//  [[[conn getArrowView] window] makeKeyAndOrderFront: nil];
//}

-(void)setActiveController:(id)sender {
  [sender setArrowVisible: YES];
  XXArrowView *activeArrowView = [sender getArrowView];
  
  //BOOL isRelative = NO;
  
  [[activeArrowView window] makeKeyAndOrderFront: nil];
  [activeArrowView setController: self];
  
  [activeArrowView setRemote: [sender getRemoteController]];
  [activeArrowView setCaptureScreen: [sender getScreenSize]];
  
  NSPoint lockLocation = [NSEvent mouseLocation];
  [activeArrowView setLockLocation: lockLocation];
  
  [self toggleLock:nil];
}

-(void)toggleLock:(id)sender {
  if (conn != nil) {
    BOOL isRelative = NO;
    
    XXWindowPos capturePos = [conn getPosition];
    
    XXArrowView *activeArrowView = [conn getArrowView];
    [activeArrowView toggleLockWithPosition: isRelative == NO ? capturePos : XXWindowNone];

//
//    if (activeArrowView != nil) {
//      [activeArrowView toggleLockWithPosition: isRelative == NO ? capturePos : XXWindowNone];
//    } else {
//      [xxView toggleLockWithPosition: isRelative == NO ? capturePos : XXWindowNone];
//    }
//    
//    inputLock = !inputLock;
//
//    if (!inputLock)
//    {
//      activeArrowView = nil;
//      [remote setArrowVisible: NO];
//      remote = nil;
//    }
  }
}

@end