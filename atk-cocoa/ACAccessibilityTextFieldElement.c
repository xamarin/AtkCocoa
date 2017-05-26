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

#import "ACAccessibilityTextFieldElement.h"
#include "acelement.h"
#include "acdebug.h"

@implementation ACAccessibilityTextFieldElement

- (instancetype)initWithDelegate:(AcElement *)delegate
{
	return [super initWithDelegate:delegate];
}

- (BOOL)respondsToSelector:(SEL)sel
{
    // NSLog (@"Entry: %@", NSStringFromSelector (sel));
    return [super respondsToSelector:sel];
}

- (NSString *)accessibilityValue
{
	GObject *owner = ac_element_get_owner ([self delegate]);

	if (!GTK_IS_ENTRY (owner)) {
        return @"";
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

- (NSInteger)accessibilityNumberOfCharacters
{
    AcElement *delegate = [self delegate];
    if (!ATK_IS_TEXT (delegate)) {
        return 0;
    }

    return atk_text_get_character_count (ATK_TEXT (delegate));
}

- (NSInteger)accessibilityInsertionPointLineNumber
{
    return 0;
}

- (NSString *)accessibilitySelectedText
{
    AcElement *delegate = [self delegate];
    if (!ATK_IS_TEXT (delegate)) {
        return @"";
    }

    int s, e;
    return nsstring_from_cstring (atk_text_get_selection (ATK_TEXT (delegate), 0, &s, &e));
}

- (NSRange)accessibilitySelectedTextRange
{
    AcElement *delegate = [self delegate];
    if (!ATK_IS_TEXT (delegate)) {
        return NSMakeRange (0, 0);
    }

    int s, e;
    char *sel = atk_text_get_selection (ATK_TEXT (delegate), 0, &s, &e);
    if (sel == NULL) {
        return NSMakeRange (atk_text_get_caret_offset (ATK_TEXT (delegate)), 0);
    } else {
        g_free (sel);
        return NSMakeRange (s, e - s);
    }
}

- (NSString *)accessibilityStringForRange:(NSRange)range
{
    AcElement *delegate = [self delegate];
    if (!ATK_IS_TEXT (delegate)) {
        return @"";
    }

    int e = range.location + range.length;
    return nsstring_from_cstring (atk_text_get_text (ATK_TEXT (delegate), range.location, e));
}

- (NSRect)accessibilityFrameForRange:(NSRange)range
{
    return NSMakeRect (200, 200, 50, 50);
}

- (NSRange)accessibilityVisibleCharacterRange
{
    AcElement *delegate = [self delegate];
    if (!ATK_IS_TEXT (delegate)) {
        return NSMakeRange (0, 0);
    }

    return NSMakeRange (0, atk_text_get_character_count (ATK_TEXT (delegate)));
}

- (NSRange)accessibilityRangeForLine:(NSInteger)line
{
    AcElement *delegate = [self delegate];
    if (!ATK_IS_TEXT (delegate)) {
        return NSMakeRange (0, 0);
    }

    return NSMakeRange (0, atk_text_get_character_count (ATK_TEXT (delegate)));
}

- (NSInteger)accessibilityLineForIndex:(NSInteger)index
{
    return 0;
}

@end