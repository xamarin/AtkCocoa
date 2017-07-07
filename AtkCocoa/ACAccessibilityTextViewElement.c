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

#import "atk-cocoa/ACAccessibilityTextViewElement.h"
#include <gtk/gtk.h>
#include "atk-cocoa/acelement.h"
#include "atk-cocoa/acdebug.h"
#include "atk-cocoa/gailtextview.h"

@implementation ACAccessibilityTextViewElement {
}

- (instancetype)initWithDelegate:(AcElement *)delegate
{
    if (!GAIL_IS_TEXT_VIEW (delegate)) {
        return nil;
    }

	return [super initWithDelegate:delegate];
}

- (NSString *)accessibilityStringForRange:(NSRange)range
{
    GtkTextView *textview = GTK_TEXT_VIEW (ac_element_get_owner ([self delegate]));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (textview);
    GtkTextIter startIter, endIter;

    gtk_text_buffer_get_iter_at_offset (buffer, &startIter, (int)range.location);
    gtk_text_buffer_get_iter_at_offset (buffer, &endIter, (int)(range.location + range.length - 1));

    char *text = gtk_text_buffer_get_slice (buffer, &startIter, &endIter, FALSE);
    NSString *retString = nsstring_from_cstring (text);
    g_free (text);

    return retString;
}

- (NSInteger)accessibilityLineForIndex:(NSInteger)index
{
    GtkTextView *textview = GTK_TEXT_VIEW (ac_element_get_owner ([self delegate]));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (textview);
    GtkTextIter lineIter;

    gtk_text_buffer_get_iter_at_offset (buffer, &lineIter, (int)index);

    return gtk_text_iter_get_line (&lineIter);
}

- (NSRange)accessibilityRangeForLine:(NSInteger)lineNumber
{
    GtkTextView *textview = GTK_TEXT_VIEW (ac_element_get_owner ([self delegate]));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (textview);
    GtkTextIter lineIter;

    gtk_text_buffer_get_iter_at_line (buffer, &lineIter, (int)lineNumber);
    
    return NSMakeRange (gtk_text_iter_get_offset (&lineIter), gtk_text_iter_get_chars_in_line (&lineIter));
}

- (NSRect)accessibilityFrameForRange:(NSRange)range
{
    GtkTextView *textview = GTK_TEXT_VIEW (ac_element_get_owner ([self delegate]));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (textview);
    GtkTextIter startIter, endIter;
    GdkRectangle startRect, endRect;
    int startWinX, endWinX, startWinY, endWinY;

    gtk_text_buffer_get_iter_at_offset (buffer, &startIter, (int)range.location);
    gtk_text_buffer_get_iter_at_offset (buffer, &endIter, (int)(range.location + range.length - 1));

    gtk_text_view_get_iter_location (textview, &startIter, &startRect);
    gtk_text_view_get_iter_location (textview, &endIter, &endRect);

//    g_print ("Getting %d -> %d\n", (range.location, (int)(range.location + range.length - 1));
//    g_print ("Buffer rect %d,%d -> %d,%d\n", startRect.x, startRect.y, endRect.x, endRect.y);

    gtk_text_view_buffer_to_window_coords (textview, GTK_TEXT_WINDOW_WIDGET, startRect.x, startRect.y, &startWinX, &startWinY);
    gtk_text_view_buffer_to_window_coords (textview, GTK_TEXT_WINDOW_WIDGET, endRect.x, endRect.y, &endWinX, &endWinY);

//    g_print ("Window rect %d,%d -> %d,%d\n", startWinX, startWinY, endWinX, endWinY);

    // FIXME: Some cunning calculation to work out line wrap?
//    g_print ("Frame: %d,%d, %d x %x", startWinX, startWinY, endWinX - startWinX, endWinY - startWinY);
    return NSMakeRect (startWinX, startWinY, endWinX - startWinX, endWinY - startWinY);
}

- (NSString *)accessibilityValue
{
    GtkTextView *textview = GTK_TEXT_VIEW (ac_element_get_owner ([self delegate]));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (textview);
    GtkTextIter startIter, endIter;

    gtk_text_buffer_get_bounds (buffer, &startIter, &endIter);
    char *text = gtk_text_buffer_get_text (buffer, &startIter, &endIter, FALSE);

    NSString *retString = nsstring_from_cstring (text);
    g_free (text);

    return retString;
}

// The compiler will complain that accessibilityFrame is not defined
// but we want accessibilityFrameInParentSpace to be called instead
// and Cocoa will call accessibilityFrame over accessibilityFrameInParentSpace
// so we just accept the warning
- (NSRect)accessibilityFrame
{
    return [super accessibilityFrame];
}

- (id)accessibilityParent
{
    return [super accessibilityParent];
}
@end
