//
//  GBAppDelegate.h
//  g1bson
//
//  Created by Jon Bardin on 2/2/14.
//
//

#import <Cocoa/Cocoa.h>
#import "GBController.h"
#import "XXConnection.h"

@interface GBAppDelegate : NSObject <NSApplicationDelegate, GBController> {
  XXConnection *conn;
}

@property (assign) IBOutlet NSWindow *window;

@end
