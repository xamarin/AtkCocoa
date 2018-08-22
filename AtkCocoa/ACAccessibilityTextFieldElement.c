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

    if (!gtk_entry_get_visibility (GTK_ENTRY (owner))){
        return nil;
    }

    return nsstring_from_cstring (get_entry_text([self delegate]));
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

- (NSInteger)accessibilityNumberOfCharacters
{
    return strlen (get_entry_text([self delegate]));
}

- (NSInteger)accessibilityLineForIndex:(NSInteger)index
{
    return 0;
}

const char *
get_entry_text (AcElement *element)
{
    GtkWidget *widget = gtk_accessible_get_widget(GTK_ACCESSIBLE (element));
    if (GTK_IS_ENTRY(widget)) {
        return gtk_entry_get_text(GTK_ENTRY (widget));
    }

    return "";
}

- (NSRange)accessibilityRangeForLine:(NSInteger)line
{
    return NSMakeRange(0, strlen (get_entry_text(AC_ELEMENT ([self delegate]))));
}

- (NSString *)accessibilityStringForRange:(NSRange)range
{
    NSString *ret = nsstring_from_cstring(get_entry_text([self delegate]));

    return [ret substringWithRange:range];
}

- (NSAttributedString *)accessibilityAttributedStringForRange:(NSRange)range
{
    NSString *ret = nsstring_from_cstring(get_entry_text([self delegate]));

    return [[NSAttributedString alloc] initWithString:[ret substringWithRange:range]];
}

#define PADDING 2
- (NSRect)accessibilityFrameForRange:(NSRange)range
{
    GtkWidget *widget = gtk_accessible_get_widget(GTK_ACCESSIBLE([self delegate]));
    PangoLayout *layout = gtk_entry_get_layout(GTK_ENTRY (widget));
    PangoRectangle first_rect, last_rect;

    pango_layout_index_to_pos (layout, (int)range.location, &first_rect);
    pango_layout_index_to_pos (layout, (int)(range.location + range.length - 1), &last_rect);

    const char *text = get_entry_text([self delegate]);
    NSString *t = nsstring_from_cstring(text);

    NSRect first = NSMakeRect(first_rect.x / PANGO_SCALE, first_rect.y / PANGO_SCALE,
                              first_rect.width / PANGO_SCALE, first_rect.height / PANGO_SCALE);
    NSRect last = NSMakeRect(last_rect.x / PANGO_SCALE, last_rect.y / PANGO_SCALE,
                             last_rect.width / PANGO_SCALE, last_rect.height / PANGO_SCALE);

    NSRect fullRect = NSUnionRect(first, last);
    NSRect frame = [self accessibilityFrame];

    int x_layout, y_layout;
    gtk_entry_get_layout_offsets (GTK_ENTRY (widget), &x_layout, &y_layout);

    int halfFrameHeight = frame.size.height / 2;
    int dy = halfFrameHeight - y_layout;
    int cocoaY = halfFrameHeight + dy;

    NSRect ret = NSMakeRect(frame.origin.x + fullRect.origin.x + x_layout - PADDING,
                            frame.origin.y + fullRect.origin.y + dy - PADDING,
                            fullRect.size.width + (PADDING * 2), fullRect.size.height + (PADDING * 2));

    return ret;
}

@end
