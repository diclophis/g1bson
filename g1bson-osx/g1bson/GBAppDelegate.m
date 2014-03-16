//
//  GBAppDelegate.m
//  g1bson
//
//  Created by Jon Bardin on 2/2/14.
//
//

#import "GBAppDelegate.h"
#import "XXConnection.h"

@implementation GBAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  // Insert code here to initialize your application
  
  XXConnection *connection = [[XXConnection alloc] initWithHost: @"192.168.1.101"
                                     withPosition: 0
                                     withUsername: @""
                                     withPassword: @""
                                         withType: XXConnectionX11
                                       withCursor: 1
                                    forController: self];
}

@end