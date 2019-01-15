/*
 * AtkCocoa
 * Copyright 2016 Microsoft Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#undef GTK_DISABLE_DEPRECATED

#import "atk-cocoa/ACAccessibilityComboBoxElement.h"
#include "atk-cocoa/acelement.h"
#include "atk-cocoa/acdebug.h"

@implementation ACAccessibilityComboBoxElement

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
    GtkComboBox *combobox = GTK_COMBO_BOX (ac_element_get_owner([self delegate]));
    char *text;
    NSString *ret;

    return @(gtk_combo_box_get_active_text (GTK_COMBO_BOX (combobox)));
}

- (id)accessibilityValue
{
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

@end
