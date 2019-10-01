//
//  ACAccessibilityPopupMenuElement.m
//  AtkCocoa
//
//  Created by iain on 16/01/2019.
//  Copyright © 2019 Microsoft. All rights reserved.
//

#undef GTK_DISABLE_DEPRECATED

#import "atk-cocoa/ACAccessibilityPopupMenuElement.h"
#include "atk-cocoa/acelement.h"
#include "atk-cocoa/acdebug.h"

@implementation ACAccessibilityPopupMenuElement

- (instancetype)initWithDelegate:(AcElement *)delegate
{
    [self setAccessibilityRole:NSAccessibilityPopUpButtonRole];
    return [super initWithDelegate:delegate];
}

- (NSArray *)accessibilityChildren
{
    return nil;
}

- (BOOL)accessibilityPerformPress
{
    return ac_element_perform_press ([self delegate]);
}

- (BOOL)accessibilityPerformShowMenu
{
    return ac_element_perform_show_menu ([self delegate]);
}

- (NSString *)accessibilityLabel
{
    if ([self delegateIsInvalid]) {
        return nil;
    }

    GtkComboBox *combobox = GTK_COMBO_BOX (ac_element_get_owner([self delegate]));
    char *text;
    NSString *ret;

    return @(gtk_combo_box_get_active_text (GTK_COMBO_BOX (combobox)));
}

- (id)accessibilityValue
{
    if ([self delegateIsInvalid]) {
        return nil;
    }

    GtkComboBox *combobox = GTK_COMBO_BOX (ac_element_get_owner([self delegate]));
    char *text;
    NSString *ret;

    return @(gtk_combo_box_get_active_text (GTK_COMBO_BOX (combobox)));
}

/*
 * This appears to be a hidden method in Cocoa that controls whether a notification should be sent.
 * by default it appears to always return YES
 - (BOOL)accessibilityShouldSendNotification:(id) arg2
 {
 return YES;
 }
 */

- (NSRect)accessibilityFrame
{
    return [super accessibilityFrame];
}

- (id)accessibilityParent
{
    return [super accessibilityParent];
}

- (NSString *)accessibilityRole
{
    if ([self delegateIsInvalid]) {
        return nil;
    }
    
    GtkComboBox *combobox = GTK_COMBO_BOX (ac_element_get_owner([self delegate]));

    if (gtk_combo_box_get_has_entry(combobox) || GTK_IS_COMBO_BOX_ENTRY(combobox)) {
        return NSAccessibilityComboBoxRole;
    } else {
        return NSAccessibilityPopUpButtonRole;
    }
}
@end

