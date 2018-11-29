//
//  ACAccessibilityTableHeaderView.m
//  AtkCocoa
//
//  Created by iain on 28/11/2018.
//  Copyright © 2018 Microsoft. All rights reserved.
//

#import "atk-cocoa/ACAccessibilityTableHeaderElement.h"

@implementation ACAccessibilityTableHeaderElement

- (NSString *)accessibilityRole
{
    return NSAccessibilityGroupRole;
}
@end
