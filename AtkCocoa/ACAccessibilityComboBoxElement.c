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
	return [super initWithDelegate:delegate];
}

- (BOOL)hasDynamicChildren
{
    return YES;
}

static void
dump_child (GtkWidget *child, gpointer data)
{
    NSMutableArray *children = (__bridge NSMutableArray *) data;
    if (GTK_IS_TOGGLE_BUTTON (child)) {
        AcElement *element = AC_ELEMENT (gtk_widget_get_accessible(child));
        ACAccessibilityElement *e = ac_element_get_accessibility_element(element);

        [e setAccessibilityRole:NSAccessibilityPopUpButtonRole];
        [children addObject:e];
    }
}

- (NSArray *)accessibilityChildren
{
    GtkComboBox *combobox = GTK_COMBO_BOX (ac_element_get_owner([self delegate]));
    NSMutableArray *children = [NSMutableArray array];

    gtk_container_forall(GTK_CONTAINER(combobox), dump_child, (__bridge void *) children);

    return children;
}

- (BOOL)accessibilityPerformPress
{
    return ac_element_perform_press ([self delegate]);
}

- (BOOL)accessibilityPerformShowMenu
{
    return ac_element_perform_show_menu ([self delegate]);
}

- (GtkEntry *)getEntry
{
    GtkComboBox *combobox = GTK_COMBO_BOX (ac_element_get_owner([self delegate]));

    GtkWidget *entry = gtk_bin_get_child (GTK_BIN (combobox));
    if (!GTK_IS_ENTRY(entry)) {
        return NULL;
    }

    return (GtkEntry *)entry;
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
    // If this is an editable combo, then we just pretend it's a group with
    // two children, a textfield and a button
    if ([self getEntry] != NULL) {
        return NSAccessibilityGroupRole;
    }

    return NSAccessibilityPopUpButtonRole;
}


@end
