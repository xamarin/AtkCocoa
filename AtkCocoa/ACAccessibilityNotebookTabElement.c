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

#import "atk-cocoa/ACAccessibilityNotebookTabElement.h"
#include "atk-cocoa/acelement.h"
#include "atk-cocoa/acdebug.h"
#include "atk-cocoa/acutils.h"
#include "atk-cocoa/gailnotebookpage.h"

@implementation ACAccessibilityNotebookTabElement {
	GailNotebookPage *_delegate;
}

- (BOOL)respondsToSelector:(SEL)sel
{
    return [super respondsToSelector:sel];
}

- (instancetype)initWithDelegate:(GailNotebookPage *)delegate
{
	self = [super init];

	if (self == nil) {
		return nil;
	}

	_delegate = delegate;

	[self setAccessibilityRole:NSAccessibilityRadioButtonRole];
	[self setAccessibilityRoleDescription:@"tab"];

	return self;
}

- (GailNotebookPage *)delegate
{
	return _delegate;
}

- (id)accessibilityValue
{
	int currentPage = gtk_notebook_get_current_page (_delegate->notebook);

	return (currentPage == _delegate->index) ? @(1) : @(0);
}

- (NSString *)accessibilityTitle
{
    // Widget is defunct
    if (_delegate->notebook == NULL) {
        return nil;
    }

    GtkWidget *page = gtk_notebook_get_nth_page (_delegate->notebook, _delegate->index);
    GtkWidget *label = gtk_notebook_get_tab_label (_delegate->notebook, page);

    if (GTK_IS_CONTAINER (label))
        label = gail_notebook_page_find_label_child (GTK_CONTAINER (label));

    if (GTK_IS_LABEL (label)) {
      const char *label_text = gtk_label_get_text (GTK_LABEL (label));
      return nsstring_from_cstring (label_text);
    } else {
      return nil;
    }
}

- (BOOL)accessibilityPerformPress
{
	gtk_notebook_set_current_page (_delegate->notebook, _delegate->index);
	return YES;
}

- (NSRect)accessibilityFrameInParentSpace
{
	// Widget is defunct
	if (_delegate->notebook == NULL) {
		return NSZeroRect;
	}

	GtkWidget *page = gtk_notebook_get_nth_page (_delegate->notebook, _delegate->index);
	GtkWidget *label = gtk_notebook_get_tab_label (_delegate->notebook, page);
	int x, y;
	int px, py;

	if (page == NULL || label == NULL) {
		return NSZeroRect;
	}
	GdkRectangle pageAllocation = GTK_WIDGET(_delegate->notebook)->allocation;
	GdkRectangle allocation = label->allocation;

	x = allocation.x - pageAllocation.x;
	y = allocation.y - pageAllocation.y;

	int halfHeight = pageAllocation.height / 2;
	int dy = halfHeight - y;
	int cocoaY = halfHeight + dy;

	NSRect rect = NSMakeRect (x, cocoaY - allocation.height, allocation.width, allocation.height);

	return rect;
}
@end
