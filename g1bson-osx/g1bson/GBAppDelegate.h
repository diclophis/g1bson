//
//  GBAppDelegate.h
//  g1bson
//
//  Created by Jon Bardin on 2/2/14.
//
//

#import <Cocoa/Cocoa.h>
#import "GBController.h"

@interface GBAppDelegate : NSObject <NSApplicationDelegate, GBController>

@property (assign) IBOutlet NSWindow *window;

@end
