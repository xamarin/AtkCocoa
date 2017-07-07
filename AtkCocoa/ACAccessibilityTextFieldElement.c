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

#include <gtk/gtk.h>

#import "atk-cocoa/ACAccessibilityTextFieldElement.h"
#include "atk-cocoa/acelement.h"
#include "atk-cocoa/acdebug.h"

@implementation ACAccessibilityTextFieldElement

- (instancetype)initWithDelegate:(AcElement *)delegate
{
	return [super initWithDelegate:delegate];
}

- (NSString *)accessibilityValue
{
	GObject *owner = ac_element_get_owner ([self delegate]);

	if (!GTK_IS_ENTRY (owner)) {
        return nil;
	}

    return nsstring_from_cstring (ac_element_get_text ([self delegate]));
}

- (void)setAccessibilityValue:(id)newValue
{
    GObject *owner = ac_element_get_owner ([self delegate]);

	if (!GTK_IS_ENTRY (owner)) {
        return;
	}

    if (!gtk_editable_get_editable (GTK_EDITABLE (owner))) {
        return;
    }

    if (![newValue isKindOfClass:[NSString class]]) {
        return;
    }

    NSString *newStr = (NSString *)newValue;
    gtk_entry_set_text (GTK_ENTRY (owner), [newStr UTF8String]);
}

- (NSRect)accessibilityFrame
{
    return [super accessibilityFrame];
}

- (nullable id)accessibilityParent
{
    return [super accessibilityParent];
}
@end
