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

- (NSArray *)accessibilityChildren
{
    return [super accessibilityChildren];
}

- (CGRect)accessibilityFrameInParentSpace
{
    GtkWidget *widget = GTK_WIDGET (ac_element_get_owner([self delegate]));
    return NSMakeRect(0, widget->allocation.height - 24.0, widget->allocation.width, 24.0);
}

- (GdkRectangle)frameInGtkWindowSpace
{
    GdkRectangle parentFrame = [super frameInGtkWindowSpace];
    parentFrame.height = 24;

    return parentFrame;
}
@end
