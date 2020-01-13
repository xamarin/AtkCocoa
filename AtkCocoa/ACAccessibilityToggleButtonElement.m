//
//  ACAccessibilityToggleButtonElement.m
//  AtkCocoa
//
//  Created by iain on 09/01/2020.
//  Copyright © 2020 Microsoft. All rights reserved.
//

#import "atk-cocoa/ACAccessibilityToggleButtonElement.h"
#import "atk-cocoa/ACAccessibilityComboBoxElement.h"

@implementation ACAccessibilityToggleButtonElement

@synthesize comboBox;

// When a toggle button is inside a combobox, then we want to forward all the requests to the combobox.
- (NSString *)accessibilityLabel
{
    if (![self comboBox]) {
        return [super accessibilityLabel];
    }

    return [[self comboBox] accessibilityLabel];
}

- (NSString *)accessibilityTitle
{
    if (![self comboBox]) {
        return [super accessibilityTitle];
    }

    return [[self comboBox] accessibilityTitle];
}

- (id)accessibilityTitleUIElement
{
    if (![self comboBox]) {
        return [super accessibilityTitleUIElement];
    }

    return [[self comboBox] accessibilityTitleUIElement];
}
@end
