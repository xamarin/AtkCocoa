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
    // don't perform if not enabled
    if ([self isAccessibilityEnabled]) {
        return ac_element_perform_press ([self delegate]);
    }
    return FALSE;
}

- (BOOL)accessibilityPerformShowMenu
{
    // don't perform if not enabled
    if ([self isAccessibilityEnabled]) {
        return ac_element_perform_show_menu ([self delegate]);
    }
    return FALSE;
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

- (BOOL)isAccessibilityEnabled
{
    if ([super isAccessibilityEnabled]) {
        // depending on the current GtkSensitivityType of the button
        // and whether the menu has any items, the ComboBox might appear
        // as insenstitive, even if gtk_widget_is_sensitive is true.
        // This behaviour doesn't make sense with VoiceOver and we
        // should always return false here if the menu has no items.
        GtkComboBox *combo_box = GTK_COMBO_BOX (ac_element_get_owner([self delegate]));
        GtkTreeModel *model = gtk_combo_box_get_model (combo_box);
        GtkTreeIter iter;
        if (gtk_tree_model_get_iter_first (model, &iter)) {
            return YES;
        }
    }
    return NO;
}

@end
